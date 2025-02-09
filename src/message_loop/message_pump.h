/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include <chrono>

class MessagePump
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Milliseconds = std::chrono::milliseconds;

    class Delegate
    {
    public:
        virtual ~Delegate() = default;
        virtual bool doWork() = 0;
        virtual bool doDelayedWork(TimePoint* next_delayed_work_time) = 0;
    };

    virtual ~MessagePump() = default;
    virtual void run(Delegate* delegate) = 0;
    virtual void quit() = 0;
    virtual void scheduleWork() = 0;
    virtual void scheduleDelayedWork(const TimePoint& delayed_work_time) = 0;
};