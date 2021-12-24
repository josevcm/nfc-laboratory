/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2014-2015 Benjamin Vernoux <bvernoux@airspy.com>
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
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#define AIRSPY_RX_VERSION "1.0.5 23 April 2016"

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#ifdef _WIN32
#include <windows.h>

#ifdef _MSC_VER

#ifdef _WIN64
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif

#define strtoull _strtoui64
#define snprintf _snprintf

int gettimeofday(struct timeval *tv, void* ignored)
{
	FILETIME ft;
	unsigned __int64 tmp = 0;
	if (NULL != tv) {
		GetSystemTimeAsFileTime(&ft);
		tmp |= ft.dwHighDateTime;
		tmp <<= 32;
		tmp |= ft.dwLowDateTime;
		tmp /= 10;
		tmp -= 11644473600000000Ui64;
		tv->tv_sec = (long)(tmp / 1000000UL);
		tv->tv_usec = (long)(tmp % 1000000UL);
	}
	return 0;
}

#endif
#endif

#if defined(__GNUC__)
#include <unistd.h>
#include <sys/time.h>
#endif

#include <signal.h>

#if defined _WIN32
	#define sleep(a) Sleep( (a*1000) )
#endif

#define SAMPLE_SCALE_FLOAT_TO_INT ( (8192.0f) )

#define FLOAT32_EL_SIZE_BYTE (4)	/* 4bytes = 32bit float */
#define INT16_EL_SIZE_BYTE (2)   /* 2bytes = 16bit int */
#define INT12_EL_SIZE_BITS (12)
#define INT8_EL_SIZE_BITS (8)

#define FD_BUFFER_SIZE (16*1024)

#define FREQ_ONE_MHZ (1000000ul)
#define FREQ_ONE_MHZ_U64 (1000000ull)

#define DEFAULT_SAMPLE_TYPE (AIRSPY_SAMPLE_INT16_IQ)

#define DEFAULT_FREQ_HZ (900000000ul) /* 900MHz */

#define DEFAULT_VGA_IF_GAIN (5)
#define DEFAULT_LNA_GAIN (1)
#define DEFAULT_MIXER_GAIN (5)

#define PACKING_MAX (0xffffffff)

#define FREQ_HZ_MIN (24000000ul) /* 24MHz */
#define FREQ_HZ_MAX (1900000000ul) /* 1900MHz (officially 1750MHz) */
#define SAMPLE_TYPE_MAX (AIRSPY_SAMPLE_END-1)
#define BIAST_MAX (1)
#define VGA_GAIN_MAX (15)
#define MIXER_GAIN_MAX (15)
#define LNA_GAIN_MAX (14)
#define LINEARITY_GAIN_MAX (21)
#define SENSITIVITY_GAIN_MAX (21)
#define SAMPLES_TO_XFER_MAX_U64 (0x8000000000000000ull) /* Max value */

#define MIN_SAMPLERATE_BY_VALUE (1000000)

/* WAVE or RIFF WAVE file format containing data for AirSpy compatible with SDR# Wav IQ file */
typedef struct 
{
		char groupID[4]; /* 'RIFF' */
		uint32_t size; /* File size + 8bytes */
		char riffType[4]; /* 'WAVE'*/
} t_WAVRIFF_hdr;

#define FormatID "fmt "   /* chunkID for Format Chunk. NOTE: There is a space at the end of this ID. */

typedef struct {
	char chunkID[4]; /* 'fmt ' */
	uint32_t chunkSize; /* 16 fixed */

	uint16_t wFormatTag; /* 1=PCM8/16, 3=Float32 */
	uint16_t wChannels;
	uint32_t dwSamplesPerSec; /* Freq Hz sampling */
	uint32_t dwAvgBytesPerSec; /* Freq Hz sampling x 2 */
	uint16_t wBlockAlign;
	uint16_t wBitsPerSample;
} t_FormatChunk;

typedef struct 
{
		char chunkID[4]; /* 'data' */
		uint32_t chunkSize; /* Size of data in bytes */
	/* For IQ samples I(16 or 32bits) then Q(16 or 32bits), I, Q ... */
} t_DataChunk;

typedef struct
{
	t_WAVRIFF_hdr hdr;
	t_FormatChunk fmt_chunk;
	t_DataChunk data_chunk;
} t_wav_file_hdr;

