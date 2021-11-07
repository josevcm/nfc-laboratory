/*
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

#define PORT_NUM_INVALID (255)
#define PIN_NUM_INVALID  (255)

#define PORT_NUM_MIN (0)
#define PORT_NUM_MAX (7)

#define PIN_NUM_MIN (0)
#define PIN_NUM_MAX (31)

static void usage() {
	printf("WARNING this tool reconfigure GPIO Direction IN/OUT and can destroy GPIO/MCU in case of mistake\n");
	printf("Usage:\n");
	printf("\t-p, --port_no <p>: set port number<p>[0,7] for subsequent read/write operations\n");
	printf("\t-n, --pin_no <n>: set pin number<n>[0,31] for subsequent read/write operations\n");
	printf("\t-r, --read: read port number/pin number direction specified by last -n argument, or all port/pin\n");
	printf("\t-w, --write <v>: write value port direction specified by last -n argument with value<v>[0,1](0=IN,1=OUT)\n");
	printf("\t[-s serial_number_64bits]: Open board with specified 64bits serial number.\n");
	printf("\nExamples:\n");
	printf("\t<command> -p 0 -n 12 -r # reads gpio direction from port 0 pin number 12\n");
	printf("\t<command> -r          # reads gpio direction on all pins and all ports\n");
	printf("\t<command> -p 0 -n 10 -w 1 # writes gpio direction port 0 pin number 10 with 1(output) decimal\n");
}

static struct option long_options[] = {
	{ "port_no", required_argument, 0, 'p' },
	{ "pin_no", required_argument, 0, 'n' },
	{ "write", required_argument, 0, 'w' },
	{ "read", no_argument, 0, 'r' },
	{ 0, 0, 0, 0 },
};

int parse_u8(char* const s, uint8_t* const value) {
	char* s_end = s;
	const long int long_value = strtol(s, &s_end, 10);
	if( (s != s_end) && (*s_end == 0) ) {
		if((long_value >=0 ) && (long_value < 256)) {
			*value = (uint8_t)long_value;
			return AIRSPY_SUCCESS;
		} else {
			return AIRSPY_ERROR_INVALID_PARAM;
		}
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

int dump_port_pin(struct airspy_device* device,
									airspy_gpio_port_t port_number,
									airspy_gpio_pin_t pin_number)
{
	uint8_t value;
	int result;

	result = airspy_gpiodir_read(device, port_number, pin_number, &value);
	if( result == AIRSPY_SUCCESS ) {
		if(value == 1)
			printf("gpiodir[%1d][%2d] -> out(1)\n", port_number, pin_number);
		else
			printf("gpiodir[%1d][%2d] -> in(0)\n", port_number, pin_number);
	} else {
		printf("airspy_gpiodir_read() failed: %s (%d)\n", airspy_error_name(result), result);
	}
	return result;
}

int dump_port(struct airspy_device* device, airspy_gpio_port_t port_number)
{
	airspy_gpio_pin_t pin_number;
	int result = AIRSPY_SUCCESS;

	for(pin_number = GPIO_PIN0; pin_number < (GPIO_PIN31+1); pin_number++)
	{
		result = dump_port_pin(device, port_number, pin_number);
	}
	return result;
}

int dump_ports(struct airspy_device* device)
{
	uint8_t port_number;
	int result = AIRSPY_SUCCESS;

	for(port_number = GPIO_PORT0; port_number < (GPIO_PORT7+1); port_number++)
	{
		result = dump_port(device, port_number);
		if( result != AIRSPY_SUCCESS ) {
			break;
		}
	}
	return result;
}

int write_port_pin(struct airspy_device* device,
										airspy_gpio_port_t port_number,
										airspy_gpio_pin_t pin_number,
										uint8_t value)
{
	int result;
	result = airspy_gpiodir_write(device, port_number, pin_number, value);

	if( result == AIRSPY_SUCCESS ) {
		printf("0x%02X -> gpiodir[%1d][%2d]\n", value, port_number, pin_number);
	} else {
		printf("airspy_gpiodir_write() failed: %s (%d)\n", airspy_error_name(result), result);
	}

	return result;
}

bool serial_number = false;
uint64_t serial_number_val;

int main(int argc, char** argv) {
	int opt;
	uint8_t port_number = PORT_NUM_INVALID;
	uint8_t pin_number = PIN_NUM_INVALID;
	uint8_t value;
	struct airspy_device* device = NULL;
	int option_index;
	uint32_t serial_number_msb_val;
	uint32_t serial_number_lsb_val;
	int result;

	option_index = 0;
	while( (opt = getopt_long(argc, argv,  "p:n:rw:s:", long_options, &option_index)) != EOF )
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
	while( (opt = getopt_long(argc, argv, "p:n:rw:", long_options, &option_index)) != EOF )
	{
		switch( opt ) {
		case 'p':
			result = parse_u8(optarg, &port_number);
			if((result != AIRSPY_SUCCESS) || (port_number > PORT_NUM_MAX))
			{
				printf("Error parameter -p shall be between %d and %d\n", PORT_NUM_MIN, PORT_NUM_MAX);
				result = AIRSPY_ERROR_OTHER;
			}
			break;

		case 'n':
			result = parse_u8(optarg, &pin_number);
			if((result != AIRSPY_SUCCESS) || (pin_number > PIN_NUM_MAX))
			{
				printf("Error parameter -n shall be between %d and %d\n", PIN_NUM_MIN, PIN_NUM_MAX);
				result = AIRSPY_ERROR_OTHER;
			}
			break;

		case 'r':
			if( port_number == PORT_NUM_INVALID )
			{
				result = dump_ports(device);
			} else
			{
				if( pin_number == PORT_NUM_INVALID )
				{
					result = dump_port(device, port_number);
				} else
				{
					result = dump_port_pin(device, port_number, pin_number);
				}
			}
			if( result != AIRSPY_SUCCESS )
				printf("argument error: %s (%d)\n", airspy_error_name(result), result);
			break;

		case 'w':
			result = parse_u8(optarg, &value);
			if( result == AIRSPY_SUCCESS ) {
				result = write_port_pin(device, port_number, pin_number, value);
				if( result != AIRSPY_SUCCESS )
					printf("argument error: %s (%d)\n", airspy_error_name(result), result);
			}
			break;
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

