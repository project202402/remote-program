/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "message_loop/pending_task.h"

//--------------------------------------------------------------------------------------------------
PendingTask::PendingTask(
    Callback&& callback, TimePoint delayed_run_time, int sequence_num)
    : callback(std::move(callback)),
    sequence_num(sequence_num),
    delayed_run_time(delayed_run_time)
{
}

//--------------------------------------------------------------------------------------------------
bool PendingTask::operator<(const PendingTask& other) const
{
    if (delayed_run_time < other.delayed_run_time)
        return false;

    if (delayed_run_time > other.delayed_run_time)
        return true;

    return (sequence_num - other.sequence_num) > 0;
}