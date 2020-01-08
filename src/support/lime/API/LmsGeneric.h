/*
 * File:   LimeSDR.h
 * Author: Ignas J
 *
 * Created on September 18, 2017
 */

#ifndef LMSGENERIC_H
#define	LMSGENERIC_H
#include "lms7_device.h"

namespace lime
{

class LMS7_Generic : public LMS7_Device
{
public:
    LMS7_Generic(lime::IConnection* conn, LMS7_Device *obj = nullptr);
private:
};

}

#endif	/* LMSGENERIC_H */

