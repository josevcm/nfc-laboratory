#ifndef LMS_GOERTZEL_H
#define LMS_GOERTZEL_H

namespace lime{
class IConnection;
}

int SelectGoertzelBin(lime::IConnection* port, uint16_t bin, uint16_t samplesCount);
int CalculateGoertzelBin(lime::IConnection *dataPort, int64_t *real, int64_t *imag);

#endif //LMS_GOERTZEL_H