t_wav_file_hdr wave_file_hdr = 
{
	/* t_WAVRIFF_hdr */
	{
		{ 'R', 'I', 'F', 'F' }, /* groupID */
		0, /* size to update later */
		{ 'W', 'A', 'V', 'E' }
	},
	/* t_FormatChunk */
	{
		{ 'f', 'm', 't', ' ' }, /* char		chunkID[4];  */
		16, /* uint32_t chunkSize; */
		0, /* uint16_t wFormatTag; to update later */
		0, /* uint16_t wChannels; to update later */
		0, /* uint32_t dwSamplesPerSec; Freq Hz sampling to update later */
		0, /* uint32_t dwAvgBytesPerSec; to update later */
		0, /* uint16_t wBlockAlign; to update later */
		0, /* uint16_t wBitsPerSample; to update later  */
	},
	/* t_DataChunk */
	{
		{ 'd', 'a', 't', 'a' }, /* char chunkID[4]; */
		0, /* uint32_t	chunkSize; to update later */
	}
};

#define U64TOA_MAX_DIGIT (31)
typedef struct 
{
		char data[U64TOA_MAX_DIGIT+1];
} t_u64toa;

receiver_mode_t receiver_mode = RECEIVER_MODE_RX;

uint32_t vga_gain = DEFAULT_VGA_IF_GAIN;
uint32_t lna_gain = DEFAULT_LNA_GAIN;
uint32_t mixer_gain = DEFAULT_MIXER_GAIN;

uint32_t linearity_gain_val;
bool linearity_gain = false;

uint32_t sensitivity_gain_val;
bool sensitivity_gain = false;

/* WAV default values */
uint16_t wav_format_tag=1; /* PCM8 or PCM16 */
uint16_t wav_nb_channels=2;
uint32_t wav_sample_per_sec;
uint16_t wav_nb_byte_per_sample=2;
uint16_t wav_nb_bits_per_sample=16;

airspy_read_partid_serialno_t read_partid_serialno;

volatile bool do_exit = false;

FILE* fd = NULL;

bool verbose = false;
bool receive = false;
bool receive_wav = false;

struct timeval time_start;
struct timeval t_start;

bool got_first_packet = false;
float average_rate = 0.0f;
float global_average_rate = 0.0f;
uint32_t rate_samples = 0;
uint32_t buffer_count = 0;
uint32_t sample_count = 0;
	
bool freq = false;
uint32_t freq_hz;

bool limit_num_samples = false;
uint64_t samples_to_xfer = 0;
uint64_t bytes_to_xfer = 0;

bool call_set_packing = false;
uint32_t packing_val = 0;

bool sample_rate = false;
uint32_t sample_rate_val;

bool sample_type = false;
enum airspy_sample_type sample_type_val = AIRSPY_SAMPLE_INT16_IQ;

bool biast = false;
uint32_t biast_val;

bool serial_number = false;
uint64_t serial_number_val;

static float
TimevalDiff(const struct timeval *a, const struct timeval *b)
{
	return (a->tv_sec - b->tv_sec) + 1e-6f * (a->tv_usec - b->tv_usec);
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

int parse_u32(char* s, uint32_t* const value)
{
	uint_fast8_t base = 10;
	char* s_end;
	uint64_t ulong_value;

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
	ulong_value = strtoul(s, &s_end, base);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = (uint32_t)ulong_value;
		return AIRSPY_SUCCESS;
	} else {
		return AIRSPY_ERROR_INVALID_PARAM;
	}
}

