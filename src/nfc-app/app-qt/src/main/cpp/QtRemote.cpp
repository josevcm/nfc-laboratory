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

#include <QDebug>
#include <QSettings>

#ifdef ENABLE_GRPC_API
#include <grpcpp/grpcpp.h>
#include "remote/GrpcControl.h"
#endif

#include <events/StreamFrameEvent.h>
#include <events/SystemShutdownEvent.h>
#include <events/SystemStartupEvent.h>

#include "QtRemote.h"

struct QtRemote::Impl
{
   // configuration
   QSettings settings;

#ifdef ENABLE_GRPC_API
   std::unique_ptr<GrpcControl> control;
   std::unique_ptr<grpc::Server> server;
#endif

   Impl()
   {
#ifdef ENABLE_GRPC_API
      control = std::make_unique<GrpcControl>();
#endif
   }

   ~Impl()
   {
      stop();
   }

   void start()
   {
#ifdef ENABLE_GRPC_API
      // start gRPC server if configured
      const int port = settings.value("grpc/port", 0).toInt();

      if (port <= 0)
      {
         qInfo() << "gRPC server disabled";
         return;
      }

      grpc::ServerBuilder builder;
      builder.AddListeningPort("0.0.0.0:" + std::to_string(port), grpc::InsecureServerCredentials());
      builder.RegisterService(control.get());

      server = builder.BuildAndStart();

      if (server)
         qInfo() << "gRPC server listening on port" << port;
      else
         qWarning() << "gRPC server failed to start on port" << port;
#endif
   }

   void stop()
   {
#ifdef ENABLE_GRPC_API
      if (server)
      {
         qInfo() << "stopping gRPC server";
         server->Shutdown();
         server.reset();
      }
#endif
   }

   void frame(StreamFrameEvent *event)
   {
#ifdef ENABLE_GRPC_API
      if (!control)
         return;

      const lab::RawFrame &f = event->frame();

      if (!f.isValid())
         return;

      EventNotification notification;
      notification.set_timestamp(static_cast<uint64_t>(f.dateTime() * 1000.0));

      FrameEvent *fe = notification.mutable_frame();

      switch (f.techType())
      {
         case lab::NfcAnyTech: fe->set_tech(FrameTech_NfcAnyTech);
            break;
         case lab::NfcATech: fe->set_tech(FrameTech_NfcATech);
            break;
         case lab::NfcBTech: fe->set_tech(FrameTech_NfcBTech);
            break;
         case lab::NfcFTech: fe->set_tech(FrameTech_NfcFTech);
            break;
         case lab::NfcVTech: fe->set_tech(FrameTech_NfcVTech);
            break;
         case lab::IsoAnyTech: fe->set_tech(FrameTech_IsoAnyTech);
            break;
         case lab::Iso7816Tech: fe->set_tech(FrameTech_Iso7816Tech);
            break;
         default: fe->set_tech(FrameTech_NoneTech);
            break;
      }

      switch (f.frameType())
      {
         case lab::NfcCarrierOff: fe->set_type(FrameType_NfcCarrierOff);
            break;
         case lab::NfcCarrierOn: fe->set_type(FrameType_NfcCarrierOn);
            break;
         case lab::NfcPollFrame: fe->set_type(FrameType_NfcPollFrame);
            break;
         case lab::NfcListenFrame: fe->set_type(FrameType_NfcListenFrame);
            break;
         case lab::IsoVccLow: fe->set_type(FrameType_IsoVccLow);
            break;
         case lab::IsoVccHigh: fe->set_type(FrameType_IsoVccHigh);
            break;
         case lab::IsoRstLow: fe->set_type(FrameType_IsoRstLow);
            break;
         case lab::IsoRstHigh: fe->set_type(FrameType_IsoRstHigh);
            break;
         case lab::IsoATRFrame: fe->set_type(FrameType_IsoATRFrame);
            break;
         case lab::IsoRequestFrame: fe->set_type(FrameType_IsoRequestFrame);
            break;
         case lab::IsoResponseFrame: fe->set_type(FrameType_IsoResponseFrame);
            break;
         case lab::IsoExchangeFrame: fe->set_type(FrameType_IsoExchangeFrame);
            break;
         default: fe->set_type(FrameType_NoneType);
            break;
      }

      fe->set_flags(f.frameFlags());
      fe->set_time_start(static_cast<uint64_t>(f.timeStart() * 1e6));
      fe->set_time_end(static_cast<uint64_t>(f.timeEnd() * 1e6));
      fe->set_rate(f.frameRate());

      if (f.remaining() > 0)
         fe->set_data(reinterpret_cast<const char *>(f.ptr()), f.remaining());

      control->publish(notification);
#endif
   }
};

QtRemote::QtRemote() : impl(new Impl())
{
}

void QtRemote::handleEvent(QEvent *event) const
{
   if (event->type() == StreamFrameEvent::Type)
      impl->frame(dynamic_cast<StreamFrameEvent *>(event));
   else if (event->type() == SystemStartupEvent::Type)
      impl->start();
   else if (event->type() == SystemShutdownEvent::Type)
      impl->stop();
}
