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

#include <unistd.h>

#include <cmath>
#include <list>
#include <fstream>
#include <algorithm>

#include <rt/Logger.h>

#include <hw/usb/Usb.h>

#include <hw/SignalType.h>

#include <hw/logic/DSLogicDevice.h>

#include "DSLogicInternal.h"

#define DEVICE_TYPE_PREFIX "logic.dreamsourcelab"
#define CHANNEL_BUFFER_SIZE (1 << 16) // must be multiple of 64
#define CHANNEL_BUFFER_SAMPLES 16384 // number of samples per buffer

namespace hw::logic {

struct DSLogicDevice::Impl
{
   rt::Logger *log = rt::Logger::getLogger("hw.DSLogicDevice");

   // device parameters
   std::string deviceName;
   std::string deviceVendor;
   std::string deviceModel;
   std::string deviceVersion;
   std::string deviceSerial;
   std::string firmwarePath;

   // Underlying USB device.
   Usb usb;

   // Device profile.
   const dsl_profile *profile = nullptr;

   // Device status.
   unsigned char hwStatus {};
   version_info fwVersion {};
   unsigned char fpgaVersion {};
   double vth {};
   int thLevel {};

   // Trigger parameters
   dsl_trigger trigger {};

   // Probe configuration
   std::list<dsl_channel> channels;

   // Device configuration
   unsigned int timebase = 0;
   unsigned int samplerate = 0;
   unsigned int streamTime = 0;
   unsigned long long limitSamples = 0;
   unsigned long long captureSamples = 0;
   unsigned long long captureBytes = 0;
   unsigned long long currentSamples = 0;
   unsigned long long currentBytes = 0;
   unsigned long long droppedSamples = 0;
   unsigned long long droppedBytes = 0;

   // Operational settings
   int deviceStatus = STATUS_ERROR;
   int operationMode;
   int channelMode;
   int testMode;
   unsigned int totalChannels;
   unsigned int validChannels;

   // device flags
   bool clockType;
   bool clockEdge;
   bool rleCompress;
   bool rleSupport;
   bool stream;

   int filter;
   int sampleratesMinIndex;
   int sampleratesMaxIndex;

   // trigger options
   int triggerChannel;
   TriggerSlope triggerSlope;
   TriggerSource triggerSource;
   int triggerHRate;
   int triggerHPos;
   int triggerHoldoff;
   int triggerMargin;

   /*
    * Control led blink status
    */
   bool blinkStatus = false;
   std::chrono::time_point<std::chrono::steady_clock> lastBlink;

   /*
    * Transfer buffers
    */
   std::list<Usb::Transfer *> transfers;

   /*
    * Received buffers
    */
   SignalBuffer buffer;

   /*
    * Receive handler
    */
   StreamHandler streamHandler;

   /*
    * Control commands.
    */
   const usb_rd_cmd rd_cmd_hw_status {.header {.dest = DSL_CTL_HW_STATUS, .size = sizeof(hwStatus)}, .data = &hwStatus};
   const usb_rd_cmd rd_cmd_fw_version {.header {.dest = DSL_CTL_FW_VERSION, .size = sizeof(fwVersion)}, .data = &fwVersion};
   const usb_rd_cmd rd_cmd_fpga_version {.header {.dest = DSL_CTL_I2C_STATUS, .offset = HDL_VERSION_ADDR, .size = 1}, .data = &fpgaVersion};

   const usb_wr_cmd wr_cmd_prog_b_low {.header {.dest = DSL_CTL_PROG_B, .size = 1}, .data {static_cast<uint8_t>(~bmWR_PROG_B)}};
   const usb_wr_cmd wr_cmd_b_high {.header {.dest = DSL_CTL_PROG_B, .size = 1}, .data {static_cast<uint8_t>(bmWR_PROG_B)}};
   const usb_wr_cmd wr_cmd_led_off {.header {.dest = DSL_CTL_LED, .size = 1}, .data {static_cast<uint8_t>(~bmLED_GREEN & ~bmLED_RED)}};
   const usb_wr_cmd wr_cmd_led_red_on {.header {.dest = DSL_CTL_LED, .size = 1}, .data {static_cast<uint8_t>((bmLED_RED))}};
   const usb_wr_cmd wr_cmd_led_green_on {.header {.dest = DSL_CTL_LED, .size = 1}, .data {static_cast<uint8_t>((bmLED_GREEN))}};
   const usb_wr_cmd wr_cmd_fw_intrdy_low {.header {.dest = DSL_CTL_INTRDY, .size = 1}, .data {static_cast<uint8_t>(~bmWR_INTRDY)}};
   const usb_wr_cmd wr_cmd_fw_intrdy_high {.header {.dest = DSL_CTL_INTRDY, .size = 1}, .data {static_cast<uint8_t>(bmWR_INTRDY)}};
   const usb_wr_cmd wr_cmd_worldwide {.header {.dest = DSL_CTL_WORDWIDE, .size = 1}, .data {static_cast<uint8_t>(bmWR_WORDWIDE)}};
   const usb_wr_cmd wr_cmd_acquisition_start {.header {.dest = DSL_CTL_START, .size = 0}};
   const usb_wr_cmd wr_cmd_acquisition_stop {.header {.dest = DSL_CTL_STOP, .size = 0}};

   // sample conversion map
   const static float dsl_samples[256][8];

   explicit Impl(const std::string &name) : deviceName(name),
                                            operationMode(0),
                                            channelMode(0),
                                            testMode(0),
                                            totalChannels(0),
                                            validChannels(0),
                                            clockType(false),
                                            clockEdge(false),
                                            rleCompress(false),
                                            rleSupport(false),
                                            stream(false),
                                            filter(0),
                                            sampleratesMinIndex(0),
                                            sampleratesMaxIndex(0),
                                            triggerChannel(0),
                                            triggerHRate(0),
                                            triggerHPos(0),
                                            triggerHoldoff(0),
                                            triggerMargin(0)
   {
      log->debug("created DSLogicDevice [{}]", {deviceName});
   }

   ~Impl()
   {
      close();
      log->debug("destroy DSLogicDevice [{}]", {deviceName});
   }

   bool open(Mode mode)
   {
      if (usb.isOpen())
      {
         log->error("device already open!, close first");
         return false;
      }

      if (mode != Read)
      {
         log->warn("invalid device mode [{}]", {mode});
         return false;
      }

      if (deviceName.find(DEVICE_TYPE_PREFIX) != 0)
      {
         log->warn("invalid device name [{}]", {deviceName});
         return false;
      }

      for (auto &descriptor: Usb::list())
      {
         // search for DSLogic device profile
         for (auto &p: dsl_profiles)
         {
            if (!(descriptor.vid == p.vid && descriptor.pid == p.pid))
               continue;

            if (buildName(&p) != deviceName)
               continue;

            usb = Usb(descriptor);

            break;
         }

         if (usb.isValid())
            break;
      }

      if (!usb.isValid())
      {
         log->warn("unknown device name [{}]", {deviceName});
         return false;
      }

      log->info("opening DSLogic on bus {03} device {03}", {usb.descriptor().bus, usb.descriptor().address});

      if (!usb.open())
      {
         log->error("failed to open USB device");
         return false;
      }

      while (true)
      {
         profile = nullptr;

         if (!usbRead(rd_cmd_fw_version))
         {
            log->error("failed to get firmware version");
            break;
         }

         if (fwVersion.major != DSL_REQUIRED_VERSION_MAJOR)
         {
            log->error("expected firmware version {}.{} got {}.{}.", {DSL_REQUIRED_VERSION_MAJOR, DSL_REQUIRED_VERSION_MINOR, fwVersion.major, fwVersion.minor});
            break;
         }

         if (!(usb.isHighSpeed() || usb.isSuperSpeed()))
         {
            log->error("failed to open, usb speed is too low, speed type: {}", {usb.speed()});
            break;
         }

         if (!usb.claimInterface(USB_INTERFACE))
         {
            log->error("failed to claim USB interface {}", {USB_INTERFACE});
            break;
         }

         /* check profile. */
         for (auto &p: dsl_profiles)
         {
            // find device and initialize for selected profile
            if (usb.descriptor().vid == p.vid && usb.descriptor().pid == p.pid && usb.speed() == p.usb_speed)
            {
               profile = &p;

               // initialize device defaults
               initDevice();

               // initialize channel defaults
               initChannels();

               // device selected, break
               break;
            }
         }

         if (!profile)
         {
            log->error("no profile found for device {0x4}.{04x}", {usb.descriptor().vid, usb.descriptor().pid});
            break;
         }

         if (!usbRead(rd_cmd_hw_status))
         {
            log->error("failed to get hardware status");
            break;
         }

         // check if FPGA is already programmed
         if (!(hwStatus & bmFPGA_DONE))
         {
            std::string firmware;

            switch (thLevel)
            {
               case TH_3V3:
                  firmware = profile->fpga_bit33;
                  break;
               case TH_5V0:
                  firmware = profile->fpga_bit50;
                  break;
               default:
                  log->warn("unexpected threshold level: {}", {thLevel});
                  break;
            }

            if (firmware.empty())
            {
               log->error("invalid threshold level value {}", {thLevel});
               break;
            }

            if (!fpgaUpload(firmware))
            {
               log->error("failed to write firmware");
               break;
            }

            // dessert clear
            if (!i2cWrite(CTR0_ADDR, bmNONE))
            {
               log->error("failed to send command DSL_CTL_I2C_REG");
               break;
            }
         }
         else
         {
            // dessert clear
            if (!i2cWrite(CTR0_ADDR, bmNONE))
            {
               log->error("failed to send command DSL_CTL_I2C_REG");
               break;
            }

            // read FPGA version
            if (!usbRead(rd_cmd_fpga_version))
            {
               log->error("failed to read FPGA version");
               break;
            }

            if (fpgaVersion != DSL_HDL_VERSION && fpgaVersion)
            {
               log->error("incompatible FPGA version {}!", {fpgaVersion});
               break;
            }

            if (!usbWrite(wr_cmd_led_green_on))
            {
               log->error("failed to switch ON green led");
               break;
            }
         }

         uint16_t encryption[SECU_STEPS];

         if (!nvmRead(encryption, SECU_EEP_ADDR, sizeof(encryption)))
         {
            log->error("failed to read NVM security data");
            break;
         }

         // check security
         if (profile->dev_caps.feature_caps & CAPS_FEATURE_SECURITY)
         {
            if (!securityCheck(encryption, SECU_STEPS))
            {
               log->info("security check failed!");
               break;
            }
         }

         // set v threshold
         int vthVal = profile->dev_caps.feature_caps & CAPS_FEATURE_MAX25_VTH ? static_cast<uint8_t>(vth / 3.3 * (1.0 / 2.0) * 255) : static_cast<uint8_t>(vth / 3.3 * (1.5 / 2.5) * 255);

         if (!i2cWrite(VTH_ADDR, vthVal))
         {
            log->error("failed to set VTH threshold");
            break;
         }

         // set threshold
         if (profile->dev_caps.feature_caps & CAPS_FEATURE_ADF4360)
         {
            if (!adcSetup(adc_clk_init_500m))
            {
               log->error("failed to configure ADC");
               break;
            }
         }

         // fill device info
         deviceVendor = profile->vendor;
         deviceModel = profile->model;
         deviceSerial = "dslogic";
         deviceStatus = STATUS_READY;

         // finish initialization
         const Usb::Descriptor &desc = usb.descriptor();

         log->info("opened {} on bus {03} device {03}, firmware {}.{}, hw status {02x}, fpga {}", {std::string(profile->model), desc.bus, desc.address, fwVersion.major, fwVersion.minor, hwStatus, fpgaVersion});

         return true;
      }

      usb.close();

      profile = nullptr;

      return false;
   }

