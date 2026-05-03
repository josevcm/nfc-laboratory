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

#include <atomic>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <grpcpp/grpcpp.h>

#include "nfc_control.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Pointer to the active streaming context so the signal handler can cancel it.
static std::atomic<ClientContext *> gActiveContext{nullptr};

static void signalHandler(int)
{
   if (auto *ctx = gActiveContext.load())
      ctx->TryCancel();
}

static const char *stateToString(State state)
{
   switch (state)
   {
      case State::IDLE:    return "idle";
      case State::RUNNING: return "running";
      case State::PAUSED:  return "paused";
      default:             return "unknown";
   }
}

static const char *techToString(FrameTech tech)
{
   switch (tech)
   {
      case FrameTech_NfcAnyTech:  return "NFC";
      case FrameTech_NfcATech:    return "NFC-A";
      case FrameTech_NfcBTech:    return "NFC-B";
      case FrameTech_NfcFTech:    return "NFC-F";
      case FrameTech_NfcVTech:    return "NFC-V";
      case FrameTech_IsoAnyTech:  return "ISO";
      case FrameTech_Iso7816Tech: return "ISO-7816";
      default:                    return "none";
   }
}

static const char *frameTypeToString(FrameType type)
{
   switch (type)
   {
      case FrameType_NfcCarrierOff:    return "carrier-off";
      case FrameType_NfcCarrierOn:     return "carrier-on";
      case FrameType_NfcPollFrame:     return "poll";
      case FrameType_NfcListenFrame:   return "listen";
      case FrameType_IsoVccLow:        return "vcc-low";
      case FrameType_IsoVccHigh:       return "vcc-high";
      case FrameType_IsoRstLow:        return "rst-low";
      case FrameType_IsoRstHigh:       return "rst-high";
      case FrameType_IsoATRFrame:      return "atr";
      case FrameType_IsoRequestFrame:  return "request";
      case FrameType_IsoResponseFrame: return "response";
      case FrameType_IsoExchangeFrame: return "exchange";
      default:                         return "none";
   }
}

static std::string flagsToString(uint32_t flags)
{
   if (flags == 0)
      return "none";

   std::string result;

   auto append = [&](const char *name) {
      if (!result.empty()) result += '|';
      result += name;
   };

   if (flags & Flags_Frame_ShortFrame)       append("short");
   if (flags & Flags_Frame_Encrypted)        append("encrypted");
   if (flags & Flags_Frame_Truncated)        append("truncated");
   if (flags & Flags_Frame_ParityError)      append("parity-err");
   if (flags & Flags_Frame_CrcError)         append("crc-err");
   if (flags & Flags_Frame_SyncError)        append("sync-err");
   if (flags & Flags_Protocol_RequestFrame)  append("req");
   if (flags & Flags_Protocol_ResponseFrame) append("resp");
   if (flags & Flags_Protocol_SenseFrame)    append("sense");
   if (flags & Flags_Protocol_SelectionFrame)append("select");
   if (flags & Flags_Protocol_ApplicationFrame) append("app");
   if (flags & Flags_Protocol_AuthFrame)     append("auth");

   return result;
}

static std::string toHex(const std::string &bytes)
{
   std::ostringstream oss;
   oss << std::uppercase << std::hex << std::setfill('0');
   for (size_t i = 0; i < bytes.size(); ++i)
   {
      if (i) oss << ' ';
      oss << std::setw(2) << static_cast<int>(static_cast<unsigned char>(bytes[i]));
   }
   return oss.str();
}

static void printNotification(const EventNotification &n)
{
   std::cout << "[event] ts=" << n.timestamp();

   switch (n.payload_case())
   {
      case EventNotification::kState:
         std::cout << " type=state"
                   << " state="     << stateToString(n.state().state())
                   << " message=\"" << n.state().message() << "\"";
         break;

      case EventNotification::kFrame:
      {
         const FrameEvent &fe = n.frame();
         std::cout << " type=frame"
                   << " tech="  << techToString(fe.tech())
                   << " kind="  << frameTypeToString(fe.type())
                   << " rate="  << fe.rate()
                   << std::fixed << std::setprecision(6)
                   << " t0="    << fe.time_start() / 1e6 << "s"
                   << " t1="    << fe.time_end()   / 1e6 << "s"
                   << " flags=" << flagsToString(fe.flags())
                   << " ["      << fe.data().size() << "] "
                   << toHex(fe.data());
         break;
      }

      case EventNotification::kDevice:
         std::cout << " type=device"
                   << " name="   << n.device().name()
                   << " kind="   << n.device().type()
                   << " status=" << n.device().status();
         break;

      default:
         std::cout << " type=unknown";
   }

   std::cout << '\n';
}

class RemoteControlClient
{
public:

   explicit RemoteControlClient(std::shared_ptr<Channel> channel)
      : stub(LabRemote::NewStub(channel))
   {
   }

