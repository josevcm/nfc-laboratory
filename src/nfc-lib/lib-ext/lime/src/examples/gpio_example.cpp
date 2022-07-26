/**
    @file   gpio_example.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  GPIO example
 */
#include <lime/LimeSuite.h>
#include <iostream>
#include <chrono>
#include <thread>

//Device structure, should be initialize to NULL
lms_device_t* device = NULL;

int error()
{
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}

void print_gpio(int gpio_val)
{
    for (int i = 0; i < 8; i++)
    {
        bool set = gpio_val&(1<<i); 
        std::cout << "GPIO" << i <<": " << (set ? "High" : "Low") << std::endl;
    }
}

int main(int argc, char** argv)
{
    //Find devices
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        error();

    std::cout << "Devices found: " << n << std::endl; //print number of devices
    if (n < 1)
        return -1;

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
        error();

    //Read current GPIO pins state using LMS_GPIORead()
    std::cout << "Read current GPIO state:" << std::endl;
    uint8_t gpio_val = 0;
    if (LMS_GPIORead(device, &gpio_val, 1)!=0) //1 byte buffer is enough to read 8 GPIO pins on LimeSDR-USB
        error();
    print_gpio(gpio_val);

    //change GPIO pins direction using LMS_GPIODirWrite()
    std::cout << std::endl << "Set GPIO0, GPIO1, GPIO2 GPIO3 and GPIO7 to output:" << std::endl;
    uint8_t gpio_dir = 0x8F; //set bits 0,1,2,3 and 7
    if (LMS_GPIODirWrite(device, &gpio_dir, 1)!=0) //1 byte buffer is enough to configure 8 GPIO pins on LimeSDR-USB
        error();
    std::cout << "Read GPIO:" << std::endl;
    if (LMS_GPIORead(device, &gpio_val, 1)!=0)
        error();
    print_gpio(gpio_val);
  
    //set GPIO pins output level using LMS_GPIOWrite(). Only affect GPIO that configured as output
    std::cout << std::endl << "Set GPIO0, GPIO7 output to High level:" << std::endl;
    gpio_val = 0x81;
    if (LMS_GPIOWrite(device, &gpio_val, 1)!=0) //1 byte buffer is enough to set 8 GPIO pins on LimeSDR-USB
        error();
    std::cout << "Read GPIO:" << std::endl;
    if (LMS_GPIORead(device, &gpio_val, 1)!=0)
        error();
    print_gpio(gpio_val);
    
    //togle single GPIO for some time (~16s)
    std::cout << std::endl << "Toggle GPIO0" << std::endl;
    for (int i = 0; i < 16; i++)
    {
        gpio_val = 0x81;
        if (LMS_GPIOWrite(device, &gpio_val, 1)!=0)
            error();
        gpio_val = 0x80;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (LMS_GPIOWrite(device, &gpio_val, 1)!=0)
            error();  
        std::this_thread::sleep_for(std::chrono::milliseconds(500));      
    }
       
    return 0;
}
