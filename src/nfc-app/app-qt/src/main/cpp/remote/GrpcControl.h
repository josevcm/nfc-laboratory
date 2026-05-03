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

#ifndef APP_GRPCCONTROL_H
#define APP_GRPCCONTROL_H

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "nfc_control.grpc.pb.h"

class QObject;

class GrpcControl : public LabRemote::Service
{
   private:

      struct Subscription
      {
         std::mutex mutex;
         grpc::ServerWriter<EventNotification> *writer;
         std::condition_variable cv;
         std::deque<EventNotification> queue;
      };

   public:

      grpc::Status Start(grpc::ServerContext *ctx, const ControlRequest *req, ControlResponse *resp) override;

      grpc::Status Stop(grpc::ServerContext *ctx, const ControlRequest *req, ControlResponse *resp) override;

      grpc::Status Pause(grpc::ServerContext *ctx, const ControlRequest *req, ControlResponse *resp) override;

      grpc::Status Resume(grpc::ServerContext *ctx, const ControlRequest *req, ControlResponse *resp) override;

      grpc::Status Subscribe(grpc::ServerContext *ctx, const SubscribeRequest *req, grpc::ServerWriter<EventNotification> *writer) override;

      void publish(const EventNotification &notification);

   private:

      static grpc::Status execute(int command, ControlResponse *resp);

      std::mutex subscribersMutex;
      std::vector<std::shared_ptr<Subscription>> subscribers;
};

#endif //APP_GRPCCONTROL_H
