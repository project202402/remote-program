/*
* Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "common/thread.h"
#include "common/logger.h"

//--------------------------------------------------------------------------------------------------
Thread::~Thread()
{
    stop();
}

//--------------------------------------------------------------------------------------------------
void Thread::start(MessageLoop::Type message_loop_type, Delegate* delegate)
{
    assert(!message_loop_);

    delegate_ = delegate;
    state_ = State::STARTING;

    thread_ = std::thread(&Thread::threadMain, this, message_loop_type);

    std::unique_lock lock(running_lock_);
    while (!running_)
        running_event_.wait(lock);

    state_ = State::STARTED;

    assert(message_loop_);
}

//--------------------------------------------------------------------------------------------------
void Thread::stopSoon()
{
    if (state_ == State::STOPPING || !message_loop_)
        return;

    state_ = State::STOPPING;

    message_loop_->postTask(message_loop_->quitClosure());
}

//--------------------------------------------------------------------------------------------------
void Thread::stop()
{
    if (state_ == State::STOPPED)
        return;

    stopSoon();

    join();

    assert(!message_loop_);

    delegate_ = nullptr;
}

//--------------------------------------------------------------------------------------------------
void Thread::join()
{
    if (state_ == State::STOPPED)
        return;

    if (thread_.joinable())
        thread_.join();

    state_ = State::STOPPED;
}

//--------------------------------------------------------------------------------------------------
void Thread::Delegate::onThreadRunning(MessageLoop* message_loop)
{
    message_loop->run();
}

//--------------------------------------------------------------------------------------------------
void Thread::threadMain(MessageLoop::Type message_loop_type)
{
    MessageLoop message_loop(message_loop_type);

    message_loop_ = &message_loop;

    if (delegate_)
        delegate_->onBeforeThreadRunning();

    {
        std::unique_lock lock(running_lock_);
        running_ = true;
    }

    running_event_.notify_one();

    if (delegate_)
    {
        delegate_->onThreadRunning(message_loop_);
    }
    else
    {
        message_loop_->run();
    }

    {
        std::unique_lock lock(running_lock_);
        running_ = false;
    }

    // Let the thread do extra cleanup.
    if (delegate_)
        delegate_->onAfterThreadRunning();

    // We can't receive messages anymore.
    message_loop_ = nullptr;
}