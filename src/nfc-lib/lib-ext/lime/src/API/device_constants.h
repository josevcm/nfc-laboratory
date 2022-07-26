#ifndef CONSTANTS_H
#define	CONSTANTS_H


namespace lime
{
    const double cgenMax = 640e6;
    
    namespace program_mode
    {
        const auto autoUpdate = "Automatic";
        const auto fx3Flash = "FX3 FLASH";
        const auto fx3RAM = "FX3 RAM";
        const auto fx3Reset = "FX3 Reset";       
        const auto fpgaFlash = "FPGA FLASH";
        const auto fpgaRAM = "FPGA RAM";
        const auto fpgaReset = "FPGA Reset";   
        const auto mcuRAM = "LMSMCU SRAM";
        const auto mcuEEPROM = "LMSMCU EEPROM";
        const auto mcuReset = "LMSMCU Reset";  
    }
    
}


#endif	

