/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef LOGIC_DSLOGICINTERNAL_H
#define LOGIC_DSLOGICINTERNAL_H

#include <cstdint>

#include <libusb.h>

#include <hw/logic/DSLogicDevice.h>

#include <LogicInternal.h>

#define USB_INTERFACE        0
#define USB_CONFIGURATION    1
#define NUM_TRIGGER_STAGES  16
#define NUM_TRIGGER_PROBES  16
#define NUM_SIMUL_TRANSFERS 64
#define MAX_EMPTY_POLL      16
#define MAX_TRIGGER_PROBES  32
#define S_TRIGGER_DATA_STAGE 3

// Definiciones de VID y PID del DSLogic Plus
#define DSLOGIC_PLUS_VID 0x2A0E
#define DSLOGIC_PLUS_PID 0x0030

#define DSL_REQUIRED_VERSION_MAJOR   2
#define DSL_REQUIRED_VERSION_MINOR   0
#define DSL_HDL_VERSION              0x0E

/* Protocol commands */
#define CMD_CTL_WR       0xb0
#define CMD_CTL_RD_PRE   0xb1
#define CMD_CTL_RD       0xb2

// read only
#define bmGPIF_DONE     (1 << 7)
#define bmFPGA_DONE     (1 << 6)
#define bmFPGA_INIT_B   (1 << 5)
// write only
#define bmCH_CH0        (1 << 7)
#define bmCH_COM        (1 << 6)
#define bmCH_CH1        (1 << 5)
// read/write
#define bmSYS_OVERFLOW  (1 << 4)
#define bmSYS_CLR       (1 << 3)
#define bmSYS_EN        (1 << 2)
#define bmLED_RED       (1 << 1)
#define bmLED_GREEN     (1 << 0)

#define bmWR_PROG_B        (1 << 2)
#define bmWR_INTRDY        (1 << 7)
#define bmWR_WORDWIDE      (1 << 0)

#define VTH_ADDR 0x78
#define SEC_DATA_ADDR 0x75
#define SEC_CTRL_ADDR 0x73
#define CTR1_ADDR 0x71
#define CTR0_ADDR 0x70
#define COMB_ADDR 0x68
#define EI2C_ADDR 0x60
#define ADCC_ADDR 0x48
#define HW_STATUS_ADDR 0x05
#define HDL_VERSION_ADDR 0x04

#define SECU_STEPS    8
#define SECU_START    0x0513
#define SECU_CHECK    0x0219
#define SECU_EEP_ADDR 0x3C00
#define SECU_TRY_CNT    8

#define bmSECU_READY    (1 << 3)
#define bmSECU_PASS   (1 << 4)

#define EI2C_AWR      0x82
#define EI2C_ARD      0x83

#define EI2C_CTR_OFF  0x2
#define EI2C_RXR_OFF  0x3
#define EI2C_DSL_OFF   0x4
#define EI2C_TXR_OFF  0x3
#define EI2C_CR_OFF   0x4
#define EI2C_SEL_OFF  0x7

#define bmEI2C_EN     (1 << 7)
#define bmEI2C_STA    (1 << 7)
#define bmEI2C_STO    (1 << 6)
#define bmEI2C_RD     (1 << 5)
#define bmEI2C_WR     (1 << 4)
#define bmEI2C_NACK   (1 << 3)
#define bmEI2C_RXNACK (1 << 7)
#define bmEI2C_TIP    (1 << 1)

#define bmNONE         0
#define bmEEWP         (1 << 0)
#define bmFORCE_RDY    (1 << 1)
#define bmFORCE_STOP   (1 << 2)
#define bmSCOPE_SET    (1 << 3)
#define bmSCOPE_CLR    (1 << 4)
#define bmBW20M_SET    (1 << 5)
#define bmBW20M_CLR    (1 << 6)

/*
 * packet content check
 */
#define TRIG_CHECKID 0x55555555
#define DSO_PKTID 0xa500

/* hardware Capabilities */
#define CAPS_MODE_LOGIC     (1 << 0)
#define CAPS_MODE_ANALOG    (1 << 1)
#define CAPS_MODE_DSO       (1 << 2)

#define CAPS_FEATURE_NONE 0
// voltage threshold
#define CAPS_FEATURE_VTH (1 << 0)
// with external buffer
#define CAPS_FEATURE_BUF (1 << 1)
// pre offset control
#define CAPS_FEATURE_PREOFF (1 << 2)
// small startup eemprom
#define CAPS_FEATURE_SEEP (1 << 3)
// zero calibration ability
#define CAPS_FEATURE_ZERO (1 << 4)
// use HMCAD1511 adc chip
#define CAPS_FEATURE_HMCAD1511 (1 << 5)
// usb 3.0
#define CAPS_FEATURE_USB30 (1 << 6)
// pogopin panel
#define CAPS_FEATURE_POGOPIN (1 << 7)
// use ADF4360-7 vco chip
#define CAPS_FEATURE_ADF4360 (1 << 8)
// 20M bandwidth limitation
#define CAPS_FEATURE_20M (1 << 9)
// use startup flash (fx3)
#define CAPS_FEATURE_FLASH (1 << 10)
// 32 channels
#define CAPS_FEATURE_LA_CH32 (1 << 11)
// auto tunning vgain
#define CAPS_FEATURE_AUTO_VGAIN (1 << 12)
// max 2.5v fpga threshold
#define CAPS_FEATURE_MAX25_VTH (1 << 13)
// security check
#define CAPS_FEATURE_SECURITY (1 << 14)

