//****************************************************************************
//   GOERTZEL ALGORITHM       For complex input data
//****************************************************************************
#include <iostream>
#include "ConnectionRegistry/IConnection.h"
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <chrono>
#include "Logger.h"

using namespace std;
using namespace lime;

static const double PI = 3.14159265359;

static const uint16_t C_COS_ADDR_MSB   = 0x0081;   // FPGA SPI cosine[31:16] register address
static const uint16_t C_COS_ADDR_LSB   = 0x0082;   // FPGA SPI cosine[15:0] register address
static const uint16_t C_CTRL_ADDR      = 0x0080;   // FPGA SPI control register address
static const uint16_t C_INT_ADDR_START = 0x0083;   // FPGA SPI internal register starting address
static const uint16_t C_INT_ADDR_STOP  = 0x008E;   // FPGA SPI internal register ending address

static const int C_TRIG_LEN = 32;   // cosine/sine register length in FPGA

static int32_t cos_value = 1;
static int32_t sin_value = 0;

void getGoertzelCoefficients(int k, int SP, int32_t *c, int32_t *s)
{
    float phi, wn; // algorithm variables for sine and cosine computation
    wn = PI/SP;
    phi = wn * k;

    if(c)
        *c =  cos(phi) * (~(1 << (C_TRIG_LEN-1)) );
    if(s)
        *s =  sin(phi) * (~(1 << (C_TRIG_LEN-1)) );
}

int loadGoertzelCoefficients(IConnection *dataPort, int32_t c)
{
    const uint16_t c_msb = (c >> 16) & 0xFFFF;
    const uint16_t c_lsb =  c & 0x0000FFFF;
    //dataPort->WriteRegister(C_COS_ADDR_MSB, c_msb);
    //dataPort->WriteRegister(C_COS_ADDR_LSB, c_lsb);
    const uint32_t addrs[] = {C_COS_ADDR_MSB, C_COS_ADDR_LSB};
    const uint32_t values[] = {c_msb, c_lsb};
    return dataPort->WriteRegisters(addrs, values, 2);
}

/** @brief Retrieves Goertzel computed I and Q values for the k-th bin from the FPGA.
    @param port - LMS7002M data port for communications with FPGA
    @param real - real value of the k-th bin
    @param imag - imag value of the k-th bin
    @param c    - cosine value
    @param s    - sine value
    Note: FPGA has to be streaming samples to the FX3.
*/
int readGoertzelValues(IConnection *port, int64_t *real, int64_t *imag, int32_t c, int32_t s)
{
    // read Goertzel values from the HW
    std::vector<int64_t> lr;  // lr1 lr2 li1 li2
    int64_t l_msb, l_mid, l_lsb;

    vector<uint32_t> addrs;
    for(int addr=C_INT_ADDR_START; addr<=C_INT_ADDR_STOP; ++addr)
        addrs.push_back(addr);

    vector<uint32_t> values(addrs.size(), 0);
    int status = port->ReadRegisters(addrs.data(), values.data(), addrs.size());
    if(status != 0)
        return ReportError(EIO, "readGoertzelValues: Failed to read values");

    for(unsigned int i=0; i<values.size(); i+=3)
    {
        l_msb = values[i];
        l_mid = values[i+1];
        l_lsb = values[i+2];
        l_msb = (l_msb << 48);
        l_msb = (l_msb >> 16) & 0xFFFFFFFF00000000;
        l_mid = (l_mid << 48);
        l_mid = (l_mid >> 32) & 0x00000000FFFF0000;
        l_lsb = (l_lsb) & 0x000000000000FFFF;
        l_msb += l_mid;
        l_msb += l_lsb;
        lr.push_back(l_msb);
    }
    if(real)
        *real = int64_t((int64_t(c)*lr[0]) >> (C_TRIG_LEN-1)) - lr[1] - ((int64_t(s) * lr[2]) >> (C_TRIG_LEN-1));
    if(imag)
        *imag = int64_t((int64_t(c)*lr[2]) >> (C_TRIG_LEN-1)) - lr[3] + ((int64_t(s) * lr[0]) >> (C_TRIG_LEN-1));
    return status;
}

