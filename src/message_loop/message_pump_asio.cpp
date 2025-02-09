/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "message_loop/message_pump_asio.h"
#include "common/logger.h"
#include <asio/post.hpp>

void MessagePumpForAsio::run(Delegate* delegate)
{
    assert(keep_running_);

    asio::executor_work_guard work_guard = asio::make_work_guard(io_context_);

    for (;;)
    {
        bool did_work = delegate->doWork();
        if (!keep_running_)
            break;

        did_work |= delegate->doDelayedWork(&delayed_work_time_);
        if (!keep_running_)
            break;

        io_context_.restart();
        did_work |= io_context_.poll() != 0;
        if (!keep_running_)
            break;

        if (did_work)
            continue;

        if (delayed_work_time_ == TimePoint())
        {
            io_context_.restart();
            io_context_.run_one();
        }
        else
        {
            Milliseconds delay = std::chrono::duration_cast<Milliseconds>(
                delayed_work_time_ - Clock::now());

            if (delay > Milliseconds::zero())
            {
                io_context_.restart();
                io_context_.run_one_for(delay);
            }
            else
            {
                delayed_work_time_ = TimePoint();
            }
        }
    }

    keep_running_ = true;
}

//--------------------------------------------------------------------------------------------------
void MessagePumpForAsio::quit()
{
    keep_running_ = false;
}

//--------------------------------------------------------------------------------------------------
void MessagePumpForAsio::scheduleWork()
{
    // Since this can be called on any thread, we need to ensure that our run() loop wakes up.
    asio::post(io_context_, [] {});
}

//--------------------------------------------------------------------------------------------------
void MessagePumpForAsio::scheduleDelayedWork(const TimePoint& delayed_work_time)
{
    delayed_work_time_ = delayed_work_time;
}
