/***********************************************************************
 * This is a collection of all built-in connections that are included
 * with the library. Additional connections can be added dynamically.
 **********************************************************************/

#define ENABLE_FTDI

void __loadConnectionEVB7COMEntry(void);
void __loadConnectionFX3Entry(void);
void __loadConnectionSTREAM_UNITEEntry(void);
void __loadConnectionNovenaRF7Entry(void);
void __loadConnectionFT601Entry(void);
void __loadConnectionXillybusEntry(void);
void __loadConnectionRemoteEntry(void);

void __loadAllConnections(void)
{
    #ifdef ENABLE_EVB7COM
    __loadConnectionEVB7COMEntry();
    #endif

    #ifdef ENABLE_FX3
    __loadConnectionFX3Entry();
    #endif

    #ifdef ENABLE_STREAM_UNITE
    __loadConnectionSTREAM_UNITEEntry();
    #endif

    #ifdef ENABLE_FTDI
    __loadConnectionFT601Entry();
    #endif

    #ifdef ENABLE_NOVENARF7
    __loadConnectionNovenaRF7Entry();
    #endif

    #ifdef ENABLE_PCIE_XILLYBUS
    __loadConnectionXillybusEntry();
    #endif

    #ifdef ENABLE_REMOTE
    __loadConnectionRemoteEntry();
    #endif
}