   void close()
   {
      if (usb.isOpen())
      {
         // stop acquisition
         stop();

         // release USB interface
         usb.releaseInterface(USB_INTERFACE);

         // close underlying USB device
         usb.close();

         // reset device profile
         profile = nullptr;
      }
   }

   int start(const StreamHandler &handler)
   {
      log->debug("starting acquisition for device {}", {deviceName});

      deviceStatus = STATUS_INIT;

      captureSamples = (limitSamples + SAMPLES_ALIGN) & ~SAMPLES_ALIGN;
      captureBytes = captureSamples / DSLOGIC_ATOMIC_SAMPLES * validChannels * DSLOGIC_ATOMIC_SIZE;

      currentSamples = 0;
      currentBytes = 0;

      droppedSamples = 0;
      droppedBytes = 0;

      buffer.reset();

      // stop previous acquisition
      if (!usbWrite(wr_cmd_acquisition_stop))
      {
         log->error("failed to stop previous acquisition");
         return -1;
      }

      // setting FPGA before acquisition start
      if (!fpgaSetup())
      {
         log->error("failed to setup FPGA");
         return -1;
      }

      // setup usb transfers
      beginTransfers(handler);

      // start acquisition
      if (!usbWrite(wr_cmd_acquisition_start))
      {
         log->error("failed to start acquisition");
         deviceStatus = STATUS_ERROR;
         return -1;
      }

      streamHandler = handler;
      deviceStatus = STATUS_START;

      log->debug("acquisition started for device {}", {deviceName});

      return 0;
   }

   int stop()
   {
      log->debug("stopping acquisition for device {}", {deviceName});

      // if device is not started, just return
      if (deviceStatus == STATUS_PAUSE)
         return 0;

      // stop previous acquisition
      if (!usbWrite(wr_cmd_acquisition_stop))
         log->error("failed to stop acquisition");

      /* adc power down*/
      if (profile->dev_caps.feature_caps & CAPS_FEATURE_HMCAD1511)
      {
         if (!adcSetup(adc_power_down))
            log->error("failed to power down ADC");
      }

      log->debug("cancel pending transfers for device {}", {deviceName});

      // cancel current transfers
      for (const auto transfer: transfers)
         usb.cancelTransfer(transfer);

      deviceStatus = STATUS_STOP;
      streamHandler = nullptr;

      log->debug("capture finished for device {}", {deviceName});

      return 0;
   }

   int pause()
   {
      log->debug("pause acquisition for device {}", {deviceName});

      if (deviceStatus != STATUS_DATA)
      {
         log->error("failed to pause acquisition, deice is not streaming");
         return -1;
      }

      // stop acquisition
      if (!usbWrite(wr_cmd_acquisition_stop))
         log->error("failed to pause acquisition");

      // cancel current transfers
      for (const auto transfer: transfers)
         usb.cancelTransfer(transfer);

      deviceStatus = STATUS_PAUSE;

      return 0;
   }

   int resume()
   {
      log->debug("resume acquisition for device {}", {deviceName});

      if (deviceStatus != STATUS_PAUSE)
      {
         log->error("failed to resume acquisition, deice is not paused");
         return -1;
      }

      buffer.reset();

      // setting FPGA before acquisition start
      if (!fpgaSetup())
      {
         log->error("failed to setup FPGA");
         return -1;
      }

      // setup usb transfers
      beginTransfers(streamHandler);

      // start acquisition
      if (!usbWrite(wr_cmd_acquisition_start))
      {
         log->error("failed to resume acquisition");
         deviceStatus = STATUS_ERROR;
         return -1;
      }

      deviceStatus = STATUS_START;

      return 0;
   }

   rt::Variant get(int id, int channel) const
   {
      switch (id)
      {
         case PARAM_DEVICE_NAME:
            return deviceName;

         case PARAM_DEVICE_VENDOR:
            return deviceVendor;

         case PARAM_DEVICE_MODEL:
            return deviceModel;

         case PARAM_DEVICE_SERIAL:
            return deviceSerial;

         case PARAM_DEVICE_VERSION:
            return deviceVersion;

         case PARAM_OPERATION_MODE:
            return operationMode;

         case PARAM_FILTER_MODE:
            return filter;

         case PARAM_RLE_COMPRESS:
            return rleCompress;

         case PARAM_TEST:
            return testMode != TEST_NONE;

         case PARAM_CHANNEL_MODE:
            return channelMode;

         case PARAM_CHANNEL_TOTAL:
            return totalChannels;

         case PARAM_CHANNEL_VALID:
            return validChannels;

         case PARAM_THRESHOLD_LEVEL:
            return thLevel;

         case PARAM_VOLTAGE_THRESHOLD:
            return vth;

         case PARAM_STREAM:
            return stream;

         case PARAM_STREAM_TIME:
            return streamTime;

         case PARAM_SAMPLE_RATE:
            return samplerate;

         case PARAM_SAMPLES_READ:
            return currentSamples;

         case PARAM_SAMPLES_LOST:
            return droppedSamples;

         case PARAM_TIMEBASE:
            return timebase;

         case PARAM_CLOCK_TYPE:
            return clockType;

         case PARAM_CLOCK_EDGE:
            return clockEdge;

         case PARAM_RLE_SUPPORT:
            return rleSupport;

         default:
            log->error("invalid configuration id {}", {id});
            return false;
      }
   }

