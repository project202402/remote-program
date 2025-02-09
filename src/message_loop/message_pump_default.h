/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "message_loop/message_pump.h"
#include "common//macros_magic.h"

#include <condition_variable>
#include <mutex>

class MessagePumpDefault final : public MessagePump
{
public:
    MessagePumpDefault() = default;
    ~MessagePumpDefault() final = default;

    // MessagePump methods:
    void run(Delegate* delegate) final;
    void quit() final;
    void scheduleWork() final;
    void scheduleDelayedWork(const TimePoint& delayed_work_time) final;

private:

    bool keep_running_ = true;

    std::condition_variable event_;

    bool have_work_ = false;
    std::mutex have_work_lock_;

    TimePoint delayed_work_time_;

    DISALLOW_COPY_AND_ASSIGN(MessagePumpDefault);
};
