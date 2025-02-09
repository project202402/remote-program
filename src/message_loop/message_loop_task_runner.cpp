/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "message_loop/message_loop.h"
#include "message_loop/message_loop_task_runner.h"

//--------------------------------------------------------------------------------------------------
std::shared_ptr<TaskRunner> MessageLoopTaskRunner::current()
{
    MessageLoop* current = MessageLoop::current();

    if (!current)
        return nullptr;

    return current->taskRunner();
}

//--------------------------------------------------------------------------------------------------
bool MessageLoopTaskRunner::belongsToCurrentThread() const
{
    return thread_id_ == std::this_thread::get_id();
}

//--------------------------------------------------------------------------------------------------
void MessageLoopTaskRunner::postTask(Callback callback)
{
    std::shared_lock lock(loop_lock_);

    if (loop_)
        loop_->postTask(std::move(callback));
}

//--------------------------------------------------------------------------------------------------
void MessageLoopTaskRunner::postDelayedTask(Callback callback, const Milliseconds& delay)
{
    std::shared_lock lock(loop_lock_);

    if (loop_)
        loop_->postDelayedTask(std::move(callback), delay);
}

//--------------------------------------------------------------------------------------------------
void MessageLoopTaskRunner::postQuit()
{
    std::shared_lock lock(loop_lock_);

    if (loop_)
        loop_->postTask(loop_->quitClosure());
}

//--------------------------------------------------------------------------------------------------
MessageLoopTaskRunner::MessageLoopTaskRunner(MessageLoop* loop)
    : loop_(loop),
      thread_id_(std::this_thread::get_id())
{
    // Nothing
}

//--------------------------------------------------------------------------------------------------
// Called directly by MessageLoop::~MessageLoop.
void MessageLoopTaskRunner::willDestroyCurrentMessageLoop()
{
    std::unique_lock lock(loop_lock_);
    loop_ = nullptr;
}

