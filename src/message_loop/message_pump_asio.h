/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include <asio/io_context.hpp>
#include "message_loop/message_pump.h"

class MessagePumpForAsio final : public MessagePump
{
public:
    MessagePumpForAsio() = default;
    ~MessagePumpForAsio() final = default;

    // MessagePump methods:
    void run(Delegate* delegate) final;
    void quit() final;
    void scheduleWork() final;
    void scheduleDelayedWork(const TimePoint& delayed_work_time) final;

    asio::io_context& ioContext() { return io_context_; }

private:
    bool keep_running_ = true;
    asio::io_context io_context_;
    TimePoint delayed_work_time_;
};