static char *stringrev(char *str)
{
	char *p1, *p2;

	if(! str || ! *str)
		return str;

	for(p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

char* u64toa(uint64_t val, t_u64toa* str)
{
	#define BASE (10ull) /* Base10 by default */
	uint64_t sum;
	int pos;
	int digit;
	int max_len;
	char* res;

	sum = val;
	max_len = U64TOA_MAX_DIGIT;
	pos = 0;

	do
	{
		digit = (sum % BASE);
		str->data[pos] = digit + '0';
		pos++;

		sum /= BASE;
	}while( (sum>0) && (pos < max_len) );

	if( (pos == max_len) && (sum>0) )
		return NULL;

	str->data[pos] = '\0';
	res = stringrev(str->data);

	return res;
}

int rx_callback(airspy_transfer_t* transfer)
{
	uint32_t bytes_to_write;
	void* pt_rx_buffer;
	ssize_t bytes_written;
	struct timeval time_now;
	float time_difference, rate;

	if( fd != NULL ) 
	{
		switch(sample_type_val)
		{
			case AIRSPY_SAMPLE_FLOAT32_IQ:
				bytes_to_write = transfer->sample_count * FLOAT32_EL_SIZE_BYTE * 2;
				pt_rx_buffer = transfer->samples;
				break;

			case AIRSPY_SAMPLE_FLOAT32_REAL:
				bytes_to_write = transfer->sample_count * FLOAT32_EL_SIZE_BYTE * 1;
				pt_rx_buffer = transfer->samples;
				break;

			case AIRSPY_SAMPLE_INT16_IQ:
				bytes_to_write = transfer->sample_count * INT16_EL_SIZE_BYTE * 2;
				pt_rx_buffer = transfer->samples;
				break;

			case AIRSPY_SAMPLE_INT16_REAL:
				bytes_to_write = transfer->sample_count * INT16_EL_SIZE_BYTE * 1;
				pt_rx_buffer = transfer->samples;
				break;

			case AIRSPY_SAMPLE_UINT16_REAL:
				bytes_to_write = transfer->sample_count * INT16_EL_SIZE_BYTE * 1;
				pt_rx_buffer = transfer->samples;
				break;

			case AIRSPY_SAMPLE_RAW:
				if (packing_val)
				{
					bytes_to_write = transfer->sample_count * INT12_EL_SIZE_BITS / INT8_EL_SIZE_BITS;
				}
				else
				{
					bytes_to_write = transfer->sample_count * INT16_EL_SIZE_BYTE * 1;
				}
				pt_rx_buffer = transfer->samples;
				break;

			default:
				bytes_to_write = 0;
				pt_rx_buffer = NULL;
			break;
		}

		gettimeofday(&time_now, NULL);

		if (!got_first_packet)
		{
			t_start = time_now;
			time_start = time_now;
			got_first_packet = true;
		}
		else
		{
			buffer_count++;
			sample_count += transfer->sample_count;
			if (buffer_count == 50)
			{
				time_difference = TimevalDiff(&time_now, &time_start);
				rate = (float) sample_count / time_difference;
				average_rate += 0.2f * (rate - average_rate);
				global_average_rate += average_rate;
				rate_samples++;
				time_start = time_now;
				sample_count = 0;
				buffer_count = 0;
			}
		}

		if (limit_num_samples) {
			if (bytes_to_write >= bytes_to_xfer) {
				bytes_to_write = (int)bytes_to_xfer;
			}
			bytes_to_xfer -= bytes_to_write;
		}

		if(pt_rx_buffer != NULL)
		{
			bytes_written = fwrite(pt_rx_buffer, 1, bytes_to_write, fd);
		}else
		{
			bytes_written = 0;
		}
		if ( (bytes_written != bytes_to_write) || 
				 ((limit_num_samples == true) && (bytes_to_xfer == 0)) 
				)
			return -1;
		else
			return 0;
	}else
	{
		return -1;
	}
}

static void usage(void)
{
	fprintf(stderr, "airspy_rx v%s\n", AIRSPY_RX_VERSION);
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "-r <filename>: Receive data into file\n");
	fprintf(stderr, "-w Receive data into file with WAV header and automatic name\n");
	fprintf(stderr, " This is for SDR# compatibility and may not work with other software\n");
	fprintf(stderr, "[-s serial_number_64bits]: Open device with specified 64bits serial number\n");
	fprintf(stderr, "[-p packing]: Set packing for samples, \n");
	fprintf(stderr, " 1=enabled(12bits packed), 0=disabled(default 16bits not packed)\n");
	fprintf(stderr, "[-f frequency_MHz]: Set frequency in MHz between [%lu, %lu] (default %luMHz)\n",
		FREQ_HZ_MIN / FREQ_ONE_MHZ, FREQ_HZ_MAX / FREQ_ONE_MHZ, DEFAULT_FREQ_HZ / FREQ_ONE_MHZ);
	fprintf(stderr, "[-a sample_rate]: Set sample rate\n");
	fprintf(stderr, "[-t sample_type]: Set sample type, \n");
	fprintf(stderr, " 0=FLOAT32_IQ, 1=FLOAT32_REAL, 2=INT16_IQ(default), 3=INT16_REAL, 4=U16_REAL, 5=RAW\n");
	fprintf(stderr, "[-b biast]: Set Bias Tee, 1=enabled, 0=disabled(default)\n");
	fprintf(stderr, "[-v vga_gain]: Set VGA/IF gain, 0-%d (default %d)\n", VGA_GAIN_MAX, vga_gain);
	fprintf(stderr, "[-m mixer_gain]: Set Mixer gain, 0-%d (default %d)\n", MIXER_GAIN_MAX, mixer_gain);
	fprintf(stderr, "[-l lna_gain]: Set LNA gain, 0-%d (default %d)\n", LNA_GAIN_MAX, lna_gain);
	fprintf(stderr, "[-g linearity_gain]: Set linearity simplified gain, 0-%d\n", LINEARITY_GAIN_MAX);
	fprintf(stderr, "[-h sensivity_gain]: Set sensitivity simplified gain, 0-%d\n", SENSITIVITY_GAIN_MAX);
	fprintf(stderr, "[-n num_samples]: Number of samples to transfer (default is unlimited)\n");
	fprintf(stderr, "[-d]: Verbose mode\n");
}

struct airspy_device* device = NULL;

#ifdef _MSC_VER
BOOL WINAPI
sighandler(int signum)
{
	if (CTRL_C_EVENT == signum) {
		fprintf(stderr, "Caught signal %d\n", signum);
		do_exit = true;
		return TRUE;
	}
	return FALSE;
}
#else
void sigint_callback_handler(int signum) 
{
	fprintf(stderr, "Caught signal %d\n", signum);
	do_exit = true;
}
#endif

#define PATH_FILE_MAX_LEN (FILENAME_MAX)
#define DATE_TIME_MAX_LEN (32)

int main(int argc, char** argv)
{
	int opt;
	char path_file[PATH_FILE_MAX_LEN];
	char date_time[DATE_TIME_MAX_LEN];
	t_u64toa ascii_u64_data1;
	t_u64toa ascii_u64_data2;
	const char* path = NULL;
	int result;
	time_t rawtime;
	struct tm * timeinfo;
	struct timeval t_end;
	float time_diff;
	uint32_t file_pos;
	int exit_code = EXIT_SUCCESS;

	uint32_t count;
	uint32_t packing_val_u32;
	uint32_t *supported_samplerates;
	uint32_t sample_rate_u32;
	uint32_t sample_type_u32;
	double freq_hz_temp;
	char str[20];

	while( (opt = getopt(argc, argv, "r:ws:p:f:a:t:b:v:m:l:g:h:n:d")) != EOF )
	{
		result = AIRSPY_SUCCESS;
		switch( opt ) 
		{
			case 'r':
				receive = true;
				path = optarg;
			break;

			case 'w':
				receive_wav = true;
			 break;

			case 's':
				serial_number = true;
				result = parse_u64(optarg, &serial_number_val);
			break;

			case 'p': /* packing */
				result = parse_u32(optarg, &packing_val_u32);
				switch (packing_val_u32)
				{
					case 0:
					case 1:
						packing_val = packing_val_u32;
						call_set_packing = true;
					break;

					default:
						/* Invalid value will display error */
						packing_val = PACKING_MAX;
						call_set_packing = false;
					break;
				}
			break;

			case 'f':
				freq = true;
				freq_hz_temp = strtod(optarg, NULL) * (double)FREQ_ONE_MHZ;
				if(freq_hz_temp <= (double)FREQ_HZ_MAX)
					freq_hz = (uint32_t)freq_hz_temp;
				else
					freq_hz = UINT_MAX;
			break;

			case 'a': /* Sample rate */
				sample_rate = true;
				result = parse_u32(optarg, &sample_rate_u32);
			break;

			case 't': /* Sample type see also airspy_sample_type */
				result = parse_u32(optarg, &sample_type_u32);
				switch (sample_type_u32)
				{
					case 0:
						sample_type_val = AIRSPY_SAMPLE_FLOAT32_IQ;
						wav_format_tag = 3; /* Float32 */
						wav_nb_channels = 2;
						wav_nb_bits_per_sample = 32;
						wav_nb_byte_per_sample = (wav_nb_bits_per_sample / 8);
					break;

					case 1:
						sample_type_val = AIRSPY_SAMPLE_FLOAT32_REAL;
						wav_format_tag = 3; /* Float32 */
						wav_nb_channels = 1;
						wav_nb_bits_per_sample = 32;
						wav_nb_byte_per_sample = (wav_nb_bits_per_sample / 8);
					break;

					case 2:
						sample_type_val = AIRSPY_SAMPLE_INT16_IQ;
						wav_format_tag = 1; /* PCM8 or PCM16 */
						wav_nb_channels = 2;
						wav_nb_bits_per_sample = 16;
						wav_nb_byte_per_sample = (wav_nb_bits_per_sample / 8);
					break;

					case 3:
						sample_type_val = AIRSPY_SAMPLE_INT16_REAL;
						wav_format_tag = 1; /* PCM8 or PCM16 */
						wav_nb_channels = 1;
						wav_nb_bits_per_sample = 16;
						wav_nb_byte_per_sample = (wav_nb_bits_per_sample / 8);
					break;

					case 4:
						sample_type_val = AIRSPY_SAMPLE_UINT16_REAL;
						wav_format_tag = 1; /* PCM8 or PCM16 */
						wav_nb_channels = 1;
						wav_nb_bits_per_sample = 16;
						wav_nb_byte_per_sample = (wav_nb_bits_per_sample / 8);
					break;

					case 5:
						sample_type_val = AIRSPY_SAMPLE_RAW;
						wav_nb_bits_per_sample = 12;
						wav_nb_channels = 1;
						break;

					default:
						/* Invalid value will display error */
						sample_type_val = SAMPLE_TYPE_MAX+1;
					break;
				}
			break;

			case 'b':
				serial_number = true;
				result = parse_u32(optarg, &biast_val);
			break;

			case 'v':
				result = parse_u32(optarg, &vga_gain);
			break;

			case 'm':
				result = parse_u32(optarg, &mixer_gain);
			break;

			case 'l':
				result = parse_u32(optarg, &lna_gain);
			break;

			case 'g':
				linearity_gain = true;
				result = parse_u32(optarg, &linearity_gain_val);		
			break;

			case 'h':
				sensitivity_gain = true;
				result = parse_u32(optarg, &sensitivity_gain_val);
			break;

			case 'n':
				limit_num_samples = true;
				result = parse_u64(optarg, &samples_to_xfer);
			break;

			case 'd':
				verbose = true;
			break;

			default:
				fprintf(stderr, "unknown argument '-%c %s'\n", opt, optarg);
				usage();
				return EXIT_FAILURE;
		}
		
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "argument error: '-%c %s' %s (%d)\n", opt, optarg, airspy_error_name(result), result);
			usage();
			return EXIT_FAILURE;
		}
	}

	if (sample_rate)
	{
		sample_rate_val = sample_rate_u32;
	}

	bytes_to_xfer = samples_to_xfer * wav_nb_bits_per_sample * wav_nb_channels / 8;

	if (samples_to_xfer >= SAMPLES_TO_XFER_MAX_U64) {
		fprintf(stderr, "argument error: num_samples must be less than %s/%sMio\n",
				u64toa(SAMPLES_TO_XFER_MAX_U64, &ascii_u64_data1),
				u64toa((SAMPLES_TO_XFER_MAX_U64/FREQ_ONE_MHZ_U64), &ascii_u64_data2) );
		usage();
		return EXIT_FAILURE;
	}

	if( freq ) {
		if( (freq_hz >= FREQ_HZ_MAX) || (freq_hz < FREQ_HZ_MIN) )
		{
			fprintf(stderr, "argument error: frequency_MHz=%.6f MHz and shall be between [%lu, %lu[ MHz\n",
							((double)freq_hz/(double)FREQ_ONE_MHZ), FREQ_HZ_MIN/FREQ_ONE_MHZ, FREQ_HZ_MAX/FREQ_ONE_MHZ);
			usage();
			return EXIT_FAILURE;
		}
	}else
	{
		/* Use default freq */
		freq_hz = DEFAULT_FREQ_HZ;
	}

	receiver_mode = RECEIVER_MODE_RX;
	if( receive_wav ) 
	{
		if (sample_type_val == AIRSPY_SAMPLE_RAW)
		{
			fprintf(stderr, "The RAW sampling mode is not compatible with Wave files\n");
			usage();
			return EXIT_FAILURE;
		}

		time (&rawtime);
		timeinfo = localtime (&rawtime);
		receiver_mode = RECEIVER_MODE_RX;
		/* File format AirSpy Year(2013), Month(11), Day(28), Hour Min Sec+Z, Freq kHz, IQ.wav */
		strftime(date_time, DATE_TIME_MAX_LEN, "%Y%m%d_%H%M%S", timeinfo);
		snprintf(path_file, PATH_FILE_MAX_LEN, "AirSpy_%sZ_%ukHz_IQ.wav", date_time, (uint32_t)(freq_hz/(1000ull)) );
		path = path_file;
		fprintf(stderr, "Receive wav file: %s\n", path);
	}

	if( path == NULL ) {
		fprintf(stderr, "error: you shall specify at least -r <with filename> or -w option\n");
		usage();
		return EXIT_FAILURE;
	}

	if(packing_val == PACKING_MAX) {
		fprintf(stderr, "argument error: packing out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if(sample_type_val > SAMPLE_TYPE_MAX) {
		fprintf(stderr, "argument error: sample_type out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if(biast_val > BIAST_MAX) {
		fprintf(stderr, "argument error: biast_val out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if(vga_gain > VGA_GAIN_MAX) {
		fprintf(stderr, "argument error: vga_gain out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if(mixer_gain > MIXER_GAIN_MAX) {
		fprintf(stderr, "argument error: mixer_gain out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if(lna_gain > LNA_GAIN_MAX) {
		fprintf(stderr, "argument error: lna_gain out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if(linearity_gain_val > LINEARITY_GAIN_MAX) {
		fprintf(stderr, "argument error: linearity_gain out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if(sensitivity_gain_val > SENSITIVITY_GAIN_MAX) {
		fprintf(stderr, "argument error: sensitivity_gain out of range\n");
		usage();
		return EXIT_FAILURE;
	}

	if( (linearity_gain == true) && (sensitivity_gain == true) )
	{
		fprintf(stderr, "argument error: linearity_gain and sensitivity_gain are both set (choose only one option)\n");
		usage();
		return EXIT_FAILURE;
	}

	if(verbose == true)
	{
		uint32_t serial_number_msb_val;
		uint32_t serial_number_lsb_val;

		fprintf(stderr, "airspy_rx v%s\n", AIRSPY_RX_VERSION);
		serial_number_msb_val = (uint32_t)(serial_number_val >> 32);
		serial_number_lsb_val = (uint32_t)(serial_number_val & 0xFFFFFFFF);
		if(serial_number)
			fprintf(stderr, "serial_number_64bits -s 0x%08X%08X\n", serial_number_msb_val, serial_number_lsb_val);
		fprintf(stderr, "packing -p %d\n", packing_val);
		fprintf(stderr, "frequency_MHz -f %.6fMHz (%sHz)\n",((double)freq_hz/(double)FREQ_ONE_MHZ), u64toa(freq_hz, &ascii_u64_data1) );
		fprintf(stderr, "sample_type -t %d\n", sample_type_val);
		fprintf(stderr, "biast -b %d\n", biast_val);

		if( (linearity_gain == false) && (sensitivity_gain == false) )
		{
			fprintf(stderr, "vga_gain -v %u\n", vga_gain);
			fprintf(stderr, "mixer_gain -m %u\n", mixer_gain);
			fprintf(stderr, "lna_gain -l %u\n", lna_gain);
		} else
		{
			if( linearity_gain == true)
			{
				fprintf(stderr, "linearity_gain -g %u\n", linearity_gain_val);
			}

			if( sensitivity_gain == true)
			{
				fprintf(stderr, "sensitivity_gain -h %u\n", sensitivity_gain_val);
			}
		}

		if( limit_num_samples ) {
			fprintf(stderr, "num_samples -n %s (%sM)\n",
							u64toa(samples_to_xfer, &ascii_u64_data1),
							u64toa((samples_to_xfer/FREQ_ONE_MHZ), &ascii_u64_data2));
		}
	}

	result = airspy_init();
	if( result != AIRSPY_SUCCESS ) {
		fprintf(stderr, "airspy_init() failed: %s (%d)\n", airspy_error_name(result), result);
		return EXIT_FAILURE;
	}

	if(serial_number == true)
	{
		result = airspy_open_sn(&device, serial_number_val);
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "airspy_open_sn() failed: %s (%d)\n", airspy_error_name(result), result);
			airspy_exit();
			return EXIT_FAILURE;
		}
	}else
	{
		result = airspy_open(&device);
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "airspy_open() failed: %s (%d)\n", airspy_error_name(result), result);
			airspy_exit();
			return EXIT_FAILURE;
		}
	}

	result = airspy_set_sample_type(device, sample_type_val);
	if (result != AIRSPY_SUCCESS) {
		fprintf(stderr, "airspy_set_sample_type() failed: %s (%d)\n", airspy_error_name(result), result);
		airspy_close(device);
		airspy_exit();
		return EXIT_FAILURE;
	}

	airspy_get_samplerates(device, &count, 0);

	supported_samplerates = (uint32_t *) malloc(count * sizeof(uint32_t));
	airspy_get_samplerates(device, supported_samplerates, count);

	if (sample_rate_val <= MIN_SAMPLERATE_BY_VALUE)
	{
		if (sample_rate_val < count)
		{
			wav_sample_per_sec = supported_samplerates[sample_rate_val];
		}
		else
		{
			free(supported_samplerates);
			fprintf(stderr, "argument error: unsupported sample rate\n");
			airspy_close(device);
			airspy_exit();
			return EXIT_FAILURE;
		}
	}
	else
	{
		wav_sample_per_sec = sample_rate_val;
	}

	free(supported_samplerates);

	result = airspy_set_samplerate(device, sample_rate_val);
	if (result != AIRSPY_SUCCESS) {
		fprintf(stderr, "airspy_set_samplerate() failed: %s (%d)\n", airspy_error_name(result), result);
		airspy_close(device);
		airspy_exit();
		return EXIT_FAILURE;
	}

	if (verbose)
	{
		fprintf(stderr, "sample_rate -a %d (%f MSPS %s)\n", sample_rate_val, wav_sample_per_sec * 0.000001f, wav_nb_channels == 1 ? "Real" : "IQ");
	}
	
	result = airspy_board_partid_serialno_read(device, &read_partid_serialno);
	if (result != AIRSPY_SUCCESS) {
			fprintf(stderr, "airspy_board_partid_serialno_read() failed: %s (%d)\n",
				airspy_error_name(result), result);
			airspy_close(device);
			airspy_exit();
			return EXIT_FAILURE;
	}
	fprintf(stderr, "Device Serial Number: 0x%08X%08X\n",
		read_partid_serialno.serial_no[2],
		read_partid_serialno.serial_no[3]);

	if( call_set_packing == true )
	{
		result = airspy_set_packing(device, packing_val);
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "airspy_set_packing() failed: %s (%d)\n", airspy_error_name(result), result);
			airspy_close(device);
			airspy_exit();
			return EXIT_FAILURE;
		}
	}

	result = airspy_set_rf_bias(device, biast_val);
	if( result != AIRSPY_SUCCESS ) {
		fprintf(stderr, "airspy_set_rf_bias() failed: %s (%d)\n", airspy_error_name(result), result);
		airspy_close(device);
		airspy_exit();
		return EXIT_FAILURE;
	}

	if (!strcmp(path,"-"))
		fd = stdout;
	else
		fd = fopen(path, "wb");
	if( fd == NULL ) {
		fprintf(stderr, "Failed to open file: %s\n", path);
		airspy_close(device);
		airspy_exit();
		return EXIT_FAILURE;
	}
	/* Change fd buffer to have bigger one to store data to file */
	result = setvbuf(fd , NULL , _IOFBF , FD_BUFFER_SIZE);
	if( result != 0 ) {
		fprintf(stderr, "setvbuf() failed: %d\n", result);
		airspy_close(device);
		airspy_exit();
		return EXIT_FAILURE;
	}
	
	/* Write Wav header */
	if( receive_wav ) 
	{
		fwrite(&wave_file_hdr, 1, sizeof(t_wav_file_hdr), fd);
	}
	
#ifdef _MSC_VER
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) sighandler, TRUE );
#else
	signal(SIGINT, &sigint_callback_handler);
	signal(SIGILL, &sigint_callback_handler);
	signal(SIGFPE, &sigint_callback_handler);
	signal(SIGSEGV, &sigint_callback_handler);
	signal(SIGTERM, &sigint_callback_handler);
	signal(SIGABRT, &sigint_callback_handler);
