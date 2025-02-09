/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include "message_loop/pending_task.h"
#include "message_loop/message_pump.h"
#include "message_loop/message_pump_asio.h"
#include "message_loop/message_pump_ui.h"
#include "message_loop/message_loop_task_runner.h"
#include "message_loop/message_pump_dispatcher.h"
#include <mutex>

class MessageLoop final : public MessagePump::Delegate
{
public:
    enum class Type
    {
        UI,
        ASIO,
        Default,
    };

    using Dispatcher = MessagePumpDispatcher;

    explicit MessageLoop(Type type);
    ~MessageLoop() final;

    void run(Dispatcher* dispatcher = nullptr);

    static MessageLoop* current();

    [[nodiscard]] MessagePumpForUI* pumpWin() const;
    [[nodiscard]] MessagePumpForAsio* pumpAsio() const;
    [[nodiscard]] std::shared_ptr<TaskRunner> taskRunner() const;

protected:
    friend class MessageLoopTaskRunner;
    friend class Thread;

    using Clock = MessagePump::Clock;
    using TimePoint = MessagePump::TimePoint;
    using Milliseconds = MessagePump::Milliseconds;

    void postTask(PendingTask::Callback callback);
    void postDelayedTask(PendingTask::Callback callback, const Milliseconds& delay);

    PendingTask::Callback quitClosure();
    void runTask(const PendingTask& pending_task);
    void addToDelayedWorkQueue(PendingTask* pending_task);

    void addToIncomingQueue(PendingTask::Callback&& callback, const Milliseconds& delay);
    void reloadWorkQueue();
    bool deletePendingTasks();
    static TimePoint calculateDelayedRuntime(const Milliseconds& delay);

    // MessagePump::Delegate methods:
    bool doWork() final;
    bool doDelayedWork(TimePoint* next_delayed_work_time) final;
    
    const Type type_;
    TimePoint recent_time_;
    DelayedTaskQueue delayed_work_queue_;
    TaskQueue work_queue_;

    std::shared_ptr<MessagePump> pump_;

    TaskQueue incoming_queue_;
    std::mutex incoming_queue_lock_;

    int next_sequence_num_ = 0;
    std::shared_ptr<MessageLoopTaskRunner> proxy_;

private:
    void quit();
};