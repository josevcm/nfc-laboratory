/**
    @file ConnectionSTREAM_UNITE.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM+UNITE board connection.
*/

#include "ConnectionSTREAM_UNITE.h"
using namespace std;
namespace lime
{

ConnectionSTREAM_UNITE::ConnectionSTREAM_UNITE(void* ctx, const std::string &vidpid, const std::string &serial, const unsigned index, const char* comPortName)
    : ConnectionFX3(ctx, vidpid, serial, index), comPort(nullptr)
{
    if(comPortName && strlen(comPortName))
    {
        comPort = new ConnectionEVB7COM(comPortName, 9600);
        if(comPort->IsOpen() == false)
        {
            delete comPort;
            comPort = nullptr;
            return;
        }
    }
}

ConnectionSTREAM_UNITE::~ConnectionSTREAM_UNITE(void)
{
    if(comPort)
        delete comPort;
}

DeviceInfo ConnectionSTREAM_UNITE::GetDeviceInfo(void)
{
    DeviceInfo usbInfo = ConnectionFX3::GetDeviceInfo();
    if(comPort)
    {
        DeviceInfo comInfo;
        comInfo = comPort->GetDeviceInfo();
        usbInfo.deviceName += std::string("+")+comInfo.deviceName;
    }
    return usbInfo;
}

int ConnectionSTREAM_UNITE::TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size)
{
    if(comPort && (addr == 0x10 || addr == 0x30)) //redirect LMS7002M SPI, ADF4002
    {
        return comPort->TransactSPI(addr, writeData, readData, size);
    }
    else
        return ConnectionFX3::TransactSPI(addr, writeData, readData, size);
}

int ConnectionSTREAM_UNITE::DeviceReset(int ind)
{
    if(comPort)
    {
        int status = comPort->DeviceReset();
        if(status != 0)
            return status;
    }
    return ConnectionFX3::DeviceReset();
}

int ConnectionSTREAM_UNITE::TransferPacket(GenericPacket &pkt)
{
    if(comPort && (pkt.cmd == CMD_PROG_MCU ||
                   pkt.cmd == CMD_LMS7002_WR ||
                   pkt.cmd == CMD_LMS7002_RD ))
        return comPort->TransferPacket(pkt);
    else
        return ConnectionFX3::TransferPacket(pkt);
}

}