#define DSLOGIC_ATOMIC_BITS 6
#define DSLOGIC_ATOMIC_SAMPLES (1 << DSLOGIC_ATOMIC_BITS)
#define DSLOGIC_ATOMIC_SIZE (1 << (DSLOGIC_ATOMIC_BITS - 3))
#define DSLOGIC_ATOMIC_MASK (0xFFFF << DSLOGIC_ATOMIC_BITS)

#define SAMPLES_ALIGN 1023LL

#define DS_MIN_TRIG_PERCENT 10
#define DS_MAX_TRIG_PERCENT 90

#define DS_CONF_DSO_HDIVS 10
#define DS_CONF_DSO_VDIVS 10

/*
 * for basic configuration
 */
#define TRIG_EN_BIT 0
#define CLK_TYPE_BIT 1
#define CLK_EDGE_BIT 2
#define RLE_MODE_BIT 3
#define DSO_MODE_BIT 4
#define HALF_MODE_BIT 5
#define QUAR_MODE_BIT 6
#define ANALOG_MODE_BIT 7
#define FILTER_BIT 8
#define INSTANT_BIT 9
#define SLOW_ACQ_BIT 10
#define STRIG_MODE_BIT 11
#define STREAM_MODE_BIT 12
#define LPB_TEST_BIT 13
#define EXT_TEST_BIT 14
#define INT_TEST_BIT 15

/* little macros */
#define DSL_CH(n)  (1 << n)

#define DSL_HZ(n)  (n)
#define DSL_KHZ(n) ((n) * (unsigned long long)(1000ULL))
#define DSL_MHZ(n) ((n) * (unsigned long long)(1000000ULL))
#define DSL_GHZ(n) ((n) * (unsigned long long)(1000000000ULL))

#define DSL_HZ_TO_NS(n) ((unsigned long long)(1000000000ULL) / (n))

#define DSL_NS(n)   (n)
#define DSL_US(n)   ((n) * (unsigned long long)(1000ULL))
#define DSL_MS(n)   ((n) * (unsigned long long)(1000000ULL))
#define DSL_SEC(n)  ((n) * (unsigned long long)(1000000000ULL))
#define DSL_MIN(n)  ((n) * (unsigned long long)(60000000000ULL))
#define DSL_HOUR(n) ((n) * (unsigned long long)(3600000000000ULL))
#define DSL_DAY(n)  ((n) * (unsigned long long)(86400000000000ULL))

#define DSL_n(n)  (n)
#define DSL_Kn(n) ((n) * (unsigned long long)(1000ULL))
#define DSL_Mn(n) ((n) * (unsigned long long)(1000000ULL))
#define DSL_Gn(n) ((n) * (unsigned long long)(1000000000ULL))

#define DSL_B(n)  (n)
#define DSL_KB(n) ((n) * (unsigned long long)(1024ULL))
#define DSL_MB(n) ((n) * (unsigned long long)(1048576ULL))
#define DSL_GB(n) ((n) * (unsigned long long)(1073741824ULL))

#define DSL_mV(n) (n)
#define DSL_V(n)  ((n) * (unsigned long long)(1000ULL))
#define DSL_KV(n) ((n) * (unsigned long long)(1000000ULL))
#define DSL_MV(n) ((n) * (unsigned long long)(1000000000ULL))

namespace hw::logic {

enum LedControl
{
   LED_OFF = 0,
   LED_GREEN = 1,
   LED_GREEN_BLINK = 2,
   LED_RED = 3,
   LED_RED_BLINK = 4,
};

#pragma pack(push, 1) // byte align
struct version_info
{
   uint8_t major;
   uint8_t minor;
};

struct cmd_zero_info
{
   uint8_t zero_addr;
   uint8_t voff0;
   uint8_t voff1;
   uint8_t voff2;
   uint8_t voff3;
   uint8_t voff4;
   uint8_t voff5;
   uint8_t voff6;
   uint8_t voff7;
   uint8_t voff8;
   uint8_t voff9;
   uint8_t voff10;
   uint8_t voff11;
   uint8_t voff12;
   uint8_t voff13;
   uint8_t voff14;
   uint8_t voff15;
   uint8_t diff0;
   uint8_t diff1;
   uint8_t trans0;
   uint8_t trans1;
   uint8_t comb_comp;
   uint8_t fgain0_code;
   uint8_t fgain1_code;
   uint8_t fgain2_code;
   uint8_t fgain3_code;
   uint8_t comb_fgain0_code;
   uint8_t comb_fgain1_code;
   uint8_t comb_fgain2_code;
   uint8_t comb_fgain3_code;
};

struct cmd_vga_info
{
   uint8_t vga_addr;
   uint16_t vga0;
   uint16_t vga1;
   uint16_t vga2;
   uint16_t vga3;
   uint16_t vga4;
   uint16_t vga5;
   uint16_t vga6;
   uint16_t vga7;
};

struct dsl_trigger_pos
{
   uint32_t check_id;
   uint32_t real_pos;
   uint32_t ram_saddr;
   uint32_t remain_cnt_l;
   uint32_t remain_cnt_h;
   uint32_t status;
};

struct usb_header
{
   uint8_t dest;
   uint16_t offset;
   uint8_t size;
};

struct usb_wr_cmd
{
   usb_header header;
   uint8_t data[60];
};

struct usb_rd_cmd
{
   usb_header header;
   void *data;
};
#pragma pack(pop)

enum dsl_command
{
   DSL_CTL_FW_VERSION = 0,
   DSL_CTL_REVID_VERSION = 1,
   DSL_CTL_HW_STATUS = 2,
   DSL_CTL_PROG_B = 3,
   DSL_CTL_SYS = 4,
   DSL_CTL_LED = 5,
   DSL_CTL_INTRDY = 6,
   DSL_CTL_WORDWIDE = 7,