   bool set(int id, const rt::Variant &value, int channel)
   {
      auto ch = std::find_if(channels.begin(), channels.end(), [channel](const dsl_channel &c) { return c.index == channel; });

      if (channel >= 0 && ch == channels.end())
      {
         log->error("invalid channel {}", {channel});
         return false;
      }

      switch (id)
      {
         case PARAM_SAMPLE_RATE:
         {
            if (auto v = std::get_if<unsigned int>(&value))
            {
               if (testMode != TEST_NONE)
               {
                  log->error("cannot set samplerate in test mode");
                  return false;
               }

               samplerate = *v;

               log->info("setting samplerate to {}", {samplerate});
               return true;
            }

            log->error("invalid value type for PARAM_SAMPLE_RATE");
            return false;
         }
         case PARAM_LIMIT_SAMPLES:
         {
            if (auto v = std::get_if<unsigned long long>(&value))
            {
               limitSamples = *v;
               log->info("setting limit samples to {}", {limitSamples});
               return true;
            }

            log->error("invalid value type for PARAM_LIMIT_SAMPLES");
            return false;
         }
         case PARAM_TIMEBASE:
         {
            if (auto v = std::get_if<unsigned int>(&value))
            {
               timebase = *v;
               log->info("setting timebase to {}", {timebase});
               return true;
            }

            log->error("invalid value type for PARAM_TIMEBASE");
            return false;
         }
         case PARAM_CLOCK_TYPE:
         {
            if (auto v = std::get_if<bool>(&value))
            {
               clockType = *v;
               log->info("setting clock type to {}", {clockType});
               return true;
            }

            log->error("invalid value type for PARAM_CLOCK_TYPE");
            return false;
         }
         case PARAM_CLOCK_EDGE:
         {
            if (auto v = std::get_if<bool>(&value))
            {
               clockEdge = *v;
               log->info("setting clock edge to {}", {clockEdge});
               return true;
            }

            log->error("invalid value type for PARAM_CLOCK_EDGE");
            return false;
         }
         case PARAM_RLE_SUPPORT:
         {
            if (auto v = std::get_if<bool>(&value))
            {
               rleSupport = *v;
               log->info("setting RLE support to {}", {rleSupport});
               return true;
            }

            log->error("invalid value type for PARAM_RLE_SUPPORT");
            return false;
         }
         case PARAM_RLE_COMPRESS:
         {
            if (auto v = std::get_if<bool>(&value))
            {
               rleCompress = *v;
               log->info("setting RLE mode to {}", {rleCompress});
               return true;
            }

            log->error("invalid value type for PARAM_RLE_COMPRESS");
            return false;
         }
         case PARAM_PROBE_VDIV:
         {
            if (auto v = std::get_if<int>(&value))
            {
               ch->vdiv = *v;

               log->info("setting VDIV of channel {} to {} mv", {ch->index, ch->vdiv});
               return true;
            }

            log->error("invalid value type for PARAM_PROBE_VDIV");
            return false;
         }
         case PARAM_PROBE_FACTOR:
         {
            if (auto v = std::get_if<int>(&value))
            {
               ch->vfactor = *v;
               log->info("setting VFACTOR of channel {} to {}", {ch->index, ch->vfactor});
               return true;
            }

            log->error("invalid value type for PARAM_PROBE_FACTOR");
            return false;
         }
         case PARAM_PROBE_COUPLING:
         {
            if (auto v = std::get_if<int>(&value))
            {
               ch->coupling = *v;

               if (ch->coupling == GND_COUPLING)
                  ch->coupling = DC_COUPLING;

               log->info("setting coupling of channel {} to {}", {ch->index, ch->coupling});
               return true;
            }

            log->error("invalid value type for PARAM_PROBE_COUPLING");
            return false;
         }
         case PARAM_PROBE_ENABLE:
         {
            if (auto v = std::get_if<bool>(&value))
            {
               if (testMode != TEST_NONE)
               {
                  log->error("cannot set probe in test mode");
                  return false;
               }

               ch->enabled = *v;

               // count enabled channels
               validChannels = std::count_if(channels.begin(), channels.end(), [](const dsl_channel &c) { return c.enabled; });

               log->info("setting channel {} enabled to {}", {ch->index, ch->enabled});
               return true;
            }

            log->error("invalid value type for PARAM_PROBE_ENABLE");
            return false;
         }
         case PARAM_TRIGGER_SOURCE:
         {
            if (auto v = std::get_if<int>(&value))
            {
               triggerSource = static_cast<TriggerSource>(*v);

               log->info("setting trigger source to {}", {triggerSource});
               return true;
            }

            log->error("invalid value type for PARAM_TRIGGER_SOURCE");
            return false;
         }
         case PARAM_TRIGGER_CHANNEL:
         {
            if (auto v = std::get_if<int>(&value))
            {
               triggerChannel = *v;

               log->info("setting trigger channel to {}", {triggerChannel});
               return true;
            }

            log->error("invalid value type for PARAM_TRIGGER_CHANNEL");
            return false;
         }
         case PARAM_TRIGGER_SLOPE:
         {
            if (auto v = std::get_if<int>(&value))
            {
               triggerSlope = static_cast<TriggerSlope>(*v);

               log->info("setting trigger slope to {}", {triggerSlope});
               return true;
            }

            log->error("invalid value type for PARAM_TRIGGER_SLOPE");
            return false;
         }
         case PARAM_TRIGGER_VALUE:
         {
            if (auto v = std::get_if<int>(&value))
            {
               ch->trig_value = *v;

               log->info("setting trigger value to {}", {ch->trig_value});
               return true;
            }

            log->error("invalid value type for PARAM_TRIGGER_VALUE");
            return false;
         }
         case PARAM_TRIGGER_HORIZPOS:
         {
            if (auto v = std::get_if<int>(&value))
            {
               triggerHPos = static_cast<int>(*v * limitSamples / 100.0);

               log->info("setting trigger horizontal position to {}", {triggerHPos});
               return true;
            }

            log->error("invalid value type for PARAM_TRIGGER_HORIZPOS");
            return false;
         }
         case PARAM_TRIGGER_HOLDOFF:
         {
            if (auto v = std::get_if<int>(&value))
            {
               triggerHoldoff = *v;

               log->info("setting trigger holdoff to {}", {triggerHoldoff});
               return true;
            }

            log->error("invalid value type for PARAM_TRIGGER_HOLDOFF");
            return false;
         }
         case PARAM_TRIGGER_MARGIN:
         {
            if (auto v = std::get_if<int>(&value))
            {
               triggerMargin = *v;

               log->info("setting trigger margin to {}", {triggerMargin});
               return true;
            }

            log->error("invalid value type for PARAM_TRIGGER_MARGIN");
            return false;
         }
         case PARAM_FILTER_MODE:
         {
            if (auto v = std::get_if<int>(&value))
            {
               if (!(*v == FILTER_NONE || *v == FILTER_1T))
               {
                  log->error("invalid filter value {}", {*v});
                  return false;
               }

               filter = *v;

               log->info("setting filter to {}", {filter});
               return true;
            }

            log->error("invalid value type for PARAM_FILTER_MODE");
            return false;
         }
         case PARAM_OPERATION_MODE:
         {
            if (auto v = std::get_if<int>(&value))
            {
               if (operationMode != *v)
               {
                  if (*v == OP_BUFFER)
                  {
                     stream = false;
                     testMode = TEST_NONE;

                     for (int i = 0; i < std::size(channel_modes); i++)
                     {
                        if (channel_modes[i].mode == LOGIC && channel_modes[i].stream == stream && profile->dev_caps.channels & (1 << i))
                        {
                           channelMode = channel_modes[i].id;
                           break;
                        }
                     }
                  }
                  else if (*v == OP_STREAM)
                  {
                     stream = true;
                     testMode = TEST_NONE;

                     for (int i = 0; i < std::size(channel_modes); i++)
                     {
                        if (channel_modes[i].mode == LOGIC && channel_modes[i].stream == stream && profile->dev_caps.channels & (1 << i))
                        {
                           channelMode = channel_modes[i].id;
                           break;
                        }
                     }
                  }
                  else if (*v == OP_INTEST)
                  {
                     testMode = TEST_INTERNAL;
                     channelMode = profile->dev_caps.intest_channel;
                     stream = !(profile->dev_caps.feature_caps & CAPS_FEATURE_BUF);
                  }
                  else
                  {
                     log->error("invalid PARAM_OPERATION_MODE {}", {*v});
                     return false;
                  }

                  // update operation mode
                  operationMode = *v;

                  // setup probes
                  initChannels();

                  // adjust samplerate
                  adjustSamplerate();

                  // internal test parameters
                  if (operationMode == OP_INTEST)
                  {
                     samplerate = stream ? channel_modes[channelMode].max_samplerate / 10 : DEV_MHZ(100);
                     limitSamples = stream ? static_cast<unsigned long long>(samplerate) * 3 : profile->dev_caps.hw_depth / validChannels;
                  }
               }

               log->info("setting operation mode to {}", {operationMode});
               return true;
            }

            log->error("invalid value type for PARAM_OPERATION_MODE");
            return false;
         }
         case PARAM_CHANNEL_MODE:
         {
            if (auto v = std::get_if<int>(&value))
            {
               if (testMode != TEST_NONE)
               {
                  log->error("cannot set channels in test mode");
                  return false;
               }

               for (int i = 0; i < sizeof(channel_modes) / sizeof(dsl_channel_mode); i++)
               {
                  if (profile->dev_caps.channels & (1 << i))
                  {
                     if (static_cast<int>(channel_modes[i].id) == *v)
                     {
                        channelMode = *v;
                        break;
                     }
                  }
               }

               if (channelMode != *v)
               {
                  log->error("invalid channel mode {}", {*v});
                  return false;
               }

               // setup probes
               initChannels();

               // adjust samplerate
               adjustSamplerate();

               log->info("setting channel mode to {}", {channelMode});
               return true;
            }

            log->error("invalid value type for PARAM_CHANNEL_MODE");
            return false;
         }
         case PARAM_THRESHOLD_LEVEL:
         {
            if (auto v = std::get_if<int>(&value))
            {
               if (testMode != TEST_NONE)
               {
                  log->error("cannot set threshold level in test mode");
                  return false;
               }

               if (thLevel != *v)
               {
                  if (*v != TH_3V3 && *v != TH_5V0)
                  {
                     log->error("invalid threshold level {}", {*v});
                     return false;
                  }

                  thLevel = *v;
                  std::string firmware;

                  switch (thLevel)
                  {
                     case TH_3V3:
                        firmware = profile->fpga_bit33;
                        break;
                     case TH_5V0:
                        firmware = profile->fpga_bit50;
                        break;
                     default:
                        log->error("invalid PARAM_THRESHOLD_LEVEL value {}", {thLevel});
                        return false;
                  }

                  if (!fpgaUpload(firmware))
                  {
                     log->error("failed to write firmware");
                     return false;
                  }
               }

               log->info("setting threshold level to {}", {thLevel});
               return true;
            }

            log->error("invalid value type for PARAM_THRESHOLD_LEVEL");
            return false;
         }
         case PARAM_VOLTAGE_THRESHOLD:
         {
            if (auto v = std::get_if<float>(&value))
            {
               if (testMode != TEST_NONE)
               {
                  log->error("cannot set VTH in test mode");
                  return false;
               }

               vth = *v;

               int vthVal = profile->dev_caps.feature_caps & CAPS_FEATURE_MAX25_VTH ? static_cast<uint8_t>(vth / 3.3 * (1.0 / 2.0) * 255) : static_cast<uint8_t>(vth / 3.3 * (1.5 / 2.5) * 255);

               if (!i2cWrite(VTH_ADDR, vthVal))
               {
                  log->error("failed to set PARAM_VOLTAGE_THRESHOLD threshold");
                  return false;
               }

               log->info("setting voltage threshold to {}", {vth});
               return true;
            }

            log->error("invalid value type for PARAM_VOLTAGE_THRESHOLD");
            return false;
         }
         case PARAM_STREAM:
         {
            if (auto v = std::get_if<bool>(&value))
            {
               stream = *v;

               log->info("setting stream to {}", {stream});
               return true;
            }

            log->error("invalid value type for PARAM_STREAM");
            return false;
         }
         case PARAM_FIRMWARE_PATH:
         {
            if (auto v = std::get_if<std::string>(&value))
            {
               firmwarePath = *v;

               log->info("setting firmware path to {}", {firmwarePath});
               return true;
            }

            log->error("invalid value type for PARAM_FIRMWARE_PATH");
            return false;
         }
         default:
            log->error("unknown configuration id {}", {id});
            return false;
      }
   }

   void initDevice()
   {
      // device flags
      operationMode = OP_STREAM;
      testMode = TEST_NONE;
      thLevel = TH_3V3;
      filter = FILTER_NONE;

      // device settings
      samplerate = profile->dev_caps.default_samplerate;
      limitSamples = profile->dev_caps.default_samplelimit;
      channelMode = profile->dev_caps.default_channelid;
      timebase = 10000;
      clockType = false;
      clockEdge = false;
      rleCompress = false;
      stream = true;
      vth = 1.0;

      // trigger settings
      triggerSlope = TRIGGER_RISING;
      triggerSource = TRIGGER_AUTO;
      triggerChannel = 0;
      triggerHPos = 0;
      triggerHRate = 0;
      triggerHoldoff = 0;

      // channels
      totalChannels = 0;
      validChannels = 0;

      /*
       * Trigger settings.
       */
      trigger.trigger_enabled = 0;
      trigger.trigger_mode = SIMPLE_TRIGGER;
      trigger.trigger_position = 0;
      trigger.trigger_stages = 0;

      for (int i = 0; i <= NUM_TRIGGER_STAGES; i++)
      {
         for (int j = 0; j < NUM_TRIGGER_PROBES; j++)
         {
            trigger.trigger0[i][j] = 'X';
            trigger.trigger1[i][j] = 'X';
         }

         trigger.trigger0_count[i] = 0;
         trigger.trigger1_count[i] = 0;
         trigger.trigger0_inv[i] = 0;
         trigger.trigger1_inv[i] = 0;
         trigger.trigger_logic[i] = 1;
      }

      adjustSamplerate();
   }

