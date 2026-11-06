# Decodificador NFC-A (ISO/IEC 14443 tipo A)

Este documento describe cómo funciona el decodificador NFC-A de `lab-radio` y recoge los errores y las optimizaciones encontrados al revisarlo.

Ficheros:

| Fichero | Contenido |
|---|---|
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcA.cpp` | Demodulador, decodificador de tramas y lógica de protocolo (`NfcA::Impl`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/tech/NfcA.h` | Interfaz pública y códigos de comando |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/NfcTech.h` / `NfcTech.cpp` | Estructuras compartidas por todas las tecnologías y preprocesado de cada muestra (`nextSample`) |
| `src/nfc-lib/lib-lab/lab-radio/src/main/cpp/NfcDecoder.cpp` | Orquestador: detección de portadora y reparto entre NFC-A/B/F/V |
| `src/nfc-lib/lib-lab/lab-radio/src/main/include/lab/nfc/Nfc.h` | Constantes de temporización (FGT, FWT, SFGT, tablas FSD/FWT) |

---

## 1. Visión general

```
SignalBuffer (magnitud de la señal, 1 canal float)
        │
        ▼
NfcDecoderStatus::nextSample()        ← preprocesado común por muestra
        │   envolvente, filtro DC (IIR), desviación media, profundidad de modulación
        │   → buffer circular sample[1024]
        ▼
NfcDecoder::nextFrames()
   ├─ sin modulación activa → NfcA::detect() (y B, F, V) en cada muestra
   └─ con modulación NFC-A  → NfcA::decode()
                                 ├─ decodePollFrame()    (lector → tarjeta, ASK 100 % + Miller modificado)
                                 └─ decodeListenFrame()  (tarjeta → lector)
                                        ├─ 106 kbps: OOK de subportadora + Manchester
                                        └─ 212/424 kbps: BPSK de subportadora + NRZ-L
        │
        ▼
process(RawFrame) → fase de protocolo, CRC, timings (FGT/FWT) para la siguiente trama
        │
        ▼
