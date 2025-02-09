/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include "message_loop/message_pump.h"
#include "message_pump_dispatcher.h"
#include <mutex>

class MessagePumpForUI final : public MessagePump
{
public:
    MessagePumpForUI() = default;
    ~MessagePumpForUI() final;

    // MessagePump methods:
    void run(Delegate* delegate) final;
    void quit() final;
    void scheduleWork() final;
    void scheduleDelayedWork(const TimePoint& delayed_work_time) final;
    void runWithDispatcher(Delegate* delegate, MessagePumpDispatcher* dispatcher);

private:

    MessagePumpDispatcher* dispatcher_;
    bool keep_running_ = true;
    TimePoint delayed_work_time_;

    int target_fps_ = 40;
    TimePoint last_start_frame_time_;
};