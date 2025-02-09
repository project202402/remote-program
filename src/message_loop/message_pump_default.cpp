/*
* Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "message_loop/message_pump_default.h"
#include "common/logger.h"
#include <mutex>

//--------------------------------------------------------------------------------------------------
void MessagePumpDefault::run(Delegate* delegate)
{
    assert(keep_running_);

    for (;;)
    {
        bool did_work = delegate->doWork();
        if (!keep_running_)
            break;

        did_work |= delegate->doDelayedWork(&delayed_work_time_);
        if (!keep_running_)
            break;

        if (did_work)
            continue;

        if (delayed_work_time_ == TimePoint())
        {
            std::unique_lock lock(have_work_lock_);

            while (!have_work_)
                event_.wait(lock);

            have_work_ = false;
        }
        else
        {
            auto delay = std::chrono::duration_cast<Milliseconds>(
                delayed_work_time_ - Clock::now());

            if (delay > Milliseconds::zero())
            {
                std::unique_lock lock(have_work_lock_);

                do
                {
                    if (event_.wait_for(lock, delay) == std::cv_status::timeout)
                        break;

                    if (have_work_)
                        break;

                    delay = std::chrono::duration_cast<Milliseconds>(
                        delayed_work_time_ - Clock::now());
                }
                while (delay > Milliseconds::zero());

                have_work_ = false;
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
void MessagePumpDefault::quit()
{
    keep_running_ = false;
}

//--------------------------------------------------------------------------------------------------
void MessagePumpDefault::scheduleWork()
{
    {
        std::scoped_lock lock(have_work_lock_);
        have_work_ = true;
    }

    event_.notify_one();
}

//--------------------------------------------------------------------------------------------------
void MessagePumpDefault::scheduleDelayedWork(const TimePoint& delayed_work_time)
{
    delayed_work_time_ = delayed_work_time;
}
