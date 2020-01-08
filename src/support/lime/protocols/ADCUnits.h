/**
@file	ADCUnits.h
@author Lime Microsystems
@brief	enumerations of ADC sensor units
*/
#pragma once

namespace lime {

enum eADC_UNITS
{
	RAW,
	VOLTAGE,
	CURRENT,
	RESISTANCE,
	POWER,
	TEMPERATURE,
	ADC_UNITS_COUNT
};

static const char  adc_units_text[][8] = {"", "V", "A", "Ohm", "W", "C"};

static const char* adcUnits2string(const unsigned units)
{
	if (units < ADC_UNITS_COUNT)
		return adc_units_text[units];
	else
		return " unknown";
}

}