   DSL_CTL_START = 8,
   DSL_CTL_STOP = 9,
   DSL_CTL_BULK_WR = 10,
   DSL_CTL_REG = 11,
   DSL_CTL_NVM = 12,

   DSL_CTL_I2C_DSO = 13,
   DSL_CTL_I2C_REG = 14,
   DSL_CTL_I2C_STATUS = 15,

   DSL_CTL_DSO_EN0 = 16,
   DSL_CTL_DSO_DC0 = 17,
   DSL_CTL_DSO_ATT0 = 18,
   DSL_CTL_DSO_EN1 = 19,
   DSL_CTL_DSO_DC1 = 20,
   DSL_CTL_DSO_ATT1 = 21,

   DSL_CTL_AWG_WR = 22,
   DSL_CTL_I2C_PROBE = 23,
   DSL_CTL_I2C_EXT = 24,
};

struct dsl_caps
{
   long mode_caps;
   long feature_caps;
   long channels;
   int total_ch_num;
   long long hw_depth;
   int dso_depth;
   int intest_channel;
   const long *vdivs;
   const unsigned long long *samplerates;
   int vga_id;
   int default_channelid;
   long default_samplerate;
   long default_samplelimit;
   int default_pwmtrans;
   int default_pwmmargin;
   int ref_min;
   int ref_max;
   int default_comb_comp;
   long half_samplerate;
   long quarter_samplerate;
};

struct dsl_profile
{
   int vid;
   int pid;
   libusb_speed usb_speed;

   const char *vendor;
   const char *model; //product name
   const char *model_version;

   const char *firmware;

   const char *fpga_bit33;
   const char *fpga_bit50;

   dsl_caps dev_caps;
};

struct dsl_adc_config
{
   uint8_t dest;
   uint8_t cnt;
   uint8_t delay;
   uint8_t byte[4];
};

/*
 * hardware setting for each capture
 */
struct dsl_setting
{
   uint32_t sync;

   uint16_t mode_header; // 0
   uint16_t mode;
   uint16_t divider_header; // 1-2
   uint16_t div_l;
   uint16_t div_h;
   uint16_t count_header; // 3-4
   uint16_t cnt_l;
   uint16_t cnt_h;
   uint16_t trig_pos_header; // 5-6
   uint16_t tpos_l;
   uint16_t tpos_h;
   uint16_t trig_glb_header; // 7
   uint16_t trig_glb;
   uint16_t dso_count_header; // 8-9
   uint16_t dso_cnt_l;
   uint16_t dso_cnt_h;
   uint16_t ch_en_header; // 10-11
   uint16_t ch_en_l;
   uint16_t ch_en_h;
   uint16_t fgain_header; // 12
   uint16_t fgain;

   uint16_t trig_header; // 64
   uint16_t trig_mask0[NUM_TRIGGER_STAGES];
   uint16_t trig_mask1[NUM_TRIGGER_STAGES];
   uint16_t trig_value0[NUM_TRIGGER_STAGES];
   uint16_t trig_value1[NUM_TRIGGER_STAGES];
   uint16_t trig_edge0[NUM_TRIGGER_STAGES];
   uint16_t trig_edge1[NUM_TRIGGER_STAGES];
   uint16_t trig_logic0[NUM_TRIGGER_STAGES];
   uint16_t trig_logic1[NUM_TRIGGER_STAGES];
   uint32_t trig_count[NUM_TRIGGER_STAGES];

   uint32_t end_sync;
};

struct dsl_setting_ext32
{
   uint32_t sync;

   uint16_t trig_header; // 96
   uint16_t trig_mask0[NUM_TRIGGER_STAGES];
   uint16_t trig_mask1[NUM_TRIGGER_STAGES];
   uint16_t trig_value0[NUM_TRIGGER_STAGES];
   uint16_t trig_value1[NUM_TRIGGER_STAGES];
   uint16_t trig_edge0[NUM_TRIGGER_STAGES];
   uint16_t trig_edge1[NUM_TRIGGER_STAGES];

   uint16_t align_bytes;
   uint32_t end_sync;
};

struct dsl_trigger
{
   int trigger_enabled;
   int trigger_mode;
   int trigger_position;
   int trigger_stages;
   unsigned char trigger_logic[NUM_TRIGGER_STAGES + 1];
   unsigned char trigger0_inv[NUM_TRIGGER_STAGES + 1];
   unsigned char trigger1_inv[NUM_TRIGGER_STAGES + 1];
   char trigger0[NUM_TRIGGER_STAGES + 1][NUM_TRIGGER_PROBES];
   char trigger1[NUM_TRIGGER_STAGES + 1][NUM_TRIGGER_PROBES];
   int trigger0_count[NUM_TRIGGER_STAGES + 1];
   int trigger1_count[NUM_TRIGGER_STAGES + 1];
};

struct dsl_vga
{
   int id;
   long key;
   long vgain;
   int preoff;
   int preoff_comp;
};

struct dsl_channel_mode
{
   int id;
   int mode;
   int type;
   bool stream;
   int num;
   int vld_num;
   int unit_bits;
   long min_samplerate;
   long max_samplerate;
   long hw_min_samplerate;
   long hw_max_samplerate;
   int pre_div;
   const char *descr;
};

struct dsl_channel
{
   int index;
   int type;
   bool enabled;
   const char *name;
   const char *trigger;
   int bits;
   int vdiv;
   int vfactor;
   int offset;
   int zero_offset;
   int hw_offset;
   int vpos_trans;
   int coupling;
   int trig_value;
   int comb_diff_top;
   int comb_diff_bom;
   int comb_comp;
   int digi_fgain;