std::list<RawFrame>
```

El decodificador es una máquina de estados que procesa muestra a muestra. El estado se reparte en:

- `bitrateParams[rate]`: duraciones de símbolo en muestras para 106, 212 y 424 kbps (índices `r106k`..`r424k`).
- `modulationStatus[rate]`: estado del detector o demodulador para cada velocidad (integradores, ventanas de búsqueda, picos, buffers).
- `symbolStatus`: último símbolo decodificado (patrón, valor, inicio, fin, flanco).
- `streamStatus`: bits y bytes acumulados de la trama en curso.
- `frameStatus`: tipo de trama esperada (poll o listen), inicio y fin, y ventanas `guardEnd` y `waitingEnd`.
- `protocolStatus`: parámetros negociados (FSD, FWT, SFGT) que se actualizan con RATS/ATS.
- `chainedFlags`: flags que se propagan a las tramas siguientes (p. ej. `Encrypted` tras MIFARE AUTH).

`decoder->bitrate` y `decoder->modulation` apuntan a la velocidad detectada. Si `decoder->modulation == nullptr`, el orquestador vuelve al modo de detección.

## 2. Preprocesado común (`NfcDecoderStatus::nextSample`)

Por cada muestra de entrada:

1. Incrementa `signalClock`, el reloj maestro en muestras (`unsigned int`).
2. Actualiza la envolvente (`signalEnvelope`) con una media exponencial (constante 5·10⁵/fs), pero solo cuando la muestra se separa menos de un 5 % de la envolvente, o tras 10 ETU sin actualizar. Así las pausas de modulación no arrastran la envolvente.
3. Aplica un filtro IIR de eliminación de DC: `y = x + 0.9·y₁; filtered = y − y₁`.
4. Calcula la desviación media de la señal filtrada (`meanDeviation`) y la media lenta (`signalAverage`, usada para detectar la portadora).
5. Guarda en `sample[clock & 1023]` cuatro valores:
   - `samplingValue`: magnitud cruda.
   - `filteredValue`: magnitud sin DC.
   - `meanDeviation`: nivel de ruido.
   - `modulateDepth = (env − clamp(x, 0, env)) / env`: profundidad de modulación, de 0 a 1.

Todos los demoduladores leen de este buffer con offsets fijos (`offsetSignalIndex`, `offsetDelayNIndex`, `offsetFutureIndex`), que se calculan en `initialize()`.

## 3. Parámetros por velocidad (`initialize`)

Con `u = fs / fc` (muestras por ciclo de portadora, `sampleTimeUnit`) y `rate` = 0, 1, 2 (106, 212, 424 kbps):

| Campo | Valor | Significado |
|---|---|---|
| `period1SymbolSamples` | `u·(128 >> rate)` | 1 símbolo (1 ETU) |
| `period2SymbolSamples` | `u·(64 >> rate)` | ½ símbolo |
| `period4SymbolSamples` | `u·(32 >> rate)` | ¼ símbolo |
| `period8SymbolSamples` | `u·(16 >> rate)` | ⅛ símbolo |
| `period0SymbolSamples` | `u·(256 >> rate)` | 2 símbolos |
| `symbolDelayDetect` | suma de `period1` de las velocidades inferiores | retardo extra de cada velocidad |

El retardo `symbolDelayDetect` hace que 212 y 424 kbps se evalúen sobre muestras más antiguas. Así, en cada muestra se evalúa primero 106 kbps y una velocidad superior solo gana si la inferior no ha validado ya el pulso. Todos los tiempos que se publican se corrigen restando este retardo.

Timings de protocolo por defecto, en unidades de 1/fc (`Nfc.h`):

| Constante | Valor | Uso |
|---|---|---|
| `NFCA_FGT_DEF` | 1024 | Guarda mínima antes de buscar la respuesta (TR0) |
| `NFCA_FWT_ATQA` | 128·18 | Espera máxima de la respuesta a REQA/WUPA/SELn |
| `NFCA_FWT_DEF` | 256·16·2⁴ | FWT por defecto (FWI = 4) |
| `NFCA_SFGT_DEF` | 256·16 | SFGT por defecto |
| `NFC_FWT_ACTIVATION` | 71680 | Espera máxima del ATS tras RATS |

## 4. Detección de la trama del lector (`detectModulation`)

Se ejecuta en cada muestra mientras no hay modulación activa, y solo si la envolvente supera `powerLevelThreshold`. Para cada velocidad:

1. **Integrador de medio símbolo**: `filterIntegrate` es la suma móvil de `samplingValue` sobre `period2` muestras.
2. **Buffer de correlación**: `correlationData[t mod P1] = filterIntegrate`, un buffer circular de un símbolo.
3. **Correlación**:
   - `S0 = I(t) − I(t − P/2)`: segunda mitad del último símbolo menos la primera.
   - `S1 = I(t − P/2) − I(t − 1)` (ver el hallazgo E7).
   - `SD = (S0 − S1) / P2`, con signo.

   Una pausa ASK que entra en la ventana produce un mínimo negativo de `SD`, y al salir produce un máximo positivo.
4. **Fase 1, flanco de bajada**: si `SD < −envolvente·correlationThreshold` (0.75 por defecto), registra el mínimo (`correlatedPeakTime`) y la profundidad máxima (`modulateDepth`). La búsqueda acaba ¼ de símbolo después del último mínimo. Si la profundidad no llega a `minimumModulationDeep` (0.90), se descarta.
5. **Fase 2, flanco de subida**: abre una ventana de ±⅛ de símbolo alrededor de `pico + P/2` y busca el máximo positivo de `SD`.
6. **Validación**: la anchura `fin − inicio` debe estar dentro de `P1 ± P1/4`.
7. Si el pulso es válido, se considera el SoF (patrón Z). Entonces:
   - fija `decoder->bitrate` y `decoder->modulation`;
   - pone `frameStatus.frameType = NfcPollFrame`;
   - prepara la ventana del siguiente símbolo (`searchSyncTime = fin + P1`, ±P8);
   - fija el umbral de símbolo en `searchValueThreshold = pico/2`.

Si en un símbolo no se completa la búsqueda (`clock > peak + P1`), el estado se reinicia.

## 5. Trama del lector: ASK 100 % + Miller modificado

### 5.1 Símbolos (`decodePollFrameSymbolAsk`)

Usa el mismo correlador que la detección, pero con `SD = |S0 − S1| / P2`. En cada ventana `[sync − P8, sync + P8]`:

- Sigue el máximo de `SD` por encima del umbral, para resincronizar el fin de símbolo.
- En `sync`, captura `SD`, `S0` y `S1`.
- Al cerrar la ventana clasifica el símbolo:
  - `SD(sync) < umbral` → **Patrón Y**, sin pausa. El tiempo se extrapola desde `sync`, sin resincronizar.
  - `S0 > S1` → **Patrón Z**, pausa al inicio del bit. Resincroniza con el pico.
  - en otro caso → **Patrón X**, pausa a mitad de bit. Resincroniza con el pico.
- La siguiente ventana se abre en `fin + P1`.

### 5.2 Bits y bytes (`decodePollFrame`)

- Cada bit se decodifica con un símbolo de retraso (`streamStatus.previous`). Así, al detectar el EoF no se cuenta como dato el "0" que lo precede. X equivale a 1; Y y Z equivalen a 0.
- **EoF**: un patrón Y precedido de Y o de Z. Es el "0 lógico + Y" de ISO 14443-2.
- Cada byte tiene 8 bits de datos (LSB primero) más 1 bit de paridad impar (`checkParity`). Un error de paridad marca `ParityError` en la trama.
- Al cerrar la trama:
  - si quedan ≥ 7 bits sueltos, se añaden como último byte;
  - una trama de exactamente 7 bits se marca `ShortFrame` (REQA/WUPA);
  - si se llega a `maxFrameSize`, la trama se marca `Truncated`.
- Después se llama a `process()`, se añade la trama a la lista y se limpia el estado del demodulador, conservando `decoder->bitrate` para buscar la respuesta.

## 6. Trama de la tarjeta (listen)

`process()` fija dos instantes a partir del final de la trama del lector:

- `guardEnd = frameEnd + FGT`: antes de este instante no se busca respuesta.
- `waitingEnd = frameEnd + FWT`: si no hay respuesta, se considera timeout.

La tarjeta responde a la misma velocidad que el lector.

### 6.1 106 kbps: OOK de subportadora (fc/16) + Manchester

**SoF, patrón D (`decodeListenFrameStartAsk`)**

- Señal: `filteredValue² · 10`, la energía de la subportadora, integrada sobre ½ símbolo.
- Correlación `S0 = I(t) − I(t − P/2)`.
- En `guardEnd` el umbral se calibra con el ruido: `meanDeviation · P8`.
- Si se pasa de `waitingEnd`, o reaparece modulación del lector (`modulateDepth > 0.9`), devuelve `NoPattern` (timeout).
- El pulso válido es un máximo positivo seguido de un mínimo negativo, con anchura `P1 ± P8`. Tras validarlo, el umbral pasa a ser el 25 % del pico.

**Símbolos (`decodeListenFrameSymbolAsk`)**

- Mismo correlador, con `SD = |S0 − S1|`.
- `SD(sync) > umbral`: si `S0 > S1` es el **patrón E** (0, subportadora en la segunda mitad); en otro caso es el **patrón D** (1, subportadora en la primera mitad). El umbral se adapta al 25 % del último pico.
- Sin modulación → **patrón F** (EoF).

**Bytes (`decodeListenFrame`)**

- 8 bits de datos más paridad impar.
- Se aceptan tramas de 4 bits (ACK/NAK de MIFARE).

### 6.2 212/424 kbps: BPSK de subportadora + NRZ-L

**SoF, patrón S (`decodeListenFrameStartBpsk`)**

- Demodulación diferencial: `filtered(t) · filtered(t − P1) · 10`, integrada sobre ¼ de símbolo (`phaseIntegrate`).
- El valor es positivo si el símbolo actual tiene la misma fase que el anterior, y negativo si la fase se ha invertido.
- Busca el preámbulo de subportadora sin cambios de fase (32 ciclos de subportadora, entre 3 y 4 ETU de 106 kbps). Termina con la primera inversión de fase.

**Símbolos (`decodeListenFrameSymbolBpsk`)**

- Un detector de cruce por cero, que actúa una sola vez por símbolo, resincroniza `searchSyncTime`.
- En `sync`:
  - si `|phase| < umbral`, es el EoF (**patrón O**);
  - si `phase < −umbral`, hay inversión de fase: se conmuta el valor del bit y el patrón M/N;
  - en otro caso se mantiene el valor y el umbral pasa a ser el 25 % de `phase`.
- Bytes: 8 bits de datos más 1 de paridad. El último byte se comprueba con paridad par, invertida respecto al resto.

## 7. Capa de protocolo (`process`)

Antes de clasificar, una trama del lector restaura los timings por defecto de `protocolStatus`. Después se prueban, en este orden:

| Función | Reconoce (poll) | Efecto |
|---|---|---|
| `processREQA` | `0x26`/`0x52`, 1 byte | Reinicia los parámetros de protocolo y `chainedFlags`. FWT pasa a ser `NFCA_FWT_ATQA` |
| `processHLTA` | `0x50`, 4 bytes | Comprueba CRC, reinicia los parámetros y la modulación (la tarjeta no responde) |
| `processSELn` | `0x93`/`0x95`/`0x97` | Mismos timings que REQA |
| `processRATS` | `0xE0` | FSD = `NFC_FDS_TABLE[FSDI]`, FWT = activación. En la respuesta (ATS) lee TB(1) → SFGI, FWI |
| `processPPSr` | `0xDx` | Comprueba CRC |
| `processAUTH` | `0x60`/`0x61` | Tras la respuesta activa `Encrypted` en `chainedFlags`. A partir de ahí toda trama se considera de aplicación y sin paridad válida |
| `processIBlock` / `RBlock` / `SBlock` | máscaras del PCB | Comprueban CRC |
| `processOther` | resto | Fase de aplicación, comprueba CRC |

Las respuestas se clasifican según `frameStatus.lastCommand`, que se pone a 0 tras cada respuesta.

Al acabar, `process()` calcula `guardEnd`/`waitingEnd` para la respuesta (tramas poll) o `guardEnd` para el siguiente comando (tramas listen). También vuelve a poner `frameType` en modo detección.

---

## 8. Errores encontrados

Están ordenados por gravedad. Las líneas se refieren a `tech/NfcA.cpp`.

### E1. Desbordamiento de `streamStatus.buffer` con FSD ≥ 512. **Grave**

`streamStatus.buffer` tiene 512 bytes (`NfcTech.h`), pero `processRATS` (l. 1716) asigna `maxFrameSize = NFC_FDS_TABLE[fsdi]`, que puede valer hasta 4096 (FSDI 9–C).

- Con FSD ≥ 1024, el bucle de bytes escribe más allá del array: `bytes < maxFrameSize` lo permite.
- Con FSD = 512, el caso de truncado escribe en `buffer[512]`. Pasa en `bits >= 7` (l. 455), `bits == 4` (l. 615) y `bits == 9` (l. 728), porque se escribe en `buffer[bytes++]` justo cuando `bytes == maxFrameSize`.

La escritura corrompe `frameStatus`, el miembro que viene a continuación en `Impl`. Basta un RATS con FSDI ≥ 9, real o corrupto por ruido.

**Corrección**: limitar `maxFrameSize` a `sizeof(streamStatus.buffer) − 1`, o ampliar el buffer a 4096 + margen.

### E2. FSDI reservado (D–F) bloquea el decodificador hasta el siguiente REQA. **Grave**

`NFC_FDS_TABLE[13..15] = 0`. Con `maxFrameSize = 0`, cada trama se trunca en su primer símbolo, o se aborta con `resetModulation()`, hasta que llegue un REQA, WUPA o HLTA. ISO 14443-4 y NFC Forum piden tratar un FSDI reservado como el máximo (o 256).

**Corrección**: mapear FSDI > 0xC a 256 o 4096.

### E3. Las S-Blocks nunca se reconocen. **Media**

`NFCA_SBLOCK = 0xC0`, pero un PCB de S-block cumple `(pcb & 0xC7) == 0xC2`, porque el bit b2 siempre vale 1. Además se exige `limit() == 4`, pero S(DESELECT) sin CID ocupa 3 bytes.

Resultado: DESELECT (`C2`) y WTX (`F2`) acaban en `processOther`. El parser de la aplicación (`ParserNfc.cpp:245`) sí usa 0xC2 con 3–4 bytes.

**Corrección**: `NFCA_SBLOCK = 0xC2` y `limit() >= 3 && limit() <= 4` (o 5 con CID).

### E4. `resetFrameSearch()` no limpia el estado de bits ni el umbral. **Media**

Si un pulso espurio se toma como SoF y se decodifican unos pocos bits antes del EoF (l. 653), solo se reinician los tiempos de modulación y `frameStart`. Quedan sin limpiar:

- `streamStatus.bits`, `data` y `flags`: la siguiente respuesta real empieza desplazada y con paridad errónea;
- `searchValueThreshold`: queda en el 25 % del pico espurio. Como solo se recalibra cuando `clock == guardEnd`, y ese instante ya pasó, el nuevo SoF se busca con un umbral arbitrario.

**Corrección**: `streamStatus = {}` en `resetFrameSearch()` y recalcular el umbral desde `meanDeviation`.

### E5. Lecturas fuera de rango en RATS/ATS. **Media**

- `processRATS` (poll) lee `frame[1]` sin comprobar que `limit() >= 2`.
- En el ATS, `T0`, `TA` y `TB` se leen sin comparar con `TL` ni con `limit()`. Si un ATS viene corto o con TL erróneo, se usan los bytes de CRC (o memoria fuera de la trama, porque `Buffer::operator[]` no comprueba límites) como TB. Eso configura FWT/SFGT con valores arbitrarios, y FWI puede llegar a dar FWT de varios segundos.

**Corrección**: validar `offset < tl && offset < limit() − 2` antes de cada lectura.

### E6. Bloqueo si la ventana de búsqueda queda en el pasado. **Baja (robustez)**

Las búsquedas terminan con `signalClock == searchEndTime` o `== searchSyncTime` (l. 321, 873, 1029, 1162, 1304, BPSK). Si por cualquier camino el fin de ventana se calcula en el pasado, el bucle consume muestras sin fin y el decodificador se queda en esa trama hasta el final del stream. Ejemplos:

- `correlatedPeakTime == 0` al resincronizar en `decodePollFrameSymbolAsk`. Es posible si `SD(sync) == umbral`, porque el pico exige `>` y la clasificación exige `<`.
- El desbordamiento de `signalClock` (E8).

Lo mismo pasa con la calibración `clock == guardEnd` (l. 987, 1253): si `guardEnd` ya ha pasado, el umbral no se recalibra.

**Corrección**: usar `>=` para cerrar las ventanas, y abortar la trama si `searchEndTime < signalClock`.

### E7. `S1` no es una medida independiente. **Baja (claridad / posible error)**

`filterPoint3 = (t + P1 − 1) % P1` apunta a la posición escrita en la muestra anterior (`t − 1`), no a la más antigua del buffer (`t + 1 ≡ t − P1 + 1`). Por eso:

```
S1 = I(t−P/2) − I(t−1) ≈ −S0
SD = S0 − S1 = 2·S0 − (s[t] − s[t−P/2])
```

La comparación `S0 > S1` equivale a `S0 > 0`. El decodificador funciona porque los umbrales (0.75·env, pico/2) están calibrados con este `SD ≈ 2·S0`, pero el nombre y el comentario sugieren otra cosa.

**Corrección**: o bien es intencionado, y se documenta y se simplifica a `SD = 2·S0`, o bien se pretendía comparar con el semisímbolo anterior, y hay que revisarlo junto con los umbrales.

### E8. Reloj de 32 bits. **Baja (afecta a todo el decodificador)**

`signalClock`, `frameStart`/`frameEnd` y todos los `search*Time` son `unsigned int`. A 10 MS/s desbordan cada ~7,1 min, y a 40 MS/s cada ~1,8 min. En capturas en vivo largas:

- las marcas de tiempo de las tramas (`timeStart = frameStart / fs`) vuelven a 0;
- las comparaciones `<`/`>` de las ventanas fallan alrededor del desbordamiento.

**Corrección**: usar `uint64_t` para el reloj y los tiempos, o comparar con diferencias con signo.

### E9. `maximumModulationDeep` no se usa en NFC-A. **Baja**

Se configura desde la API y la interfaz (`setModulationThresholdNfcA(min, max)`) y aparece en el log, pero ningún camino de NFC-A la consulta. En NFC-B sí se usa. El parámetro "max" de NFC-A no tiene efecto.

### E10. Otros detalles menores

- `decodeListenFrame` devuelve `RawFrame::Nil` en una función `bool` (l. 593). Funciona por `operator bool()` (devuelve `false`), pero debería ser `return false;`.
- `processHLTA` comprueba `!hasFrameFlags(CrcError)` antes de calcular el CRC, así que la condición siempre se cumple. Tampoco comprueba que el segundo byte sea `0x00`.
- `processREQA` acepta `0x26`/`0x52` como trama de 8 bits. Debería exigir `ShortFrame` (7 bits).
- `processIBlock` exige `limit() > 4`, por lo que no reconoce I-blocks con INF de 0–1 byte (p. ej. encadenado o R/ACK de 1 byte). `processRBlock`, con `limit() == 3`, no reconoce R-blocks con CID.
- `processSELn` no comprueba CRC en SELECT (NVB = 0x70) ni en SAK.
- Las tramas de anticolisión orientadas a bit (NVB ≠ 0x70) pueden tener entre 1 y 6 bits en el último byte, y esos bits se descartan sin ningún flag. Lo mismo ocurre con respuestas que no sean de 4 ni de 8·n bits.
- `maxFrameSize` (el FSD, máximo que recibe el lector) también se aplica a las tramas del lector, cuando el límite correcto para esas es el FSC (FSCI en T0 del ATS).
- No se procesa S(WTX). Una respuesta tras WTX que supere el FWT negociado se pierde por timeout.
- Si un comando no tiene respuesta (timeout), `lastCommand` no se reinicia. Si después llega una trama no reconocida (`processOther`, que no modifica `lastCommand`), su respuesta se interpretaría según el comando anterior. Por ejemplo, se parsearía como ATS y cambiaría FWT/SFGT.
- Hay campos que se calculan pero no se usan: `startUpGuardTime`, `requestGuardTime`, `lastFrameEnd` y `detectIntegrate`.
- La longitud de la cadena de retardos (`symbolDelayDetect` + P1 a 424 kbps ≈ 224/fc) debe caber en `BUFFER_SIZE = 1024`, lo que limita fs a ≈ 60 MS/s. No se comprueba.

## 9. Optimizaciones

### 9.1 Aplicadas

Todas mantienen la salida exacta del decodificador: con las 19 capturas de `wav/` las tramas generadas (datos, flags, fase y muestras de inicio y fin) son idénticas byte a byte antes y después, tanto con todas las tecnologías activas como con solo NFC-A.

1. **Una división por muestra en lugar de tres.** `correlationPoints()` calcula `filterPoint1 = signalIndex % P1` y deriva los otros dos puntos con una comparación y una resta (`point2 = p1 + P2`, restando P1 si se pasa; `point3 = p1 − 1` con vuelta a `P1 − 1`). Se usa en detección y en los demoduladores ASK.
2. **Inversa precalculada.** `period2Inverse[rate] = 1 / P2` se calcula en `initialize()`, y la normalización de `SD` pasa a ser una multiplicación.
3. **Copias locales en `detectModulation`.** `signalClock` y el puntero de debug se leen una vez por llamada. Antes el compilador tenía que releerlos de memoria en cada uso, porque las escrituras en `NfcModulationStatus` podían solaparse con ellos.
4. **Paridad sin bucle.** `checkParity` usa un plegado XOR de 8 bits, portable a MSVC.
5. **Código duplicado eliminado.**
   - `resetSearch(modulation)` sustituye a los 5 bloques de reinicio de búsqueda, incluido `resetFrameSearch`. Algunos bloques no limpiaban `searchPulseWidth` ni `symbolRiseTime`; ambos se escriben siempre antes de leerse, así que limpiarlos no cambia el resultado.
   - `buildFrame(type, rate, flags)` sustituye a los 3 bloques que construían un `RawFrame`.

**Rendimiento medido** (mejor de 40 repeticiones, 8 rondas intercaladas, en una máquina compartida con bastante ruido):

| Escenario | Mejora media |
|---|---|
| Solo NFC-A, `test_NFC-A_106kbps_424kbps_001` | ≈ +14 % |
| Solo NFC-A, `test_NFC-F_212kbps_003` (detector NFC-A sin tramas A) | ≈ +16 % |
| Las cuatro tecnologías activas | ≈ +5–10 % (B/F/V y `nextSample` siguen igual) |

### 9.2 Descartadas o pendientes

- **Índices circulares incrementales** (un contador por velocidad en lugar de `%`): se probó y resultó **más lento** (~5 %). Mantener y validar el estado del contador cuesta más que una división entera en una CPU actual.
- **Simplificar el correlador a `SD = 2·S0`** (E7): cambiaría `SD` en el término `s[t] − s[t−P/2]` y, con ello, los resultados. Antes hay que decidir si el comportamiento actual es intencionado.
- **Salida temprana en detección**: el camino sin señal ya se reduce a unas pocas comparaciones, así que la ganancia esperada es pequeña.
- **Reinicios más baratos**: `resetModulation()` limpia 3 × 8 KB y `decodePollFrame` hace un `memset` de 8 KB. `integrationData` se indexa con el buffer completo y se leen valores antiguos tras el reinicio, así que limpiar menos cambiaría la salida. Se ejecuta una vez por trama, por lo que no compensa.
- **Debug en tiempo de compilación** (plantilla o `constexpr`): la comprobación ya es una sola rama muy predecible tras la copia local.
- Fuera del alcance de NFC-A: `NfcDecoderStatus::nextSample()` supone ~30 % del tiempo (con una división por `signalEnvelope` en cada muestra) y es común a todas las tecnologías.

## 10. Pruebas

En `wav/` hay capturas de referencia (`test_*.wav`) con su salida esperada (`*.json`). Cualquier cambio en el demodulador debería validarse comparando las tramas decodificadas con esos ficheros antes y después del cambio. Con el código actual, las 19 capturas reproducen exactamente sus `.json`.