   int initChannels()
   {
      channels.clear();

      for (int i = 0; i < channel_modes[channelMode].vld_num; i++)
      {
         dsl_channel channel {
            .index = i,
            .type = channel_modes[channelMode].type,
            .enabled = true,
            .name = probe_names[i],
            .bits = channel_modes[channelMode].unit_bits,
            .vdiv = 1000,
            .vfactor = 1,
            .offset = (1 << (channel.bits - 1)),
            .vpos_trans = profile->dev_caps.default_pwmtrans,
            .coupling = DC_COUPLING,
            .trig_value = (1 << (channel.bits - 1)),
            .comb_comp = profile->dev_caps.default_comb_comp,
            .digi_fgain = 0,
            .cali_fgain0 = 1,
            .cali_fgain1 = 1,
            .cali_fgain2 = 1,
            .cali_fgain3 = 1,
            .cali_comb_fgain0 = 1,
            .cali_comb_fgain1 = 1,
            .cali_comb_fgain2 = 1,
            .cali_comb_fgain3 = 1,
            .map_default = true,
            .map_unit = probe_units[0],
            .map_min = -(static_cast<double>(channel.vdiv) * static_cast<double>(channel.vfactor) * DS_CONF_DSO_VDIVS / 2000.0),
            .map_max = static_cast<double>(channel.vdiv) * static_cast<double>(channel.vfactor) * DS_CONF_DSO_VDIVS / 2000.0,
         };

         if (profile->dev_caps.vdivs)
         {
            for (int j = 0; profile->dev_caps.vdivs[j]; j++)
            {
               dsl_vga vga {
                  .id = profile->dev_caps.vga_id,
                  .key = profile->dev_caps.vdivs[j],
                  .vgain = 0,
                  .preoff = 0,
                  .preoff_comp = 0,
               };

               for (const auto &vga_default: vga_defaults)
               {
                  if (vga_default.id == profile->dev_caps.vga_id && vga_default.key == profile->dev_caps.vdivs[j])
                  {
                     vga.vgain = vga_defaults[j].vgain;
                     vga.preoff = vga_defaults[j].preoff;
                     vga.preoff_comp = 0;
                  }
               }

               channel.vga_list.push_back(vga);
            }
         }

         channels.push_back(channel);
      }

      totalChannels = channel_modes[channelMode].vld_num;
      validChannels = channel_modes[channelMode].vld_num;

      return true;
   }

   bool beginTransfers(const StreamHandler &handler)
   {
      // create header buffer
      auto *transfer = new Usb::Transfer();
      transfer->data = new unsigned char[headerSize()];
      transfer->available = headerSize();
      transfer->timeout = 30000;
      transfer->callback = [=](Usb::Transfer *t) -> Usb::Transfer * { return usbProcessHeader(t); };

      // add transfer to device list
      transfers.push_back(transfer);

      // submit transfer of header buffer
      usb.asyncTransfer(Usb::In, 6, transfer);

      log->debug("usb transfer header size {}", {headerSize()});
      log->debug("usb transfer buffer size {}", {bufferSize()});

      // submit transfer of data buffers
      for (int i = 0; i < totalTransfers(); i++)
      {
         transfer = new Usb::Transfer();
         transfer->data = new unsigned char[bufferSize()];
         transfer->available = bufferSize();
         transfer->timeout = 5000;
         transfer->callback = [=](Usb::Transfer *t) -> Usb::Transfer * { return usbProcessData(t, handler); };

         // clean buffer
         memset(transfer->data, 0, transfer->available);

         // add transfer to device list
         transfers.push_back(transfer);

         // submit transfer of data buffer
         if (!usb.asyncTransfer(Usb::In, 6, transfer))
         {
            log->error("failed to setup async transfer: {}", {usb.lastError()});

            for (Usb::Transfer *t: transfers)
               usb.cancelTransfer(t);

            break;
         }
      }

      return true;
   }

   bool securityCheck(uint16_t *encryption, int steps) const
   {
      uint16_t temp;
      int tryCnt = SECU_TRY_CNT;

      log->info("perform security check");

      // reset security
      if (!securityReset())
         return false;

      // check security pass
      if (securityStatus(bmSECU_PASS))
         return false;

      // security write
      if (!securityWrite(SECU_START, 0))
         return false;

      while (steps--)
      {
         if (securityStatus(bmSECU_PASS))
            return false;

         while (!securityStatus(bmSECU_READY))
         {
            if (tryCnt-- == 0)
            {
               log->error("get security ready failed");
               return false;
            }
         }

         if (!securityRead(temp))
            return false;

         if (temp != 0)
            return false;

         if (!securityWrite(SECU_CHECK, encryption[steps]))
            return false;
      }

      log->info("security check pass!");

      return true;
   }

   bool securityReset() const
   {
      if (!i2cWrite(SEC_CTRL_ADDR + 0, 0))
         return false;

      if (!i2cWrite(SEC_CTRL_ADDR + 1, 0))
         return false;

      usleep(10 * 1000);

      if (!i2cWrite(SEC_CTRL_ADDR + 0, 1))
         return false;

      if (!i2cWrite(SEC_CTRL_ADDR + 1, 0))
         return false;

      return true;
   }

   bool securityRead(uint16_t &data) const
   {
      if (!i2cRead(SEC_DATA_ADDR + 1, reinterpret_cast<uint8_t *>(&data)))
         return false;

      data <<= 8;

      if (!i2cRead(SEC_DATA_ADDR + 0, reinterpret_cast<uint8_t *>(&data)))
         return false;

      return true;
   }

   bool securityWrite(uint16_t cmd, uint16_t data) const
   {
      if (!i2cWrite(SEC_DATA_ADDR + 0, data & 0xff))
         return false;

      if (!i2cWrite(SEC_DATA_ADDR + 1, data >> 8))
         return false;

      if (!i2cWrite(SEC_CTRL_ADDR + 0, cmd & 0xff))
         return false;

      if (!i2cWrite(SEC_CTRL_ADDR + 1, cmd >> 8))
         return false;

      return true;
   }

   bool securityStatus(uint8_t mask) const
   {
      uint8_t value;

      if (!i2cRead(SEC_CTRL_ADDR + 0, &value))
         return false;

      return value & mask;
   }

   bool waitStatus(int flags, int timeout = 1000) const
   {
      // get current time
      auto start = std::chrono::system_clock::now();

      do
      {
         if (!usbRead(rd_cmd_hw_status))
            return false;

         // if timeout exceeded, return false
         if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() > timeout)
            return false;

      }
      while (!(hwStatus & flags));

      return true;
   }

