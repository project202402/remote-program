/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include "message_loop/message_loop.h"

#include <atomic>
#include <condition_variable>
#include <thread>


class Thread
{
public:
    Thread() = default;
    ~Thread();

    class Delegate
    {
    public:
        virtual ~Delegate() = default;
        virtual void onBeforeThreadRunning() {}
        virtual void onThreadRunning(MessageLoop* message_loop);
        virtual void onAfterThreadRunning() {}
    };

    void start(MessageLoop::Type message_loop_type, Delegate* delegate = nullptr);

    void stopSoon();

    void stop();

    void join();

    bool isRunning() const { return running_; }
    MessageLoop* messageLoop() const { return message_loop_; }

    std::shared_ptr<TaskRunner> taskRunner() const
    {
        return message_loop_ ? message_loop_->taskRunner() : nullptr;
    }

private:
    void threadMain(MessageLoop::Type message_loop_type);

    Delegate* delegate_ = nullptr;

    enum class State { STARTING, STARTED, STOPPING, STOPPED };

    std::atomic<State> state_ = State::STOPPED;

    std::thread thread_;

    bool running_ = false;
    std::mutex running_lock_;
    std::condition_variable running_event_;

    MessageLoop* message_loop_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(Thread);
};
