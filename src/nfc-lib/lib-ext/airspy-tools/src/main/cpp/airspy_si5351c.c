/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013/2014 Benjamin Vernoux <bvernoux@airspy.com>
 *
 * This file is part of AirSpy (based on HackRF project).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <airspy.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

static void usage() {
	printf("\nUsage:\n");
	printf("\t-c, --config: print textual configuration information\n");
	printf("\t-n, --register <n>: set register number for subsequent read/write operations\n");
	printf("\t-r, --read: read register specified by last -n argument, or all registers\n");
	printf("\t-w, --write <v>: write register specified by last -n argument with value <v>\n");
	printf("\t[-s serial_number_64bits]: Open board with specified 64bits serial number.\n");
	printf("\nExamples:\n");
	printf("\t<command> -n 12 -r    # reads from register 12\n");
	printf("\t<command> -r          # reads all registers\n");
	printf("\t<command> -n 10 -w 22 # writes register 10 with 22 decimal\n");
}

static struct option long_options[] = {
	{ "config", no_argument, 0, 'c' },
	{ "register", required_argument, 0, 'n' },
	{ "write", required_argument, 0, 'w' },
	{ "read", no_argument, 0, 'r' },
	{ "serial", required_argument, 0, 's' },
	{ 0, 0, 0, 0 },
};

int parse_int(char* const s, uint8_t* const value) {
	char* s_end = s;
	const long long_value = strtol(s, &s_end, 10);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = (uint8_t)long_value;
		return AIRSPY_SUCCESS;
	} else {
		return AIRSPY_ERROR_INVALID_PARAM;
	}
}

int parse_u64(char* s, uint64_t* const value) {
	uint_fast8_t base = 10;
	char* s_end;
	uint64_t u64_value;

	if( strlen(s) > 2 ) {
		if( s[0] == '0' ) {
			if( (s[1] == 'x') || (s[1] == 'X') ) {
				base = 16;
				s += 2;
			} else if( (s[1] == 'b') || (s[1] == 'B') ) {
				base = 2;
				s += 2;
			}
		}
	}

	s_end = s;
	u64_value = strtoull(s, &s_end, base);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = u64_value;
		return AIRSPY_SUCCESS;
	} else {
		return AIRSPY_ERROR_INVALID_PARAM;
	}
}

int dump_register(struct airspy_device* device, const uint8_t register_number) {
	uint8_t register_value;
	int result;

	result = airspy_si5351c_read(device, register_number, &register_value);
	
	if( result == AIRSPY_SUCCESS ) {
		printf("[%3d] -> 0x%02x\n", register_number, register_value);
	} else {
		printf("airspy_si5351c_read() failed: %s (%d)\n", airspy_error_name(result), result);
	}
	
	return result;
}

int dump_registers(struct airspy_device* device) {
	int register_number;
	int result = AIRSPY_SUCCESS;
	
	for(register_number=0; register_number<256; register_number++) {
		result = dump_register(device, (uint8_t)register_number);
		if( result != AIRSPY_SUCCESS ) {
			break;
		}
	}
	
	return result;
}

int write_register(	struct airspy_device* device,	const uint8_t register_number, const uint8_t register_value)
{
	int result = AIRSPY_SUCCESS;
	result = airspy_si5351c_write(device, register_number, register_value);
	
	if( result == AIRSPY_SUCCESS ) {
		printf("0x%2x -> [%3d]\n", register_value, register_number);
	} else {
		printf("airspy_si5351c_write() failed: %s (%d)\n", airspy_error_name(result), result);
	}
	
	return result;
}