   bool fpgaUpload(const std::string &firmware) const
   {
      std::fstream file;

      log->info("uploading firmware {} to FPGA", {firmware});

      if (firmwarePath.empty())
         file.open("./firmware/" + firmware, std::ios::in | std::ios::binary);
      else
         file.open(firmwarePath + "/" + firmware, std::ios::in | std::ios::binary);

      if (!file.is_open())
      {
         log->error("failed to open firmware configuration file {}", {firmware});
         return false;
      }

      // get filesize
      file.seekg(0, std::ios::end);
      uint64_t filesize = file.tellg();
      file.seekg(0, std::ios::beg);

      // step0: assert PROG_B low
      if (!usbWrite(wr_cmd_prog_b_low))
         return false;

      // step1: turn off GREEN/RED led
      if (!usbWrite(wr_cmd_led_off))
         return false;

      // step2: assert PORG_B high
      if (!usbWrite(wr_cmd_b_high))
         return false;

      // step3: wait INIT_B go high
      if (!waitStatus(bmFPGA_INIT_B))
         return false;

      // step4: assert INTRDY low (indicate data start)
      if (!usbWrite(wr_cmd_fw_intrdy_low))
         return false;

      // step4: send firmware size command
      if (!usbWrite({.header = {.dest = DSL_CTL_BULK_WR, .size = 3}, .data {static_cast<uint8_t>(filesize), static_cast<uint8_t>(filesize >> 8), static_cast<uint8_t>(filesize >> 16)}}))
         return false;

      // step5: send firmware data
      std::vector<unsigned char> buffer((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

      int transferred = usb.syncTransfer(Usb::Out, 2, buffer.data(), static_cast<int>(buffer.size()));

      if (transferred < 0)
      {
         log->error("failed to send firmware data: ", {usb.lastError()});
         return false;
      }

      if (transferred != buffer.size())
      {
         log->error("configure FPGA error: expected transfer size {} actually {}.", {static_cast<int>(buffer.size()), transferred});
         return false;
      }

      // step6: assert INTRDY high (indicate data end)
      if (!usbWrite(wr_cmd_fw_intrdy_high))
         return false;

      // step7: check GPIF_DONE
      if (!waitStatus(bmGPIF_DONE))
         return false;

      // step8: assert INTRDY low
      if (!usbWrite(wr_cmd_fw_intrdy_low))
         return false;

      // step9: check FPGA_DONE bit
      if (!waitStatus(bmFPGA_DONE))
         return false;

      // step10: turn on GREEN led
      if (!usbWrite(wr_cmd_led_green_on))
         return false;

      // step11: recover GPIF to be wordwide
      if (!usbWrite(wr_cmd_worldwide))
         return false;

      log->info("firmware upload done, {} bytes sent", {static_cast<int>(buffer.size())});

      return true;
   }

   bool fpgaSetup()
   {
      bool qutrTrig;
      bool halfTrig;
      int transferred;

      log->info("arming FPGA to start acquisition");

      dsl_setting settings {
         .sync = 0xf5a5f5a5,
         .mode_header = 0x0001,
         .divider_header = 0x0102,
         .count_header = 0x0302,
         .trig_pos_header = 0x0502,
         .trig_glb_header = 0x0701,
         .dso_count_header = 0x0802,
         .ch_en_header = 0x0a02,
         .fgain_header = 0x0c01,
         .trig_header = 0x40a0,
         .end_sync = 0xfa5afa5a,
      };

      dsl_setting_ext32 settingsExt32 {
         .sync = 0xf5a5f5a5,
         .trig_header = 0x6060,
         .align_bytes = 0xffff,
         .end_sync = 0xfa5afa5a,
      };

      // basic configuration
      settings.mode += (trigger.trigger_enabled << TRIG_EN_BIT);
      settings.mode += (clockType << CLK_TYPE_BIT);
      settings.mode += (clockEdge << CLK_EDGE_BIT);
      settings.mode += (rleCompress << RLE_MODE_BIT);
      settings.mode += ((samplerate == profile->dev_caps.half_samplerate) << HALF_MODE_BIT);
      settings.mode += ((samplerate == profile->dev_caps.quarter_samplerate) << QUAR_MODE_BIT);
      settings.mode += ((filter == FILTER_1T) << FILTER_BIT);
      settings.mode += ((bytesPerMs() < 1024) << SLOW_ACQ_BIT);
      settings.mode += ((trigger.trigger_mode == SERIAL_TRIGGER) << STRIG_MODE_BIT);
      settings.mode += ((stream) << STREAM_MODE_BIT);
      settings.mode += ((testMode == TEST_LOOPBACK) << LPB_TEST_BIT);
      settings.mode += ((testMode == TEST_EXTERNAL) << EXT_TEST_BIT);
      settings.mode += ((testMode == TEST_INTERNAL) << INT_TEST_BIT);

      // sample rate divider
      unsigned int pre = ceil(static_cast<double>(channel_modes[channelMode].hw_max_samplerate) / static_cast<double>(samplerate));
      unsigned int div = ceil(static_cast<double>(pre) / channel_modes[channelMode].pre_div);

      if (pre > channel_modes[channelMode].pre_div)
         pre = channel_modes[channelMode].pre_div;

      settings.div_l = (div & 0xffff);
      settings.div_h = (div >> 16) + ((pre - 1) << 8);

      // capture counters
      settings.cnt_l = (captureSamples >> 4) & 0x0000ffff; // hardware minimum unit is 16 logic samples
      settings.cnt_h = (captureSamples >> 20);
      settings.dso_cnt_l = captureSamples & 0x0000ffff; // // hardware minimum unit is 1 analog sample
      settings.dso_cnt_h = captureSamples >> 16;

      // trigger position, must be align to minimum parallel bits
      auto tpos = static_cast<unsigned int>(fmax(static_cast<unsigned int>(trigger.trigger_position / 100.0 * limitSamples), DSLOGIC_ATOMIC_SAMPLES));

      if (stream)
         tpos = static_cast<unsigned int>(fmin(tpos, static_cast<double>(channelDepth()) * DS_MIN_TRIG_PERCENT / 100));
      else
         tpos = static_cast<unsigned int>(fmin(tpos, static_cast<double>(channelDepth()) * DS_MAX_TRIG_PERCENT / 100));

      settings.tpos_l = tpos & DSLOGIC_ATOMIC_MASK;
      settings.tpos_h = tpos >> 16;

      // trigger global settings
      settings.trig_glb = ((validChannels & 0x1f) << 8) + (trigger.trigger_stages & 0x00ff);

      // channel enable mapping
      settings.ch_en_l = 0;
      settings.ch_en_h = 0;

      for (auto &channel: channels)
      {
         if (channel.index < 16)
            settings.ch_en_l += channel.enabled << channel.index;
         else
            settings.ch_en_h += channel.enabled << (channel.index - 16);
      }

      // digital fgain
      for (auto &channel: channels)
      {
         settings.fgain = channel.digi_fgain;
         break;
      }

      // trigger advanced configuration
      if (trigger.trigger_mode == SIMPLE_TRIGGER)
      {
         qutrTrig = !(profile->dev_caps.feature_caps & CAPS_FEATURE_ADF4360) && (settings.mode & (1 << QUAR_MODE_BIT));
         halfTrig = (!(profile->dev_caps.feature_caps & CAPS_FEATURE_ADF4360) && settings.mode & (1 << HALF_MODE_BIT)) || ((profile->dev_caps.feature_caps & CAPS_FEATURE_ADF4360) && settings.mode & (1 << QUAR_MODE_BIT));

         settings.trig_mask0[0] = triggerMask0(NUM_TRIGGER_STAGES, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
         settings.trig_mask1[0] = triggerMask1(NUM_TRIGGER_STAGES, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
         settings.trig_value0[0] = triggerValue0(NUM_TRIGGER_STAGES, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
         settings.trig_value1[0] = triggerValue1(NUM_TRIGGER_STAGES, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
         settings.trig_edge0[0] = triggerEdge0(NUM_TRIGGER_STAGES, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
         settings.trig_edge1[0] = triggerEdge1(NUM_TRIGGER_STAGES, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);

         settingsExt32.trig_mask0[0] = triggerMask0(NUM_TRIGGER_STAGES, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
         settingsExt32.trig_mask1[0] = triggerMask1(NUM_TRIGGER_STAGES, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
         settingsExt32.trig_value0[0] = triggerValue0(NUM_TRIGGER_STAGES, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
         settingsExt32.trig_value1[0] = triggerValue1(NUM_TRIGGER_STAGES, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
         settingsExt32.trig_edge0[0] = triggerEdge0(NUM_TRIGGER_STAGES, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
         settingsExt32.trig_edge1[0] = triggerEdge1(NUM_TRIGGER_STAGES, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);

         settings.trig_logic0[0] = (trigger.trigger_logic[NUM_TRIGGER_STAGES] << 1) + trigger.trigger0_inv[NUM_TRIGGER_STAGES];
         settings.trig_logic1[0] = (trigger.trigger_logic[NUM_TRIGGER_STAGES] << 1) + trigger.trigger1_inv[NUM_TRIGGER_STAGES];

         settings.trig_count[0] = trigger.trigger0_count[NUM_TRIGGER_STAGES];

         for (int i = 1; i < NUM_TRIGGER_STAGES; i++)
         {
            settings.trig_mask0[i] = 0xffff;
            settings.trig_mask1[i] = 0xffff;
            settings.trig_value0[i] = 0;
            settings.trig_value1[i] = 0;
            settings.trig_edge0[i] = 0;
            settings.trig_edge1[i] = 0;
            settings.trig_logic0[i] = 2;
            settings.trig_logic1[i] = 2;
            settings.trig_count[i] = 0;

            settingsExt32.trig_mask0[i] = 0xffff;
            settingsExt32.trig_mask1[i] = 0xffff;
            settingsExt32.trig_value0[i] = 0;
            settingsExt32.trig_value1[i] = 0;
            settingsExt32.trig_edge0[i] = 0;
            settingsExt32.trig_edge1[i] = 0;
         }
      }
      else
      {
         for (int i = 0; i < NUM_TRIGGER_STAGES; i++)
         {
            if (settings.mode & (1 << STRIG_MODE_BIT) && i == S_TRIGGER_DATA_STAGE)
            {
               qutrTrig = false;
               halfTrig = false;
            }
            else
            {
               qutrTrig = !(profile->dev_caps.feature_caps & CAPS_FEATURE_ADF4360) && (settings.mode & (1 << QUAR_MODE_BIT));
               halfTrig = (!(profile->dev_caps.feature_caps & CAPS_FEATURE_ADF4360) && settings.mode & (1 << HALF_MODE_BIT)) || ((profile->dev_caps.feature_caps & CAPS_FEATURE_ADF4360) && settings.mode & (1 << QUAR_MODE_BIT));
            }

            settings.trig_mask0[i] = triggerMask0(i, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
            settings.trig_mask1[i] = triggerMask1(i, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
            settings.trig_value0[i] = triggerValue0(i, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
            settings.trig_value1[i] = triggerValue1(i, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
            settings.trig_edge0[i] = triggerEdge0(i, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
            settings.trig_edge1[i] = triggerEdge1(i, NUM_TRIGGER_PROBES - 1, 0, qutrTrig, halfTrig);
            settings.trig_logic0[i] = (trigger.trigger_logic[i] << 1) + trigger.trigger0_inv[i];
            settings.trig_logic1[i] = (trigger.trigger_logic[i] << 1) + trigger.trigger1_inv[i];
            settings.trig_count[i] = trigger.trigger0_count[i];

            settingsExt32.trig_mask0[i] = triggerMask0(i, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
            settingsExt32.trig_mask1[i] = triggerMask1(i, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
            settingsExt32.trig_value0[i] = triggerValue0(i, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
            settingsExt32.trig_value1[i] = triggerValue1(i, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
            settingsExt32.trig_edge0[i] = triggerEdge0(i, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
            settingsExt32.trig_edge1[i] = triggerEdge1(i, 2 * NUM_TRIGGER_PROBES - 1, NUM_TRIGGER_PROBES, qutrTrig, halfTrig);
         }
      }

      if (profile->usb_speed != LIBUSB_SPEED_SUPER)
      {
         if (!usbWrite(wr_cmd_worldwide))
         {
            log->error("failed to set GPIF to be worldwide");
            return false;
         }
      }

      // send bulk write control command
      int armSize = sizeof(struct dsl_setting) / sizeof(uint16_t);

      if (!usbWrite({.header = {.dest = DSL_CTL_BULK_WR, .size = 3}, .data {static_cast<uint8_t>(armSize), static_cast<uint8_t>(armSize >> 8), static_cast<uint8_t>(armSize >> 16)}}))
      {
         log->error("failed to send bulk write command of arm FPGA");
         return false;
      }

      // check sys_clr dessert
      if (!waitStatus(bmSYS_CLR))
      {
         log->error("failed to check FPGA dessert clear");
         return false;
      }

      // send bulk data setting
      transferred = usb.syncTransfer(Usb::Out, 2, &settings, sizeof(dsl_setting), 1000);

      if (transferred < 0)
      {
         log->error("failed to send bulk data setting of arm FPGA: {}", {usb.lastError()});
         return false;
      }

      if (transferred != sizeof(dsl_setting))
      {
         log->error("configure FPGA error: expected transfer size {} actually {}.", {sizeof(dsl_setting), transferred});
         return false;
      }

      // setting_ext32
      if (profile->dev_caps.feature_caps & CAPS_FEATURE_LA_CH32)
      {
         transferred = usb.syncTransfer(Usb::Out, 2, &settingsExt32, sizeof(dsl_setting_ext32), 1000);

         if (transferred < 0)
         {
            log->error("failed to send bulk data setting of arm FPGA(setting_ext32): {}", {usb.lastError()});
            return false;
         }

         if (transferred != sizeof(dsl_setting_ext32))
         {
            log->error("configure FPGA(setting_ext32) error: expected transfer size {} actually {}.", {sizeof(dsl_setting_ext32), transferred});
            return false;
         }
      }

      // assert INTRDY high (indicate data end)
      if (!usbWrite(wr_cmd_fw_intrdy_high))
      {
         log->error("failed to set INTRDY high");
         return false;
      }

      // check GPIF_DONE bit
      if (!waitStatus(bmGPIF_DONE))
      {
         log->error("failed to check FPGA_DONE bit");
         return false;
      }

      log->info("setup FPGA successful");

      return true;
   }

   bool adcSetup(const dsl_adc_config *config) const
   {
      log->info("configuring ADC");

      while (config->dest)
      {
         if ((config->cnt > 0) && (config->cnt <= 4))
         {
            if (config->delay > 0)
               usleep(config->delay * 1000);

            for (int i = 0; i < config->cnt; i++)
            {
               if (!i2cWrite(config->dest, config->byte[i]))
                  return false;
            }
         }

         config++;
      }

      return true;
   }

   bool nvmRead(void *data, uint16_t addr, uint8_t len) const
   {
      if (!usbRead({.header = {.dest = DSL_CTL_NVM, .offset = addr, .size = len}, .data = data}))
      {
         log->error("failed to read NVM address {04x}, size {}", {addr, len});
         return false;
      }

      return true;
   }

   bool i2cRead(uint8_t addr, uint8_t *value) const
   {
      if (!usbRead({.header = {.dest = DSL_CTL_I2C_STATUS, .offset = addr, .size = 1}, .data = value}))
      {
         log->error("DSL_CTL_I2C_STATUS read command failed for address {}", {addr});
         return false;
      }

      return true;
   }

   bool i2cWrite(uint8_t addr, uint8_t value) const
   {
      log->debug("i2cWrite, addr {04x}, value {04x}", {addr, value});

      if (!usbWrite({.header = {.dest = DSL_CTL_I2C_REG, .offset = addr, .size = 1}, .data = {value}}))
      {
         log->error("DSL_CTL_I2C_REG write command failed for address {}, value {}", {addr, value});
         return false;
      }

      return true;
   }

   bool usbRead(const usb_rd_cmd &rd_cmd) const
   {
      if (!usb.ctrlTransfer(CMD_CTL_RD_PRE, &rd_cmd, sizeof(usb_header), CMD_CTL_RD, rd_cmd.data, rd_cmd.header.size))
      {
         log->error("usb transfer CMD_CTL_RD failed, command {}, offset {}, size {}", {rd_cmd.header.dest, rd_cmd.header.offset, rd_cmd.header.size});
         return false;
      }

      return true;
   }

   bool usbWrite(const usb_wr_cmd &wr_cmd) const
   {
      if (!usb.ctrlTransfer(CMD_CTL_WR, &wr_cmd, sizeof(usb_header) + wr_cmd.header.size, 0, nullptr, 0))
      {
         log->error("usb transfer CMD_CTL_WR failed, command {}, offset {}, size {}", {wr_cmd.header.dest, wr_cmd.header.offset, wr_cmd.header.size});
         return false;
      }

      return true;
   }

   Usb::Transfer *usbProcessHeader(Usb::Transfer *transfer)
   {
      if (deviceStatus != STATUS_ABORT)
         deviceStatus = STATUS_ERROR;

      auto *triggerPos = reinterpret_cast<dsl_trigger_pos *>(transfer->data);

      if (transfer->status == Usb::Completed && triggerPos->check_id == TRIG_CHECKID)
      {
         unsigned long long remainCount = (static_cast<unsigned long long>(triggerPos->remain_cnt_h) << 32) + triggerPos->remain_cnt_l;

         if (transfer->actual == headerSize() && (stream || remainCount < limitSamples))
         {
            if (!stream || deviceStatus == STATUS_ABORT)
            {
               captureBytes = ((limitSamples - remainCount) & ~SAMPLES_ALIGN) / (DSLOGIC_ATOMIC_SAMPLES * DSLOGIC_ATOMIC_SIZE * validChannels);
               captureSamples = captureBytes / (validChannels << 3);
            }

            deviceStatus = STATUS_DATA;
         }
      }
      else
      {
         log->error("header transfer failed with USB status {}", {transfer->status});
      }

      // remove transfer from list
      transfers.remove(transfer);

      // free header buffer
      delete transfer->data;

      // free transfer
      delete transfer;

      log->debug("finish header transfer, remain {} transfers", {transfers.size()});

      // no resend transfer
      return nullptr;
   }

   Usb::Transfer *usbProcessData(Usb::Transfer *transfer, const StreamHandler &handler)
   {
      if (deviceStatus == STATUS_START)
         deviceStatus = STATUS_DATA;

      switch (transfer->status)
      {
         case Usb::Completed:
         case Usb::TimeOut:
            break;
         case Usb::Cancelled:
            log->debug("data transfer cancelled with USB status {}", {transfer->status});
            break;
         default:
            log->error("data transfer failed with USB status {}", {transfer->status});
            deviceStatus = STATUS_ERROR;
            break;
      }

      // trigger next transfer
      if (deviceStatus == STATUS_DATA && transfer->actual != 0)
      {
         // interleave received data in single buffer with N channels stride
         std::vector<SignalBuffer> buffers = interleave(transfer);

         // call user handler for each channel
         for (auto &b: buffers)
         {
            if (!handler(b))
            {
               log->warn("data transfer stopped by handler, aborting!");
               deviceStatus = STATUS_ABORT;
               break;
            }
         }

         if (deviceStatus == STATUS_DATA)
         {
            // reset transfer buffer received size
            transfer->actual = 0;

            // clean buffer
            memset(transfer->data, 0, transfer->available);

            // resend new transfer
            return transfer;
         }
      }

      // remove transfer from list
      transfers.remove(transfer);

      // free data buffer
      delete transfer->data;

      // free transfer
      delete transfer;

      log->debug("finish data transfer, remain {} transfers", {transfers.size()});

      // no resend transfer
      return nullptr;
   }

   std::vector<SignalBuffer> interleave(Usb::Transfer *transfer)
   {
      std::vector<SignalBuffer> result;

      unsigned int start = 0; // source buffer start index
      unsigned int chunk = validChannels << 3; // minimum chunk size in bytes
      unsigned int block = chunk << 3;
      unsigned int round = CHANNEL_BUFFER_SIZE % block;
      unsigned int size = CHANNEL_BUFFER_SIZE - round;

      /*
       * if have incomplete buffer, fill it with new received data
       */
      if (buffer.isValid())
      {
         const unsigned int filled = (currentBytes % (size >> 3)) << 3; // filled samples

         start = transpose(buffer, filled % chunk, transfer->data, 0, transfer->actual);

         buffer.flip();

         result.push_back(buffer);

         buffer.reset();
      }

      /*
       * Interleave data from transfer buffer
       */
      // number of full buffers than can be processed with remain data
      const unsigned int remain = transfer->actual - start;
      const unsigned int buffers = remain / (size >> 3) + (remain % (size >> 3) ? 1 : 0);

#pragma omp parallel for default(none) shared(start, transfer, result, size, buffers, currentSamples) schedule(static)
      for (unsigned int k = 0; k < buffers; ++k)
      {
         // sample start position
         const unsigned long long bufferOffset = currentSamples + k * (size / validChannels);

         // create new buffer for interleaved data
         SignalBuffer data(size, validChannels, 1, samplerate, bufferOffset, 0, SIGNAL_TYPE_LOGIC_SAMPLES);

         // transpose data from transfer buffer to interleaved buffer
         transpose(data, 0, transfer->data, start + k * (size >> 3), transfer->actual);

#pragma omp critical
         {
            // or if buffer is not full, keep it for next transfer
            if (data.isFull())
            {
               data.flip();
               result.push_back(data);
            }
            // or add buffer to result
            else
            {
               buffer = data;
            }
         }
      }

      // update current bytes and samples
      currentBytes += transfer->actual;
      currentSamples += buffers * (size / validChannels);

      // sort buffers by offset due to parallel processing can lead to unordered buffers
      std::sort(result.begin(), result.end(), [](const SignalBuffer &a, const SignalBuffer &b) {
         return a.offset() < b.offset();
      });

      return result;
   }

   unsigned int transpose(SignalBuffer &buffer, unsigned int filled, const unsigned char *data, unsigned int source, unsigned int limit)
   {
      unsigned int ch = buffer.stride(); // number of channels
      unsigned int chunk = (ch << 3); // chunk size in bytes for interleaved data
      unsigned int pos = source % chunk;
      unsigned int col = (pos >> 3);
      unsigned int row = (pos & 0x07) << 3;

      // transpose data from transfer buffer to interleaved buffer
      for (; buffer.remaining() && source < limit;)
      {
         // reserve full buffer for interleaved data
         float *target = !filled ? buffer.push(ch << 6) : buffer.push(0) - filled; // 64 * channels

         // transpose full block of SAMPLES[64][validChannels]
         for (unsigned int c = col; c < ch; ++c)
         {
            // transpose source block of 8 bytes
            for (unsigned int i = row, t = c + row * ch; i < 8 && source < limit; ++i, ++source)
            {
               // float data conversion table
               const float *samples = dsl_samples[data[source]];

               // each byte contains 8 samples, so we need to transpose them
               for (unsigned int r = 0; r < 8; ++r, t += ch)
               {
                  target[t] = samples[r];
               }
            }

            // next loop start from first row
            row = 0;
         }

         // next loop start from first column
         col = 0;

         // next block if not filled
         filled = 0;
      }

      return source;
   }

   bool isReady() const
   {
      return usbRead(rd_cmd_fw_version);
   }

   // void ledControl(int control)
   // {
   //    if (control == LED_OFF)
   //       usbWrite(wr_cmd_led_off);
   //    else if (control == LED_GREEN)
   //       usbWrite(wr_cmd_led_green_on);
   //    else if (control == LED_RED)
   //       usbWrite(wr_cmd_led_red_on);
   // }

   void adjustSamplerate()
   {
      sampleratesMinIndex = 0;
      sampleratesMaxIndex = 0;

      for (int i = 0; profile->dev_caps.samplerates[i]; sampleratesMaxIndex = i, i++)
      {
         if (profile->dev_caps.samplerates[i] > channel_modes[channelMode].max_samplerate)
            break;
      }

      for (int i = 0; profile->dev_caps.samplerates[i]; sampleratesMinIndex = i, i++)
      {
         if (profile->dev_caps.samplerates[i] >= channel_modes[channelMode].min_samplerate)
            break;
      }

      if (samplerate > profile->dev_caps.samplerates[sampleratesMaxIndex])
         samplerate = profile->dev_caps.samplerates[sampleratesMaxIndex];

      if (samplerate < profile->dev_caps.samplerates[sampleratesMinIndex])
         samplerate = profile->dev_caps.samplerates[sampleratesMinIndex];
   }

   unsigned int totalTransfers() const
   {
      int count;

      /* Total buffer size should be able to hold about 100ms of data. */
      if (stream)
      {
         count = ceil(static_cast<double>(totalBufferTime() * bytesPerMs()) / bufferSize());
      }
      else
      {
#ifndef _WIN32
         count = 1;
#else
         count = profile->usb_speed == LIBUSB_SPEED_SUPER ? 16 : 4;
#endif
      }

      return count > NUM_SIMUL_TRANSFERS ? NUM_SIMUL_TRANSFERS : count;
   }

   unsigned int bytesPerMs() const
   {
      return ceil(static_cast<double>(samplerate) / 1000.0 * validChannels / 8);
   }

   unsigned int singleBufferTime() const
   {
      return profile->usb_speed == LIBUSB_SPEED_SUPER ? 10 : 20;
   }

   unsigned int totalBufferTime() const
   {
      return profile->usb_speed == LIBUSB_SPEED_SUPER ? 40 : 100;
   }

   unsigned int channelDepth() const
   {
      return static_cast<int>((profile->dev_caps.hw_depth / (validChannels ? validChannels : 1)) & ~SAMPLES_ALIGN);
   }

   unsigned int headerSize() const
   {
      return profile->dev_caps.feature_caps & CAPS_FEATURE_USB30 ? DEV_KB(1) : DEV_B(512);
   }

   unsigned int bufferSize() const
   {
      // The buffer should be large enough to hold 10ms of data and a multiple of 512.
      unsigned size = stream ? singleBufferTime() * bytesPerMs() : 1024 * 1024;

      return profile->usb_speed == LIBUSB_SPEED_SUPER ? static_cast<unsigned int>((size + 1023ULL) & ~1023ULL) : static_cast<int>((size + 511ULL) & ~511ULL);
   }

   uint16_t triggerMask0(uint16_t stage, uint16_t msc, uint16_t lsc, bool qutr_mode, bool half_mode)
   {
      uint16_t mask = 0;

      for (int i = msc; i >= lsc; i--)
      {
         if (i >= NUM_TRIGGER_PROBES) continue;
         mask = (mask << 1) + ((trigger.trigger0[stage][i] == 'X') | (trigger.trigger0[stage][i] == 'C'));
      }

      return triggerMode(mask, qutr_mode, half_mode);
   }

   uint16_t triggerMask1(uint16_t stage, uint16_t msc, uint16_t lsc, bool qutr_mode, bool half_mode)
   {
      uint16_t mask = 0;

      for (int i = msc; i >= lsc; i--)
      {
         if (i >= NUM_TRIGGER_PROBES) continue;
         mask = (mask << 1) + ((trigger.trigger1[stage][i] == 'X') | (trigger.trigger1[stage][i] == 'C'));
      }

      return triggerMode(mask, qutr_mode, half_mode);
   }

   uint16_t triggerValue0(uint16_t stage, uint16_t msc, uint16_t lsc, bool qutr_mode, bool half_mode)
   {
      uint16_t value = 0;

      for (int i = msc; i >= lsc; i--)
      {
         if (i >= NUM_TRIGGER_PROBES) continue;
         value = (value << 1) + ((trigger.trigger0[stage][i] == '1') | (trigger.trigger0[stage][i] == 'R'));
      }

      return triggerMode(value, qutr_mode, half_mode);
   }

   uint16_t triggerValue1(uint16_t stage, uint16_t msc, uint16_t lsc, bool qutr_mode, bool half_mode)
   {
      uint16_t value = 0;

      for (int i = msc; i >= lsc; i--)
      {
         if (i >= NUM_TRIGGER_PROBES) continue;
         value = (value << 1) + ((trigger.trigger1[stage][i] == '1') | (trigger.trigger1[stage][i] == 'R'));
      }

      return triggerMode(value, qutr_mode, half_mode);
   }

   uint16_t triggerEdge0(uint16_t stage, uint16_t msc, uint16_t lsc, bool qutr_mode, bool half_mode)
   {
      uint16_t edge = 0;

      for (int i = msc; i >= lsc; i--)
      {
         if (i >= NUM_TRIGGER_PROBES) continue;
         edge = (edge << 1) + ((trigger.trigger0[stage][i] == 'R') | (trigger.trigger0[stage][i] == 'F') | (trigger.trigger0[stage][i] == 'C'));
      }

      return triggerMode(edge, qutr_mode, half_mode);
   }

   uint16_t triggerEdge1(uint16_t stage, uint16_t msc, uint16_t lsc, bool qutr_mode, bool half_mode)
   {
      uint16_t edge = 0;

      for (int i = msc; i >= lsc; i--)
      {
         if (i >= NUM_TRIGGER_PROBES) continue;
         edge = (edge << 1) + ((trigger.trigger1[stage][i] == 'R') | (trigger.trigger1[stage][i] == 'F') | (trigger.trigger1[stage][i] == 'C'));
      }

      return triggerMode(edge, qutr_mode, half_mode);
   }

   static uint16_t triggerMode(uint16_t value, bool qutr_mode, bool half_mode)
   {
      const uint16_t qutr_mask = (0xffff >> (NUM_TRIGGER_PROBES - NUM_TRIGGER_PROBES / 4));
      const uint16_t half_mask = (0xffff >> (NUM_TRIGGER_PROBES - NUM_TRIGGER_PROBES / 2));

      if (qutr_mode)
      {
         value = ((value & qutr_mask) << (NUM_TRIGGER_PROBES / 4 * 3));
         value += ((value & qutr_mask) << (NUM_TRIGGER_PROBES / 4 * 2));
         value += ((value & qutr_mask) << (NUM_TRIGGER_PROBES / 4 * 1));
         value += ((value & qutr_mask) << (NUM_TRIGGER_PROBES / 4 * 0));
      }
      else if (half_mode)
      {
         value = ((value & half_mask) << (NUM_TRIGGER_PROBES / 2 * 1));
         value += ((value & half_mask) << (NUM_TRIGGER_PROBES / 2 * 0));
      }

      return value;
   }

   static std::string buildName(const dsl_profile *profile)
   {
      char buffer[1024];

      snprintf(buffer, sizeof(buffer), "%s://%04x:%04x@%s %s", DEVICE_TYPE_PREFIX, profile->vid, profile->pid, profile->vendor, profile->model);

      return {buffer};
   }
};

DSLogicDevice::DSLogicDevice(const std::string &name) : impl(new Impl(name))
{
}

DSLogicDevice::~DSLogicDevice() = default;

bool DSLogicDevice::open(Mode mode)
{
   return impl->open(mode);
}

void DSLogicDevice::close()
{
   impl->close();
}

int DSLogicDevice::start(StreamHandler handler)
{
   return impl->start(handler);
}

int DSLogicDevice::stop()
{
   return impl->stop();
}

int DSLogicDevice::pause()
{
   return impl->pause();
}

int DSLogicDevice::resume()
{
   return impl->resume();
}

rt::Variant DSLogicDevice::get(int id, int channel) const
{
   return impl->get(id, channel);
}

bool DSLogicDevice::set(int id, const rt::Variant &value, int channel)
{
   return impl->set(id, value, channel);
}

bool DSLogicDevice::isOpen() const
{
   return impl->usb.isOpen();
}

bool DSLogicDevice::isEof() const
{
   return impl->deviceStatus != STATUS_READY && impl->deviceStatus != STATUS_START && impl->deviceStatus != STATUS_DATA;
}

bool DSLogicDevice::isReady() const
{
   return impl->deviceStatus >= STATUS_READY && impl->isReady();
}

bool DSLogicDevice::isPaused() const
{
   return impl->deviceStatus == STATUS_PAUSE;
}

bool DSLogicDevice::isStreaming() const
{
   return impl->deviceStatus == STATUS_START || impl->deviceStatus == STATUS_DATA;
}

long DSLogicDevice::read(SignalBuffer &buffer)
{
   return -1;
}

long DSLogicDevice::write(const SignalBuffer &buffer)
{
   return -1;
}

std::vector<std::string> DSLogicDevice::enumerate()
{
   std::vector<std::string> devices;

   for (const auto &descriptor: Usb::list())
   {
      // search for DSLogic device
      for (auto &profile: dsl_profiles)
      {
         if (!(descriptor.vid == profile.vid && descriptor.pid == profile.pid))
            continue;

         devices.push_back(Impl::buildName(&profile));

         break;
      }
   }

   return devices;
}

#define L 0
#define H 1

// bitmap values from 0 to 255 to float
const float DSLogicDevice::Impl::dsl_samples[256][8] {
   {L, L, L, L, L, L, L, L},
   {H, L, L, L, L, L, L, L},
   {L, H, L, L, L, L, L, L},
   {H, H, L, L, L, L, L, L},
   {L, L, H, L, L, L, L, L},
   {H, L, H, L, L, L, L, L},
   {L, H, H, L, L, L, L, L},
   {H, H, H, L, L, L, L, L},
   {L, L, L, H, L, L, L, L},
   {H, L, L, H, L, L, L, L},
   {L, H, L, H, L, L, L, L},
   {H, H, L, H, L, L, L, L},
   {L, L, H, H, L, L, L, L},
   {H, L, H, H, L, L, L, L},
   {L, H, H, H, L, L, L, L},
   {H, H, H, H, L, L, L, L},
   {L, L, L, L, H, L, L, L},
   {H, L, L, L, H, L, L, L},
   {L, H, L, L, H, L, L, L},
   {H, H, L, L, H, L, L, L},
   {L, L, H, L, H, L, L, L},
   {H, L, H, L, H, L, L, L},
   {L, H, H, L, H, L, L, L},
   {H, H, H, L, H, L, L, L},
   {L, L, L, H, H, L, L, L},
   {H, L, L, H, H, L, L, L},
   {L, H, L, H, H, L, L, L},
   {H, H, L, H, H, L, L, L},
   {L, L, H, H, H, L, L, L},
   {H, L, H, H, H, L, L, L},
   {L, H, H, H, H, L, L, L},
   {H, H, H, H, H, L, L, L},
   {L, L, L, L, L, H, L, L},
   {H, L, L, L, L, H, L, L},
   {L, H, L, L, L, H, L, L},
   {H, H, L, L, L, H, L, L},
   {L, L, H, L, L, H, L, L},
   {H, L, H, L, L, H, L, L},
   {L, H, H, L, L, H, L, L},
   {H, H, H, L, L, H, L, L},
   {L, L, L, H, L, H, L, L},
   {H, L, L, H, L, H, L, L},
   {L, H, L, H, L, H, L, L},
   {H, H, L, H, L, H, L, L},
   {L, L, H, H, L, H, L, L},
   {H, L, H, H, L, H, L, L},
   {L, H, H, H, L, H, L, L},
   {H, H, H, H, L, H, L, L},
   {L, L, L, L, H, H, L, L},
   {H, L, L, L, H, H, L, L},
   {L, H, L, L, H, H, L, L},
   {H, H, L, L, H, H, L, L},
   {L, L, H, L, H, H, L, L},
   {H, L, H, L, H, H, L, L},
   {L, H, H, L, H, H, L, L},
   {H, H, H, L, H, H, L, L},
   {L, L, L, H, H, H, L, L},
   {H, L, L, H, H, H, L, L},
   {L, H, L, H, H, H, L, L},
   {H, H, L, H, H, H, L, L},
   {L, L, H, H, H, H, L, L},
   {H, L, H, H, H, H, L, L},
   {L, H, H, H, H, H, L, L},
   {H, H, H, H, H, H, L, L},
   {L, L, L, L, L, L, H, L},
   {H, L, L, L, L, L, H, L},
   {L, H, L, L, L, L, H, L},
   {H, H, L, L, L, L, H, L},
   {L, L, H, L, L, L, H, L},
   {H, L, H, L, L, L, H, L},
   {L, H, H, L, L, L, H, L},
   {H, H, H, L, L, L, H, L},
   {L, L, L, H, L, L, H, L},
   {H, L, L, H, L, L, H, L},
   {L, H, L, H, L, L, H, L},
   {H, H, L, H, L, L, H, L},
   {L, L, H, H, L, L, H, L},
   {H, L, H, H, L, L, H, L},
   {L, H, H, H, L, L, H, L},
   {H, H, H, H, L, L, H, L},
   {L, L, L, L, H, L, H, L},
   {H, L, L, L, H, L, H, L},
   {L, H, L, L, H, L, H, L},
   {H, H, L, L, H, L, H, L},
   {L, L, H, L, H, L, H, L},
   {H, L, H, L, H, L, H, L},
   {L, H, H, L, H, L, H, L},
   {H, H, H, L, H, L, H, L},
   {L, L, L, H, H, L, H, L},
   {H, L, L, H, H, L, H, L},
   {L, H, L, H, H, L, H, L},
   {H, H, L, H, H, L, H, L},
   {L, L, H, H, H, L, H, L},
   {H, L, H, H, H, L, H, L},
   {L, H, H, H, H, L, H, L},
   {H, H, H, H, H, L, H, L},
   {L, L, L, L, L, H, H, L},
   {H, L, L, L, L, H, H, L},
   {L, H, L, L, L, H, H, L},
   {H, H, L, L, L, H, H, L},
   {L, L, H, L, L, H, H, L},
   {H, L, H, L, L, H, H, L},
   {L, H, H, L, L, H, H, L},
   {H, H, H, L, L, H, H, L},
   {L, L, L, H, L, H, H, L},
   {H, L, L, H, L, H, H, L},
   {L, H, L, H, L, H, H, L},
   {H, H, L, H, L, H, H, L},
   {L, L, H, H, L, H, H, L},
   {H, L, H, H, L, H, H, L},
   {L, H, H, H, L, H, H, L},
   {H, H, H, H, L, H, H, L},
   {L, L, L, L, H, H, H, L},
   {H, L, L, L, H, H, H, L},
   {L, H, L, L, H, H, H, L},
   {H, H, L, L, H, H, H, L},
   {L, L, H, L, H, H, H, L},
   {H, L, H, L, H, H, H, L},
   {L, H, H, L, H, H, H, L},
   {H, H, H, L, H, H, H, L},
   {L, L, L, H, H, H, H, L},
   {H, L, L, H, H, H, H, L},
   {L, H, L, H, H, H, H, L},
   {H, H, L, H, H, H, H, L},
   {L, L, H, H, H, H, H, L},
   {H, L, H, H, H, H, H, L},
   {L, H, H, H, H, H, H, L},
   {H, H, H, H, H, H, H, L},
   {L, L, L, L, L, L, L, H},
   {H, L, L, L, L, L, L, H},
   {L, H, L, L, L, L, L, H},
   {H, H, L, L, L, L, L, H},
   {L, L, H, L, L, L, L, H},
   {H, L, H, L, L, L, L, H},
   {L, H, H, L, L, L, L, H},
   {H, H, H, L, L, L, L, H},
   {L, L, L, H, L, L, L, H},
   {H, L, L, H, L, L, L, H},
   {L, H, L, H, L, L, L, H},
   {H, H, L, H, L, L, L, H},
   {L, L, H, H, L, L, L, H},
   {H, L, H, H, L, L, L, H},
   {L, H, H, H, L, L, L, H},
   {H, H, H, H, L, L, L, H},
   {L, L, L, L, H, L, L, H},
   {H, L, L, L, H, L, L, H},
   {L, H, L, L, H, L, L, H},
   {H, H, L, L, H, L, L, H},
   {L, L, H, L, H, L, L, H},
   {H, L, H, L, H, L, L, H},
   {L, H, H, L, H, L, L, H},
   {H, H, H, L, H, L, L, H},
   {L, L, L, H, H, L, L, H},
   {H, L, L, H, H, L, L, H},
   {L, H, L, H, H, L, L, H},
   {H, H, L, H, H, L, L, H},
   {L, L, H, H, H, L, L, H},
   {H, L, H, H, H, L, L, H},
   {L, H, H, H, H, L, L, H},
   {H, H, H, H, H, L, L, H},
   {L, L, L, L, L, H, L, H},
   {H, L, L, L, L, H, L, H},
   {L, H, L, L, L, H, L, H},
   {H, H, L, L, L, H, L, H},
   {L, L, H, L, L, H, L, H},
   {H, L, H, L, L, H, L, H},
   {L, H, H, L, L, H, L, H},
   {H, H, H, L, L, H, L, H},
   {L, L, L, H, L, H, L, H},
   {H, L, L, H, L, H, L, H},
   {L, H, L, H, L, H, L, H},
   {H, H, L, H, L, H, L, H},
   {L, L, H, H, L, H, L, H},
   {H, L, H, H, L, H, L, H},
   {L, H, H, H, L, H, L, H},
   {H, H, H, H, L, H, L, H},
   {L, L, L, L, H, H, L, H},
   {H, L, L, L, H, H, L, H},
   {L, H, L, L, H, H, L, H},
   {H, H, L, L, H, H, L, H},
   {L, L, H, L, H, H, L, H},
   {H, L, H, L, H, H, L, H},
   {L, H, H, L, H, H, L, H},
   {H, H, H, L, H, H, L, H},
   {L, L, L, H, H, H, L, H},
   {H, L, L, H, H, H, L, H},
   {L, H, L, H, H, H, L, H},
   {H, H, L, H, H, H, L, H},
   {L, L, H, H, H, H, L, H},
   {H, L, H, H, H, H, L, H},
   {L, H, H, H, H, H, L, H},
   {H, H, H, H, H, H, L, H},
   {L, L, L, L, L, L, H, H},
   {H, L, L, L, L, L, H, H},
   {L, H, L, L, L, L, H, H},
   {H, H, L, L, L, L, H, H},
   {L, L, H, L, L, L, H, H},
   {H, L, H, L, L, L, H, H},
   {L, H, H, L, L, L, H, H},
   {H, H, H, L, L, L, H, H},
   {L, L, L, H, L, L, H, H},
   {H, L, L, H, L, L, H, H},
   {L, H, L, H, L, L, H, H},
   {H, H, L, H, L, L, H, H},
   {L, L, H, H, L, L, H, H},
   {H, L, H, H, L, L, H, H},
   {L, H, H, H, L, L, H, H},
   {H, H, H, H, L, L, H, H},
   {L, L, L, L, H, L, H, H},
   {H, L, L, L, H, L, H, H},
   {L, H, L, L, H, L, H, H},
   {H, H, L, L, H, L, H, H},
   {L, L, H, L, H, L, H, H},
   {H, L, H, L, H, L, H, H},
   {L, H, H, L, H, L, H, H},
   {H, H, H, L, H, L, H, H},
   {L, L, L, H, H, L, H, H},
   {H, L, L, H, H, L, H, H},
   {L, H, L, H, H, L, H, H},
   {H, H, L, H, H, L, H, H},
   {L, L, H, H, H, L, H, H},
   {H, L, H, H, H, L, H, H},
   {L, H, H, H, H, L, H, H},
   {H, H, H, H, H, L, H, H},
   {L, L, L, L, L, H, H, H},
   {H, L, L, L, L, H, H, H},
   {L, H, L, L, L, H, H, H},
   {H, H, L, L, L, H, H, H},
   {L, L, H, L, L, H, H, H},
   {H, L, H, L, L, H, H, H},
   {L, H, H, L, L, H, H, H},
   {H, H, H, L, L, H, H, H},
   {L, L, L, H, L, H, H, H},
   {H, L, L, H, L, H, H, H},
   {L, H, L, H, L, H, H, H},
   {H, H, L, H, L, H, H, H},
   {L, L, H, H, L, H, H, H},
   {H, L, H, H, L, H, H, H},
   {L, H, H, H, L, H, H, H},
   {H, H, H, H, L, H, H, H},
   {L, L, L, L, H, H, H, H},
   {H, L, L, L, H, H, H, H},
   {L, H, L, L, H, H, H, H},
   {H, H, L, L, H, H, H, H},
   {L, L, H, L, H, H, H, H},
   {H, L, H, L, H, H, H, H},
   {L, H, H, L, H, H, H, H},
   {H, H, H, L, H, H, H, H},
   {L, L, L, H, H, H, H, H},
   {H, L, L, H, H, H, H, H},
   {L, H, L, H, H, H, H, H},
   {H, H, L, H, H, H, H, H},
   {L, L, H, H, H, H, H, H},
   {H, L, H, H, H, H, H, H},
   {L, H, H, H, H, H, H, H},
   {H, H, H, H, H, H, H, H},
};

}