   bool execute(const std::string &command)
   {
      ControlRequest request;
      ControlResponse response;
      ClientContext context;
      Status status;

      if (command == "start")
         status = stub->Start(&context, request, &response);
      else if (command == "stop")
         status = stub->Stop(&context, request, &response);
      else if (command == "pause")
         status = stub->Pause(&context, request, &response);
      else if (command == "resume")
         status = stub->Resume(&context, request, &response);
      else
      {
         std::cerr << "[test-rpc-client] unknown command: " << command << std::endl;
         return false;
      }

      if (!status.ok())
      {
         std::cerr << "[test-rpc-client] " << command << " failed: " << status.error_message() << std::endl;
         return false;
      }

      std::cout << "[test-rpc-client] " << command << " response:"
                << " success=" << (response.success() ? "true" : "false")
                << " message=\"" << response.message() << "\""
                << " state=" << stateToString(response.state())
                << std::endl;

      return response.success();
   }

   // filter: comma-separated types (state,frame,device) or empty for all
   bool subscribe(const std::string &filter)
   {
      SubscribeRequest request;
      ClientContext context;

      if (!filter.empty())
      {
         std::istringstream ss(filter);
         std::string token;

         while (std::getline(ss, token, ','))
         {
            if (token == "state")       request.add_filter(EventType::STATE);
            else if (token == "frame")  request.add_filter(EventType::FRAME);
            else if (token == "device") request.add_filter(EventType::DEVICE);
            else
            {
               std::cerr << "[test-rpc-client] unknown filter type: " << token << std::endl;
               return false;
            }
         }
      }

      gActiveContext.store(&context);

      const auto reader = stub->Subscribe(&context, request);

      std::cout << "[test-rpc-client] subscribed"
                << (filter.empty() ? "" : " filter=" + filter)
                << ", waiting for events (Ctrl+C to stop)..."
                << std::endl;

      EventNotification notification;

      while (reader->Read(&notification))
         printNotification(notification);

      gActiveContext.store(nullptr);

      const Status status = reader->Finish();

      if (status.error_code() == grpc::StatusCode::CANCELLED)
      {
         std::cout << "[test-rpc-client] subscription cancelled" << std::endl;
         return true;
      }

      if (!status.ok())
      {
         std::cerr << "[test-rpc-client] subscribe ended with error: " << status.error_message() << std::endl;
         return false;
      }

      std::cout << "[test-rpc-client] subscription ended" << std::endl;
      return true;
   }

private:

   std::unique_ptr<LabRemote::Stub> stub;
};

void printUsage(const char *name)
{
   std::cout << "Usage: " << name << " [host:port] <command>" << std::endl;
   std::cout << "  host:port              server address (default: localhost:50051)" << std::endl;
   std::cout << "  start                  start capture" << std::endl;
   std::cout << "  stop                   stop capture" << std::endl;
   std::cout << "  pause                  pause capture" << std::endl;
   std::cout << "  resume                 resume capture" << std::endl;
   std::cout << "  subscribe              subscribe to all events (Ctrl+C to stop)" << std::endl;
   std::cout << "  subscribe:state        subscribe to state change events only" << std::endl;
   std::cout << "  subscribe:frame        subscribe to NFC frame events only" << std::endl;
   std::cout << "  subscribe:device       subscribe to device status events only" << std::endl;
   std::cout << "  subscribe:state,frame  subscribe to multiple event types" << std::endl;
}

int main(int argc, char *argv[])
{
   std::cout << "***********************************************************************" << std::endl;
   std::cout << "NFC laboratory, 2024 Jose Vicente Campos Martinez - <josevcm@gmail.com>" << std::endl;
   std::cout << "***********************************************************************" << std::endl;

   std::signal(SIGINT,  signalHandler);
   std::signal(SIGTERM, signalHandler);

   std::string address = "localhost:50051";
   std::string command;

   for (int i = 1; i < argc; ++i)
   {
      std::string arg(argv[i]);

      if (arg == "--help" || arg == "-h")
      {
         printUsage(argv[0]);
         return 0;
      }
      else if (arg == "start" || arg == "stop" || arg == "pause" || arg == "resume"
               || arg.rfind("subscribe", 0) == 0)
      {
         command = arg;
      }
      else
      {
         address = arg;
      }
   }

   if (command.empty())
   {
      printUsage(argv[0]);
      return 1;
   }

   RemoteControlClient client(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

   std::cout << "[test-rpc-client] connecting to " << address << std::endl;

   if (command.rfind("subscribe", 0) == 0)
   {
      // extract optional filter after "subscribe:" (9 chars + 1 colon = 10)
      const std::string filter = command.size() > 10 ? command.substr(10) : "";
      return client.subscribe(filter) ? 0 : 1;
   }

   return client.execute(command) ? 0 : 1;
}