#endif

	if( (linearity_gain == false) && (sensitivity_gain == false) )
	{
		result = airspy_set_vga_gain(device, vga_gain);
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "airspy_set_vga_gain() failed: %s (%d)\n", airspy_error_name(result), result);
		}

		result = airspy_set_mixer_gain(device, mixer_gain);
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "airspy_set_mixer_gain() failed: %s (%d)\n", airspy_error_name(result), result);
		}

		result = airspy_set_lna_gain(device, lna_gain);
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "airspy_set_lna_gain() failed: %s (%d)\n", airspy_error_name(result), result);
		}
	} else
	{
		if( linearity_gain == true )
		{
			result =  airspy_set_linearity_gain(device, linearity_gain_val);
			if( result != AIRSPY_SUCCESS ) {
				fprintf(stderr, "airspy_set_linearity_gain() failed: %s (%d)\n", airspy_error_name(result), result);
			}
		}

		if( sensitivity_gain == true )
		{
			result =  airspy_set_sensitivity_gain(device, sensitivity_gain_val);
			if( result != AIRSPY_SUCCESS ) {
				fprintf(stderr, "airspy_set_sensitivity_gain() failed: %s (%d)\n", airspy_error_name(result), result);
			}
		}
	}

	result = airspy_start_rx(device, rx_callback, NULL);
	if( result != AIRSPY_SUCCESS ) {
		fprintf(stderr, "airspy_start_rx() failed: %s (%d)\n", airspy_error_name(result), result);
		airspy_close(device);
		airspy_exit();
		return EXIT_FAILURE;
	}

	result = airspy_set_freq(device, freq_hz);
	if( result != AIRSPY_SUCCESS ) {
		fprintf(stderr, "airspy_set_freq() failed: %s (%d)\n", airspy_error_name(result), result);
		airspy_close(device);
		airspy_exit();
		return EXIT_FAILURE;
	}

	fprintf(stderr, "Stop with Ctrl-C\n");

	average_rate = (float) wav_sample_per_sec;

	sleep(1);

	while( (airspy_is_streaming(device) == AIRSPY_TRUE) &&
		(do_exit == false) )
	{
		float average_rate_now = average_rate * 1e-6f;
		sprintf(str, "%2.3f", average_rate_now);
		average_rate_now = 9.5f;
		fprintf(stderr, "Streaming at %5s MSPS\n", str);
		if ((limit_num_samples == true) && (bytes_to_xfer == 0))
			do_exit = true;
		else
			sleep(1);
	}
	
	result = airspy_is_streaming(device);	
	if (do_exit)
	{
		fprintf(stderr, "\nUser cancel, exiting...\n");
	} else {
		fprintf(stderr, "\nExiting...\n");
	}
	
	gettimeofday(&t_end, NULL);
	time_diff = TimevalDiff(&t_end, &t_start);
	fprintf(stderr, "Total time: %5.4f s\n", time_diff);
	if (rate_samples > 0)
	{
		fprintf(stderr, "Average speed %2.4f MSPS %s\n", (global_average_rate * 1e-6f / rate_samples), (wav_nb_channels == 2 ? "IQ" : "Real"));
	}
	
	if(device != NULL)
	{
		result = airspy_stop_rx(device);
		if( result != AIRSPY_SUCCESS ) {
			fprintf(stderr, "airspy_stop_rx() failed: %s (%d)\n", airspy_error_name(result), result);
		}

		result = airspy_close(device);
		if( result != AIRSPY_SUCCESS ) 
		{
			fprintf(stderr, "airspy_close() failed: %s (%d)\n", airspy_error_name(result), result);
		}
		
		airspy_exit();
	}
		
	if(fd != NULL)
	{
		if( receive_wav ) 
		{
			/* Get size of file */
			file_pos = ftell(fd);
			/* Wav Header */
			wave_file_hdr.hdr.size = file_pos - 8;
			/* Wav Format Chunk */
			wave_file_hdr.fmt_chunk.wFormatTag = wav_format_tag;
			wave_file_hdr.fmt_chunk.wChannels = wav_nb_channels;
			wave_file_hdr.fmt_chunk.dwSamplesPerSec = wav_sample_per_sec;
			wave_file_hdr.fmt_chunk.dwAvgBytesPerSec = wave_file_hdr.fmt_chunk.dwSamplesPerSec * wav_nb_byte_per_sample;
			wave_file_hdr.fmt_chunk.wBlockAlign = wav_nb_channels * (wav_nb_bits_per_sample / 8);
			wave_file_hdr.fmt_chunk.wBitsPerSample = wav_nb_bits_per_sample;
			/* Wav Data Chunk */
			wave_file_hdr.data_chunk.chunkSize = file_pos - sizeof(t_wav_file_hdr);
			/* Overwrite header with updated data */
			rewind(fd);
			fwrite(&wave_file_hdr, 1, sizeof(t_wav_file_hdr), fd);
		}	
		fclose(fd);
		fd = NULL;
	}
	fprintf(stderr, "done\n");
	return exit_code;
}