int dump_multisynth_config(struct airspy_device* device, const uint_fast8_t ms_number) {
	uint_fast8_t i;
	uint_fast8_t reg_base;
	uint8_t parameters[8];
	uint32_t p1,p2,p3,r_div;
	uint_fast8_t div_lut[] = {1,2,4,8,16,32,64,128};

	printf("MS%d:", ms_number);
	if(ms_number <6){
		reg_base = 42 + (ms_number * 8);
		for(i=0; i<8; i++) {
			uint8_t reg_number = reg_base + i;
			int result = airspy_si5351c_read(device, reg_number, &parameters[i]);
			if( result != AIRSPY_SUCCESS ) {
				return result;
			}
		}

		p1 =
			  ((parameters[2] & 0x03) << 16)
			| (parameters[3] << 8)
			| parameters[4]
			;
		p2 =
			  ((parameters[5] & 0x0F) << 16)
			| (parameters[6] << 8)
			| parameters[7]
			;
		p3 =
			  ((parameters[5] & 0xF0) << 12)
			| (parameters[0] << 8)
			|  parameters[1]
			;
		r_div =
			(parameters[2] >> 4) & 0x7
			;

		printf("\tp1 = %u\n", p1);
		printf("\tp2 = %u\n", p2);
		printf("\tp3 = %u\n", p3);
		if(p3)
			printf("\tOutput (800Mhz PLL): %#.10f Mhz\n", ((double)800 / (double)(((double)p1*p3 + p2 + 512*p3)/(double)(128*p3))) / div_lut[r_div] );
	} else {
		// MS6 and 7 are integer only
		unsigned int parms;
		reg_base = 90;

		for(i=0; i<3; i++) {
			uint_fast8_t reg_number = reg_base + i;
			int result = airspy_si5351c_read(device, reg_number, &parameters[i]);
			if( result != AIRSPY_SUCCESS ) {
				return result;
			}
		}

		r_div = (ms_number == 6) ? parameters[2] & 0x7 : (parameters[2] & 0x70) >> 4 ;
		parms = (ms_number == 6) ? parameters[0] : parameters[1];
		printf("\tp1_int = %u\n", parms);
		if(parms)
			printf("\tOutput (800Mhz PLL): %#.10f Mhz\n", (800.0f / parms) / div_lut[r_div] );
	}
	printf("\toutput divider = %u\n", div_lut[r_div]);
	
	return AIRSPY_SUCCESS;
}

int dump_configuration(struct airspy_device* device) {
	uint_fast8_t ms_number;
	int result;
			
	for(ms_number=0; ms_number<8; ms_number++) {
		result = dump_multisynth_config(device, ms_number);
		if( result != AIRSPY_SUCCESS ) {
			return result;
		}
	}
	
	return AIRSPY_SUCCESS;
}

bool serial_number = false;
uint64_t serial_number_val;

int main(int argc, char** argv) {
	int opt;
	int n_opt = 0;
	uint8_t register_number = 0;
	uint8_t register_value;
	struct airspy_device* device = NULL;
	int option_index;
	uint32_t serial_number_msb_val;
	uint32_t serial_number_lsb_val;
	int result;

	option_index = 0;
	while( (opt = getopt_long(argc, argv,  "cn:rw:s:", long_options, &option_index)) != EOF )
	{
		switch( opt ) {

		case 's':
			serial_number = true;
			result = parse_u64(optarg, &serial_number_val);
			serial_number_msb_val = (uint32_t)(serial_number_val >> 32);
			serial_number_lsb_val = (uint32_t)(serial_number_val & 0xFFFFFFFF);
			printf("Board serial number to open: 0x%08X%08X\n", serial_number_msb_val, serial_number_lsb_val);
			break;
		}
	}

	result = airspy_init();
	if( result ) {
		printf("airspy_init() failed: %s (%d)\n", airspy_error_name(result), result);
		return EXIT_FAILURE;
	}

	if(serial_number == true)
	{
		result = airspy_open_sn(&device, serial_number_val);
		if( result != AIRSPY_SUCCESS ) {
			printf("airspy_open_sn() failed: %s (%d)\n", airspy_error_name(result), result);
			usage();
			airspy_exit();
			return EXIT_FAILURE;
		}
	}else
	{
		result = airspy_open(&device);
		if( result != AIRSPY_SUCCESS ) {
			printf("airspy_open() failed: %s (%d)\n", airspy_error_name(result), result);
			usage();
			airspy_exit();
			return EXIT_FAILURE;
		}
	}

	result = AIRSPY_ERROR_OTHER;
	option_index = 0;
	optind = 0;
	while( (opt = getopt_long(argc, argv, "cn:rw:s:", long_options, &option_index)) != EOF ) {
		switch( opt ) {
		case 'n':
			result = parse_int(optarg, &register_number);
			n_opt = 1;
			break;
		
		case 'w':
			result = parse_int(optarg, &register_value);
			if( result == AIRSPY_SUCCESS ) {
				result = write_register(device, register_number, register_value);
			}
			break;
		
		case 'r':
			if( n_opt == 0 ) {
				result = dump_registers(device);
			} else {
				result = dump_register(device, register_number);
			}
			break;
		
		case 'c':
			result = dump_configuration(device);
			break;
		case 's':
			serial_number = true;
			result = parse_u64(optarg, &serial_number_val);
			break;
		default:
			usage();
		}
		
		if( result != AIRSPY_SUCCESS )
		{
			break;
		}
	}

	if( result != AIRSPY_SUCCESS )
	{
		usage();
	}

	result = airspy_close(device);
	if( result ) {
		printf("airspy_close() failed: %s (%d)\n", airspy_error_name(result), result);
		airspy_exit();
		return EXIT_FAILURE;
	}
	
	airspy_exit();
	
	return 0;
}
