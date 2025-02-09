/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "message_loop/message_pump_ui.h"

MessagePumpForUI::~MessagePumpForUI() {

}

void MessagePumpForUI::run(Delegate* delegate)
{
    for (;;)
    {
        last_start_frame_time_ = Clock::now();

        bool did_work = delegate->doWork();
        if (!keep_running_)
            break;

        bool running = dispatcher_->draw();
        if (!keep_running_ || !running)
            break;

        did_work |= delegate->doDelayedWork(&delayed_work_time_);
        if (!keep_running_)
            break;

        if (did_work)
            continue;

        TimePoint target_delayed_time = last_start_frame_time_ + Milliseconds(1000 / target_fps_);
        if (delayed_work_time_ == TimePoint() || delayed_work_time_ > target_delayed_time ) {
            delayed_work_time_ = target_delayed_time;
            auto delay = std::chrono::duration_cast<Milliseconds>(
                delayed_work_time_ - Clock::now());

            auto delay_count = delay.count();
            auto delay_seconds = double (delay_count) / 1000.0;
            if (delayed_work_time_ != TimePoint())
            {
                dispatcher_->waitEvents(delay_seconds);
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_count));
            }
        }
    }

    keep_running_ = true;
}

//--------------------------------------------------------------------------------------------------
void MessagePumpForUI::quit()
{
    keep_running_ = false;
}

//--------------------------------------------------------------------------------------------------
void MessagePumpForUI::scheduleWork()
{
    if(dispatcher_)
        dispatcher_->wakeup();
}

void MessagePumpForUI::scheduleDelayedWork(const TimePoint& delayed_work_time)
{
    delayed_work_time_ = delayed_work_time;
}

void MessagePumpForUI::runWithDispatcher(Delegate* delegate, MessagePumpDispatcher* dispatcher)
{
    dispatcher_ = dispatcher;
    run(delegate);
}
