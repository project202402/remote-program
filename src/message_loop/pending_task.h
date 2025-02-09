/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include <chrono>
#include <functional>
#include <queue>

class PendingTask
{
public:
    using Callback = std::function<void()>;
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    PendingTask(Callback&& callback, TimePoint delayed_run_time, int sequence_num = 0);
    ~PendingTask() = default;
    bool operator<(const PendingTask& other) const;
    Callback callback;
    int sequence_num;
    TimePoint delayed_run_time;
};

class TaskQueue : public std::queue<PendingTask>
{
public:
    void Swap(TaskQueue* queue)
    {
        c.swap(queue->c); // Calls std::deque::swap.
    }
};

using DelayedTaskQueue = std::priority_queue<PendingTask>;