   double cali_fgain0;
   double cali_fgain1;
   double cali_fgain2;
   double cali_fgain3;
   double cali_comb_fgain0;
   double cali_comb_fgain1;
   double cali_comb_fgain2;
   double cali_comb_fgain3;

   bool map_default;
   const char *map_unit;
   double map_min;
   double map_max;

   std::list<dsl_vga> vga_list;
};

static const char *probe_names[] = {
   "0", "1", "2", "3", "4", "5", "6", "7",
   "8", "9", "10", "11", "12", "13", "14", "15",
   "16", "17", "18", "19", "20", "21", "22", "23",
   "24", "25", "26", "27", "28", "29", "30", "31",
   nullptr,
};

static const char *probe_units[] = {
   "V",
   "A",
   "℃",
   "℉",
   "g",
   "m",
   "m/s",
};

static const unsigned long long samplerates100[] = {
   DSL_HZ(10),
   DSL_HZ(20),
   DSL_HZ(50),
   DSL_HZ(100),
   DSL_HZ(200),
   DSL_HZ(500),
   DSL_KHZ(1),
   DSL_KHZ(2),
   DSL_KHZ(5),
   DSL_KHZ(10),
   DSL_KHZ(20),
   DSL_KHZ(40),
   DSL_KHZ(50),
   DSL_KHZ(100),
   DSL_KHZ(200),
   DSL_KHZ(400),
   DSL_KHZ(500),
   DSL_MHZ(1),
   DSL_MHZ(2),
   DSL_MHZ(4),
   DSL_MHZ(5),
   DSL_MHZ(10),
   DSL_MHZ(20),
   DSL_MHZ(25),
   DSL_MHZ(50),
   DSL_MHZ(100),
   0,
};

static const unsigned long long samplerates400[] = {
   DSL_HZ(10),
   DSL_HZ(20),
   DSL_HZ(50),
   DSL_HZ(100),
   DSL_HZ(200),
   DSL_HZ(500),
   DSL_KHZ(1),
   DSL_KHZ(2),
   DSL_KHZ(5),
   DSL_KHZ(10),
   DSL_KHZ(20),
   DSL_KHZ(40),
   DSL_KHZ(50),
   DSL_KHZ(100),
   DSL_KHZ(200),
   DSL_KHZ(400),
   DSL_KHZ(500),
   DSL_MHZ(1),
   DSL_MHZ(2),
   DSL_MHZ(4),
   DSL_MHZ(5),
   DSL_MHZ(10),
   DSL_MHZ(20),
   DSL_MHZ(25),
   DSL_MHZ(50),
   DSL_MHZ(100),
   DSL_MHZ(200),
   DSL_MHZ(400),
   0,
};

static const unsigned long long samplerates1000[] = {
   DSL_HZ(10),
   DSL_HZ(20),
   DSL_HZ(50),
   DSL_HZ(100),
   DSL_HZ(200),
   DSL_HZ(500),
   DSL_KHZ(1),
   DSL_KHZ(2),
   DSL_KHZ(5),
   DSL_KHZ(10),
   DSL_KHZ(20),
   DSL_KHZ(40),
   DSL_KHZ(50),
   DSL_KHZ(100),
   DSL_KHZ(200),
   DSL_KHZ(400),
   DSL_KHZ(500),
   DSL_MHZ(1),
   DSL_MHZ(2),
   DSL_MHZ(4),
   DSL_MHZ(5),
   DSL_MHZ(10),
   DSL_MHZ(20),
   DSL_MHZ(25),
   DSL_MHZ(50),
   DSL_MHZ(100),
   DSL_MHZ(125),
   DSL_MHZ(250),
   DSL_MHZ(500),
   DSL_GHZ(1),
   0,
};

static const dsl_adc_config adc_init_fix[] = {
   {ADCC_ADDR + 1, 3, 0, {0x03, 0x01, 0x00, 0x00}}, // reset & power down
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x02, 0x01, 0x31}}, // 2x channel 1/2 clock
   {ADCC_ADDR + 1, 1, 0, {0x01, 0x00, 0x00, 0x00}}, // power up
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x02, 0x02, 0x3A}}, // adc0: ch0 adc1: ch0
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x10, 0x10, 0x3B}}, // adc2: ch3 adc3: ch3
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x00, 0x00, 0x42}}, // phase_ddr: 270 deg
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x34, 0x00, 0x50}}, // adc core current: -40% (lower performance) / VCM: +-700uA
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x22, 0x02, 0x11}}, // lvds drive strength: 1.5mA(RSDS)
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x10, 0x00, 0x25}}, // fix pattern test
   {ADCC_ADDR + 0, 4, 0, {0x00, 0x00, 0x55, 0x26}}, // test pattern
   {0, 0, 0, {0, 0, 0, 0}}
};

static const dsl_adc_config adc_clk_init_1g[] = {
   {ADCC_ADDR + 2, 1, 0, {0x01, 0x00, 0x00, 0x00}}, // power up
   {ADCC_ADDR + 0, 4, 0, {0x01, 0x61, 0x00, 0x30}}, //
   {ADCC_ADDR + 0, 4, 0, {0x01, 0x40, 0xF1, 0x46}}, //
   {ADCC_ADDR + 0, 4, 10, {0x01, 0x62, 0x3D, 0x00}}, //
   {0, 0, 0, {0, 0, 0, 0}}
};