int computeGoertzel(IConnection *dataPort)
{
    int status;
    uint16_t regValue = 0;
    /*status = dataPort->ReadRegister(C_CTRL_ADDR, regValue);
    if(status != 0)
        return ReportError(EIO, "computeGoertzel: failed to read control register");
    */

    uint32_t addrs[] = {C_CTRL_ADDR, C_CTRL_ADDR};
    uint32_t values[] = {regValue & uint32_t(~0x1), regValue | uint32_t(0x1)};

    status = dataPort->WriteRegisters(addrs, values, 2);
    if(status != 0)
        return ReportError(EIO, "computeGoertzel: failed to write control register");

    bool busy = true;
    const auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    while(busy && (t2-t1 < std::chrono::seconds(1)))
    {
        uint16_t busyCheck = 0;
        dataPort->ReadRegister(C_CTRL_ADDR, busyCheck);
        busyCheck = busyCheck & 0x0002;
        if(busyCheck != 0x0002)
            busy = false;
        t2 = std::chrono::high_resolution_clock::now();
    }
    if(t2-t1 >= std::chrono::seconds(1))
        return ReportError(EBUSY, "computeGoertzel: timeout");
    /*status = dataPort->WriteRegister(C_CTRL_ADDR, regValue & ~0x1);
    if(status != 0)
        return ReportError(EIO, "computeGoertzel: failed to write control register");
    */
    return 0;
}

int64_t c64to48(int64_t x);

static int64_t resize_int(int64_t x, int16_t n)
{
    //int64_t mask = ~(0xFFFFFFFFFFFFFFFF << (n-1));
    //int64_t sign = (x >> (n+1)) & ~mask;
    // print out masks
    /*cout << hex << setw(16) << right << setfill('0') << sign << " | ";
    cout << hex << setw(16) << right << setfill('0') << mask << endl;*/
    return x; //sign + (mask & x);
}

#include <string.h>
void checkForMinMax(int64_t lr, int64_t & lr_min, int64_t & lr_max)
{
    if(lr < lr_min)
        lr_min = lr;
    if(lr > lr_max)
        lr_max = lr;
}
static int fftBin;

