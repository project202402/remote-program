/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task_runner.h"
#include "common/macros_magic.h"

#include <shared_mutex>
#include <thread>
#include <memory>

class MessageLoopTaskRunner;
class MessagePumpForAsio;
class MessageLoop;
class MessageLoopTaskRunner final : public TaskRunner
{
public:
    static std::shared_ptr<TaskRunner> current();

    // TaskRunner implementation.
    bool belongsToCurrentThread() const final;
    void postTask(Callback callback) final;
    void postDelayedTask(Callback callback, const Milliseconds& delay) final;
    void postQuit() final;

private:
    friend class MessageLoop;

    explicit MessageLoopTaskRunner(MessageLoop* loop);

    // Called directly by MessageLoop::~MessageLoop.
    void willDestroyCurrentMessageLoop();

    MessageLoop* loop_;
    mutable std::shared_mutex loop_lock_;
    std::thread::id thread_id_;

    DISALLOW_COPY_AND_ASSIGN(MessageLoopTaskRunner);
};
