/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "net/tcp_channel.h"
#include "common/task_runner.h"

#include <memory>

class Session : public TcpChannel::Listener
{
 public:
    class Listener
    {
     public:
        virtual ~Listener() = default;

        virtual void onSessionStarted(int channel_id) = 0;
        virtual void onSessionMessageReceived(int channel_id, const ByteArray& buffer) = 0;
        virtual void onSessionMessageWritten(int channel_id, size_t pending) = 0;
        virtual void onSessionStop(int channel_id) = 0;
    };

    Session(int channel_id, std::unique_ptr<TcpChannel> channel, std::shared_ptr<TaskRunner> io_task_runner);
    ~Session() override;

    void start();
    void stop();

    int  id() const;
    void setListener(Listener* listener);
    void send(ByteArray&& buffer);

 private:
    void onTcpConnected() override;
    void onTcpDisconnected(NetworkChannel::ErrorCode error_code) override;
    void onTcpMessageReceived(const ByteArray& buffer) override;
    void onTcpMessageWritten(ByteArray&& buffer, size_t pending) override;

    int channel_id_;
    Listener* listener_ = nullptr;
    std::unique_ptr<TcpChannel> channel_;
    std::shared_ptr<TaskRunner> task_runner_;
};