static const dsl_adc_config adc_clk_init_500m[] = {
   {ADCC_ADDR + 2, 1, 0, {0x01, 0x00, 0x00, 0x00}}, // power up
   {ADCC_ADDR + 0, 4, 0, {0x01, 0x61, 0x00, 0x30}}, //
   {ADCC_ADDR + 0, 4, 0, {0x01, 0x40, 0xF1, 0x46}}, //
   {ADCC_ADDR + 0, 4, 10, {0x01, 0x62, 0x3D, 0x40}}, //
   {0, 0, 0, {0, 0, 0, 0}}
};

static const dsl_adc_config adc_power_down[] = {
   //{ADCC_ADDR+2, 1,   0,   {0x00, 0x00, 0x00, 0x00}}, // ADC_CLK power down
   {ADCC_ADDR + 1, 1, 0, {0x00, 0x00, 0x00, 0x00}}, // ADC power down
   {0, 0, 0, {0, 0, 0, 0}}
};

static const dsl_adc_config adc_power_up[] = {
   {ADCC_ADDR + 1, 1, 0, {0x01, 0x00, 0x00, 0x00}}, // ADC power up
   {0, 0, 0, {0, 0, 0, 0}}
};

static const dsl_channel_mode channel_modes[] = {
   // LA Stream
   {DSLogicDevice::DSL_STREAM20x16, LOGIC, CHANNEL_LOGIC, true, 16, 16, 1, DSL_KHZ(50), DSL_MHZ(20), DSL_KHZ(10), DSL_MHZ(100), 1, "Use 16 Channels (Max 20MHz)"},
   {DSLogicDevice::DSL_STREAM25x12, LOGIC, CHANNEL_LOGIC, true, 16, 12, 1, DSL_KHZ(50), DSL_MHZ(25), DSL_KHZ(10), DSL_MHZ(100), 1, "Use 12 Channels (Max 25MHz)"},
   {DSLogicDevice::DSL_STREAM50x6, LOGIC, CHANNEL_LOGIC, true, 16, 6, 1, DSL_KHZ(50), DSL_MHZ(50), DSL_KHZ(10), DSL_MHZ(100), 1, "Use 6 Channels (Max 50MHz)"},
   {DSLogicDevice::DSL_STREAM100x3, LOGIC, CHANNEL_LOGIC, true, 16, 3, 1, DSL_KHZ(50), DSL_MHZ(100), DSL_KHZ(10), DSL_MHZ(100), 1, "Use 3 Channels (Max 100MHz)"},
   {DSLogicDevice::DSL_STREAM20x16_3DN2, LOGIC, CHANNEL_LOGIC, true, 16, 16, 1, DSL_KHZ(100), DSL_MHZ(20), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 16 Channels (Max 20MHz)"},
   {DSLogicDevice::DSL_STREAM25x12_3DN2, LOGIC, CHANNEL_LOGIC, true, 16, 12, 1, DSL_KHZ(100), DSL_MHZ(25), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 12 Channels (Max 25MHz)"},
   {DSLogicDevice::DSL_STREAM50x6_3DN2, LOGIC, CHANNEL_LOGIC, true, 16, 6, 1, DSL_KHZ(100), DSL_MHZ(50), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 6 Channels (Max 50MHz)"},
   {DSLogicDevice::DSL_STREAM100x3_3DN2, LOGIC, CHANNEL_LOGIC, true, 16, 3, 1, DSL_KHZ(100), DSL_MHZ(100), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 3 Channels (Max 100MHz)"},
   {DSLogicDevice::DSL_STREAM10x32_32_3DN2, LOGIC, CHANNEL_LOGIC, true, 32, 32, 1, DSL_KHZ(100), DSL_MHZ(10), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 32 Channels (Max 10MHz)"},
   {DSLogicDevice::DSL_STREAM20x16_32_3DN2, LOGIC, CHANNEL_LOGIC, true, 32, 16, 1, DSL_KHZ(100), DSL_MHZ(20), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 16 Channels (Max 20MHz)"},
   {DSLogicDevice::DSL_STREAM25x12_32_3DN2, LOGIC, CHANNEL_LOGIC, true, 32, 12, 1, DSL_KHZ(100), DSL_MHZ(25), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 12 Channels (Max 25MHz)"},
   {DSLogicDevice::DSL_STREAM50x6_32_3DN2, LOGIC, CHANNEL_LOGIC, true, 32, 6, 1, DSL_KHZ(100), DSL_MHZ(50), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 6 Channels (Max 50MHz)"},
   {DSLogicDevice::DSL_STREAM100x3_32_3DN2, LOGIC, CHANNEL_LOGIC, true, 32, 3, 1, DSL_KHZ(100), DSL_MHZ(100), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 3 Channels (Max 100MHz)"},
   {DSLogicDevice::DSL_STREAM50x32, LOGIC, CHANNEL_LOGIC, true, 32, 32, 1, DSL_MHZ(1), DSL_MHZ(50), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 32 Channels (Max 50MHz)"},
   {DSLogicDevice::DSL_STREAM100x30, LOGIC, CHANNEL_LOGIC, true, 32, 30, 1, DSL_MHZ(1), DSL_MHZ(100), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 30 Channels (Max 100MHz)"},
   {DSLogicDevice::DSL_STREAM250x12, LOGIC, CHANNEL_LOGIC, true, 32, 12, 1, DSL_MHZ(1), DSL_MHZ(250), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 12 Channels (Max 250MHz)"},
   {DSLogicDevice::DSL_STREAM125x16_16, LOGIC, CHANNEL_LOGIC, true, 16, 16, 1, DSL_MHZ(1), DSL_MHZ(125), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 16 Channels (Max 125MHz)"},
   {DSLogicDevice::DSL_STREAM250x12_16, LOGIC, CHANNEL_LOGIC, true, 16, 12, 1, DSL_MHZ(1), DSL_MHZ(250), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 12 Channels (Max 250MHz)"},
   {DSLogicDevice::DSL_STREAM500x6, LOGIC, CHANNEL_LOGIC, true, 16, 6, 1, DSL_MHZ(1), DSL_MHZ(500), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 6 Channels (Max 500MHz)"},
   {DSLogicDevice::DSL_STREAM1000x3, LOGIC, CHANNEL_LOGIC, true, 8, 3, 1, DSL_MHZ(1), DSL_GHZ(1), DSL_KHZ(10), DSL_MHZ(500), 5, "Use 3 Channels (Max 1GHz)"},

   // LA Buffer
   {DSLogicDevice::DSL_BUFFER100x16, LOGIC, CHANNEL_LOGIC, false, 16, 16, 1, DSL_KHZ(50), DSL_MHZ(100), DSL_KHZ(10), DSL_MHZ(100), 1, "Use Channels 0~15 (Max 100MHz)"},
   {DSLogicDevice::DSL_BUFFER200x8, LOGIC, CHANNEL_LOGIC, false, 8, 8, 1, DSL_KHZ(50), DSL_MHZ(200), DSL_KHZ(10), DSL_MHZ(100), 1, "Use Channels 0~7 (Max 200MHz)"},
   {DSLogicDevice::DSL_BUFFER400x4, LOGIC, CHANNEL_LOGIC, false, 4, 4, 1, DSL_KHZ(50), DSL_MHZ(400), DSL_KHZ(10), DSL_MHZ(100), 1, "Use Channels 0~3 (Max 400MHz)"},

   {DSLogicDevice::DSL_BUFFER250x32, LOGIC, CHANNEL_LOGIC, false, 32, 32, 1, DSL_MHZ(1), DSL_MHZ(250), DSL_KHZ(10), DSL_MHZ(500), 5, "Use Channels 0~31 (Max 250MHz)"},
   {DSLogicDevice::DSL_BUFFER500x16, LOGIC, CHANNEL_LOGIC, false, 16, 16, 1, DSL_MHZ(1), DSL_MHZ(500), DSL_KHZ(10), DSL_MHZ(500), 5, "Use Channels 0~15 (Max 500MHz)"},
   {DSLogicDevice::DSL_BUFFER1000x8, LOGIC, CHANNEL_LOGIC, false, 8, 8, 1, DSL_MHZ(1), DSL_GHZ(1), DSL_KHZ(10), DSL_MHZ(500), 5, "Use Channels 0~7 (Max 1GHz)"},
};

static const dsl_vga vga_defaults[] = {
   {1, 10, 0x162400, (32 << 10) + 558, (32 << 10) + 558},
   {1, 20, 0x14C000, (32 << 10) + 558, (32 << 10) + 558},
   {1, 50, 0x12E800, (32 << 10) + 558, (32 << 10) + 558},
   {1, 100, 0x118000, (32 << 10) + 558, (32 << 10) + 558},
   {1, 200, 0x102400, (32 << 10) + 558, (32 << 10) + 558},
   {1, 500, 0x2E800, (32 << 10) + 558, (32 << 10) + 558},
   {1, 1000, 0x18000, (32 << 10) + 558, (32 << 10) + 558},
   {1, 2000, 0x02400, (32 << 10) + 558, (32 << 10) + 558},

   {2, 10, 0x1DA800, 45, 1024 - 920 - 45},
   {2, 20, 0x1A7200, 45, 1024 - 920 - 45},
   {2, 50, 0x164200, 45, 1024 - 920 - 45},
   {2, 100, 0x131800, 45, 1024 - 920 - 45},
   {2, 200, 0xBD000, 45, 1024 - 920 - 45},
   {2, 500, 0x7AD00, 45, 1024 - 920 - 45},
   {2, 1000, 0x48800, 45, 1024 - 920 - 45},
   {2, 2000, 0x12000, 45, 1024 - 920 - 45},

   {3, 10, 0x1C5C00, 45, 1024 - 920 - 45},
   {3, 20, 0x19EB00, 45, 1024 - 920 - 45},
   {3, 50, 0x16AE00, 45, 1024 - 920 - 45},
   {3, 100, 0x143D00, 45, 1024 - 920 - 45},
   {3, 200, 0xB1000, 45, 1024 - 920 - 45},
   {3, 500, 0x7F000, 45, 1024 - 920 - 45},
   {3, 1000, 0x57200, 45, 1024 - 920 - 45},
   {3, 2000, 0x2DD00, 45, 1024 - 920 - 45},

   {4, 10, 0x1C6C00, 60, 1024 - 900 - 60},
   {4, 20, 0x19E000, 60, 1024 - 900 - 60},
   {4, 50, 0x16A800, 60, 1024 - 900 - 60},
   {4, 100, 0x142800, 60, 1024 - 900 - 60},
   {4, 200, 0xC7F00, 60, 1024 - 900 - 60},
   {4, 500, 0x94000, 60, 1024 - 900 - 60},
   {4, 1000, 0x6CF00, 60, 1024 - 900 - 60},
   {4, 2000, 0x44F00, 60, 1024 - 900 - 60},

   {5, 10, 0x1C3400, 60, 1024 - 900 - 60},
   {5, 20, 0x19BD00, 60, 1024 - 900 - 60},
   {5, 50, 0x167400, 60, 1024 - 900 - 60},
   {5, 100, 0x13F300, 60, 1024 - 900 - 60},
   {5, 200, 0xC4F00, 60, 1024 - 900 - 60},
   {5, 500, 0x91B00, 60, 1024 - 900 - 60},
   {5, 1000, 0x69D00, 60, 1024 - 900 - 60},
   {5, 2000, 0x41D00, 60, 1024 - 900 - 60},

   {0, 0, 0, 0, 0}
};

// supported devices
static const dsl_profile dsl_profiles[] = {
   {
      .vid = 0x2A0E,
      .pid = 0x0020,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic PLus",
      .model_version = nullptr,
      .firmware = "DSLogicPlus.fw",
      .fpga_bit33 = "DSLogicPlus.bin",
      .fpga_bit50 = "DSLogicPlus.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER100x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER200x8) |
         DSL_CH(DSLogicDevice::DSL_BUFFER400x4), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_MB(256), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER100x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates400, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(200), // half_samplerate
         .quarter_samplerate = DSL_MHZ(400), // quarter_samplerate
      }
   },
   {
      .vid = 0x2A0E,
      .pid = 0x0021,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic Basic",
      .model_version = nullptr,
      .firmware = "DSLogicBasic.fw",
      .fpga_bit33 = "DSLogicBasic.bin",
      .fpga_bit50 = "DSLogicBasic.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER100x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER200x8) |
         DSL_CH(DSLogicDevice::DSL_BUFFER400x4), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_KB(256), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_STREAM20x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates400, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(200), // half_samplerate
         .quarter_samplerate = DSL_MHZ(400), // quarter_samplerate
      }
   },
   {
      .vid = 0x2A0E,
      .pid = 0x0029,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U2Basic",
      .model_version = nullptr,
      .firmware = "DSLogicU2Basic.fw",
      .fpga_bit33 = "DSLogicU2Basic.bin",
      .fpga_bit50 = "DSLogicU2Basic.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER100x16), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_MB(64), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER100x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates100, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(200), // half_samplerate
         .quarter_samplerate = DSL_MHZ(400), // quarter_samplerate
      }
   },
   {
      .vid = 0x2A0E,
      .pid = 0x002A,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U3Pro16",
      .model_version = nullptr,
      .firmware = "DSLogicU3Pro16.fw",
      .fpga_bit33 = "DSLogicU3Pro16.bin",
      .fpga_bit50 = "DSLogicU3Pro16.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_USB30 | CAPS_FEATURE_ADF4360, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3_3DN2) |
         DSL_CH(DSLogicDevice::DSL_BUFFER500x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER1000x8), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_GB(2), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER500x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates1000, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6_3DN2, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(500), // half_samplerate
         .quarter_samplerate = DSL_GHZ(1), // quarter_samplerate
      },
   },
   {
      .vid = 0x2A0E,
      .pid = 0x002A,
      .usb_speed = LIBUSB_SPEED_SUPER,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U3Pro16",
      .model_version = nullptr,
      .firmware = "DSLogicU3Pro16.fw",
      .fpga_bit33 = "DSLogicU3Pro16.bin",
      .fpga_bit50 = "DSLogicU3Pro16.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_USB30 | CAPS_FEATURE_ADF4360, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM125x16_16) |
         DSL_CH(DSLogicDevice::DSL_STREAM250x12_16) |
         DSL_CH(DSLogicDevice::DSL_STREAM500x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM1000x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER500x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER1000x8), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_GB(2), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER500x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates1000, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM500x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(500), // half_samplerate
         .quarter_samplerate = DSL_GHZ(1), // quarter_samplerate
      },
   },
   {
      .vid = 0x2A0E,
      .pid = 0x002C,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U3Pro32",
      .model_version = nullptr,
      .firmware = "DSLogicU3Pro32.fw",
      .fpga_bit33 = "DSLogicU3Pro32.bin",
      .fpga_bit50 = "DSLogicU3Pro32.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_USB30 | CAPS_FEATURE_ADF4360 | CAPS_FEATURE_LA_CH32, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM10x32_32_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM20x16_32_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12_32_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6_32_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3_32_3DN2) |
         DSL_CH(DSLogicDevice::DSL_BUFFER250x32) |
         DSL_CH(DSLogicDevice::DSL_BUFFER500x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER1000x8), // channels
         .total_ch_num = 32, // total_ch_num
         .hw_depth = DSL_GB(2), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER250x32, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates1000, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6_32_3DN2, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(500), // half_samplerate
         .quarter_samplerate = DSL_GHZ(1), // quarter_samplerate
      },
   },
   {
      .vid = 0x2A0E,
      .pid = 0x002C,
      .usb_speed = LIBUSB_SPEED_SUPER,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U3Pro32",
      .model_version = nullptr,
      .firmware = "DSLogicU3Pro32.fw",
      .fpga_bit33 = "DSLogicU3Pro32.bin",
      .fpga_bit50 = "DSLogicU3Pro32.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_USB30 | CAPS_FEATURE_ADF4360 | CAPS_FEATURE_LA_CH32, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM50x32) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x30) |
         DSL_CH(DSLogicDevice::DSL_STREAM250x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM500x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM1000x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER250x32) |
         DSL_CH(DSLogicDevice::DSL_BUFFER500x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER1000x8), // channels
         .total_ch_num = 32, // total_ch_num
         .hw_depth = DSL_GB(2), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER250x32, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates1000, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM500x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(500), // half_samplerate
         .quarter_samplerate = DSL_GHZ(1), // quarter_samplerate
      },
   },
   {
      .vid = 0x2A0E,
      .pid = 0x002D,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U2Pro16",
      .model_version = nullptr,
      .firmware = "DSLogicU2Pro16.fw",
      .fpga_bit33 = "DSLogicU2Pro16.bin",
      .fpga_bit50 = "DSLogicU2Pro16.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_ADF4360 | CAPS_FEATURE_SECURITY, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6_3DN2) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3_3DN2) |
         DSL_CH(DSLogicDevice::DSL_BUFFER500x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER1000x8), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_GB(4), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER500x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates1000, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6_3DN2, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(500), // half_samplerate
         .quarter_samplerate = DSL_GHZ(1), // quarter_samplerate
      },
   },
   {
      .vid = 0x2A0E,
      .pid = 0x0030,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic PLus",
      .model_version = nullptr,
      .firmware = "DSLogicPlus.fw",
      .fpga_bit33 = "DSLogicPlus-pgl12.bin",
      .fpga_bit50 = "DSLogicPlus-pgl12.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_MAX25_VTH | CAPS_FEATURE_SECURITY, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER100x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER200x8) |
         DSL_CH(DSLogicDevice::DSL_BUFFER400x4), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_MB(256), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER100x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates400, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(200), // half_samplerate
         .quarter_samplerate = DSL_MHZ(400), // quarter_samplerate
      },
   },
   {
      .vid = 0x2A0E,
      .pid = 0x0031,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U2Basic",
      .model_version = nullptr,
      .firmware = "DSLogicU2Basic.fw",
      .fpga_bit33 = "DSLogicU2Basic-pgl12.bin",
      .fpga_bit50 = "DSLogicU2Basic-pgl12.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_MAX25_VTH | CAPS_FEATURE_SECURITY, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER100x16), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_MB(64), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER100x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates100, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(200), // half_samplerate
         .quarter_samplerate = DSL_MHZ(400), // quarter_samplerate
      }
   },
   {
      .vid = 0x2A0E,
      .pid = 0x0034,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic PLus",
      .model_version = nullptr,
      .firmware = "DSLogicPlus-pgl12-2.fw",
      .fpga_bit33 = "DSLogicPlus-pgl12-2.bin",
      .fpga_bit50 = "DSLogicPlus-pgl12-2.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_MAX25_VTH | CAPS_FEATURE_SECURITY, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER100x16) |
         DSL_CH(DSLogicDevice::DSL_BUFFER200x8) |
         DSL_CH(DSLogicDevice::DSL_BUFFER400x4), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_MB(256), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER100x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates400, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(200), // half_samplerate
         .quarter_samplerate = DSL_MHZ(400), // quarter_samplerate
      }
   },
   {
      .vid = 0x2A0E,
      .pid = 0x0035,
      .usb_speed = LIBUSB_SPEED_HIGH,
      .vendor = "DreamSourceLab",
      .model = "DSLogic U2Basic",
      .model_version = nullptr,
      .firmware = "DSLogicU2Basic-pgl12-2.fw",
      .fpga_bit33 = "DSLogicU2Basic-pgl12-2.bin",
      .fpga_bit50 = "DSLogicU2Basic-pgl12-2.bin",
      .dev_caps {
         .mode_caps = CAPS_MODE_LOGIC, // mode_caps
         .feature_caps = CAPS_FEATURE_VTH | CAPS_FEATURE_BUF | CAPS_FEATURE_MAX25_VTH | CAPS_FEATURE_SECURITY, // feature_caps
         .channels = DSL_CH(DSLogicDevice::DSL_STREAM20x16) |
         DSL_CH(DSLogicDevice::DSL_STREAM25x12) |
         DSL_CH(DSLogicDevice::DSL_STREAM50x6) |
         DSL_CH(DSLogicDevice::DSL_STREAM100x3) |
         DSL_CH(DSLogicDevice::DSL_BUFFER100x16), // channels
         .total_ch_num = 16, // total_ch_num
         .hw_depth = DSL_MB(64), // hw_depth
         .dso_depth = 0, // dso_depth
         .intest_channel = DSLogicDevice::DSL_BUFFER100x16, // intest_channel
         .vdivs = nullptr, // vdivs
         .samplerates = samplerates100, // samplerates
         .vga_id = 0x00, // vga_id
         .default_channelid = DSLogicDevice::DSL_STREAM50x6, // default_channelid
         .default_samplerate = DSL_MHZ(1), // default_samplerate
         .default_samplelimit = DSL_Mn(1), // default_samplelimit
         .default_pwmtrans = 0x0000, // default_pwmtrans
         .default_pwmmargin = 0x0000, // default_pwmmargin
         .ref_min = 0x00000000, // ref_min
         .ref_max = 0x00000000, // ref_max
         .default_comb_comp = 0x00, // default_comb_comp
         .half_samplerate = DSL_MHZ(200), // half_samplerate
         .quarter_samplerate = DSL_MHZ(400), // quarter_samplerate
      }
   },
   {}
};

}

#endif