void CalcGoertzelI(int x[][2], int64_t real[], int64_t imag[], int Sp)
{
  const int16_t a = 0;
  const int16_t b = 0;
  const int16_t register_length = 64;
  const int16_t trig_length = 32;
  const int16_t mul_length  = register_length+trig_length-a-b;
  const int16_t add_length  = register_length+1;

  memset(real, Sp, sizeof(int64_t)*Sp);
  memset(imag, Sp, sizeof(int64_t)*Sp);

  #ifdef DEBUG_MODE
      int64_t lr1_min, lr1_max;
      int64_t li1_min, li1_max;
      int64_t lr2_min, lr2_max;
      int64_t li2_min, li2_max;

      lr1_min = LLONG_MAX;
      lr1_max = LLONG_MIN;
      li1_min = LLONG_MAX;
      li1_max = LLONG_MIN;
      lr2_min = LLONG_MAX;
      lr2_max = LLONG_MIN;
      li2_min = LLONG_MAX;
      li2_max = LLONG_MIN;
  #endif
  int32_t c, s;     // cosine and sine 16 bits registers
  int32_t x_n;      // sample (bin)

  int64_t lr1, lr2, li1, li2, temp; // algorithm registers

  int64_t mul;
  float phi, wn;    // algorithm variables for sine and cosine computation
  int n;         // loop variables

  // Prepare for computation
  wn = PI/Sp;
  // Loop through all the bins
  //for(k=0; k<Sp; k++)
  /*if(fftBin != 0)
    k = fftBin-1;
  else*/
    const int span = 60;
for(int k = fftBin-span; k <= fftBin+span; k++)
  //k = fftBin;
  {
      if(k < 0)
        continue;
    if(k>= Sp)
        break;
    lr1 = lr2 = 0;
    li1 = li2 = 0;
    temp = 0;
    mul = 0;
    // Precompute the constants for the current bin
    phi = wn * k;
    c = cos(phi) * (~(1 << (trig_length-1)) );
    s = sin(phi) * (~(1 << (trig_length-1)) );
    // Emulate data shift
    for(n=0; n<Sp; n++)
    {
      // first channel
      x_n = x[n][0];

      temp = resize_int(lr1, register_length);

      mul  = (c >> a) * (lr1 >> b);
      mul  = resize_int(mul, mul_length);
      mul  = (mul >> (trig_length-a-b-1)) << 1;

      lr1 = resize_int(mul, add_length) - resize_int(lr2, add_length) + resize_int(x_n, add_length);
      lr2 = temp;
      #ifdef DEBUG_MODE
        checkForMinMax(lr1, lr1_min, lr1_max);
        checkForMinMax(lr2, lr2_min, lr2_max);
      #endif
      lr1 = resize_int(lr1, register_length);

      // second channel
      x_n = x[n][1];

      temp = resize_int(li1, register_length);

      mul  = (c >> a) * (li1 >> b);
      mul  = resize_int(mul, mul_length);
      mul  = (mul >> (trig_length-a-b-1)) << 1;

      li1 = resize_int(mul, add_length) - resize_int(li2, add_length) + resize_int(x_n, add_length);
      li2 = temp;
      #ifdef DEBUG_MODE
        checkForMinMax(li1, li1_min, li1_max);
        checkForMinMax(li2, li2_min, li2_max);
      #endif
      li1 = resize_int(li1, register_length);
    }
    real[k] = (int64_t)( resize_int( resize_int( (c >> a)*(lr1 >> b), mul_length) >> (trig_length-a-b-1), add_length) - resize_int(lr2, add_length) - resize_int( resize_int((s >> a) * (li1 >> b), mul_length )>> (trig_length-a-b-1), add_length) );
    imag[k] = (int64_t)( resize_int( resize_int( (c >> a)*(li1 >> b), mul_length) >> (trig_length-a-b-1), add_length) - resize_int(li2, add_length) + resize_int( resize_int((s >> a) * (lr1 >> b), mul_length )>> (trig_length-a-b-1), add_length) );
  }
  #ifdef DEBUG_MODE
    /*
    cout << lr1_min << " | " << lr1_max << endl;
    cout << li1_min << " | " << lr1_max << endl;
    cout << lr2_min << " | " << lr1_max << endl;
    cout << li2_min << " | " << lr1_max << endl;
    */
  #endif
}
/* Goertzel Algorithm Implementation, float numbers*/
void CalcGoertzelF(int x[][2], float real[], float imag[], int Sp)
{
  double lr1, lr2, li1, li2, temp;
  double c, s, phi, wn;
  int n, k;

    for (int i = 0; i < Sp; i++){
        real[i] = 0.0;
        imag[i] = 0.0;
    }
  wn = PI/Sp;

  // Loop through all the bins
  int span = 60;
  for(k=fftBin-span; k<fftBin+span; k++)
  //k = fftBin;
  {
      if(k < 0)
        continue;
      if(k >= Sp)
        break;
    lr1 = lr2 = 0;
    li1 = li2 = 0;

    // Precompute the constants for the current bin
    phi = wn * k;
    c = cos(phi);
    s = sin(phi);

    // Emulate data shift
    for(n=0; n<Sp; n++)
    {
      temp = lr1;
      lr1 = 2.0*c*lr1 - lr2 + ((float)x[n][0]);
      lr2 = temp;

      temp = li1;
      li1 = 2.0*c*li1 - li2 + ((float)x[n][1]);
      li2 = temp;

      //if(k == 100) fprintf(fp, "%15f, %15f, %15f, %15f\n", lr1, lr2, li1, li2);
    }

    // Compute real and imaginary parts for the current bin
    real[k] = c*lr1 - lr2 - s*li1;
    imag[k] = c*li1 - li2 + s*lr1;
  }

  //fclose(fp);
}


int SelectGoertzelBin(IConnection* port, uint16_t bin, uint16_t samplesCount)
{
    getGoertzelCoefficients(bin, samplesCount, &cos_value, &sin_value);
    int status = loadGoertzelCoefficients(port, cos_value);
    if(status != 0)
        return status;
    return 0;
}

int CalculateGoertzelBin(IConnection *port, int64_t *real, int64_t *imag)
{
    if(port == nullptr)
        return ReportError(EINVAL, "CalculateGoertzelBin: port == nullptr");
    int64_t real_hw =0;
    int64_t imag_hw = 0;
    int status = computeGoertzel(port);
    if(status != 0)
        return status;
    status = readGoertzelValues(port, &real_hw, &imag_hw, cos_value, sin_value);
    if(status != 0)
        return status;
    if(real)
        *real = real_hw;
    if(imag)
        *imag = imag_hw;
    return 0;
}
