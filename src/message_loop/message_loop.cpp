/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "message_loop/message_loop.h"
#include "message_loop/message_loop_task_runner.h"
#include "message_loop/message_pump_default.h"
#include "common/logger.h"
#include <cassert>
#include <memory>


static thread_local MessageLoop* message_loop_for_current_thread = nullptr;

MessageLoop* MessageLoop::current()
{
    return message_loop_for_current_thread;
}


MessageLoop::MessageLoop(Type type):
    type_(type)
{
    assert(!current());

    message_loop_for_current_thread = this;

    proxy_.reset(new MessageLoopTaskRunner(this));

    switch (type)
    {
    case Type::ASIO:
        pump_ = std::make_unique<MessagePumpForAsio>();
        break;
#ifdef CLIENT_END
    case Type::UI:
        pump_ = std::make_unique<MessagePumpForUI>();
        break;
#endif
    default:
        pump_ = std::make_unique<MessagePumpDefault>();
        break;
    }
}


MessageLoop::~MessageLoop()
{
    assert(this == current());
    bool did_work;
    for (int i = 0; i < 100; ++i)
    {
        reloadWorkQueue();
        deletePendingTasks();

        did_work = deletePendingTasks();
        if (!did_work)
            break;
    }

    assert(!did_work);

    proxy_->willDestroyCurrentMessageLoop();
    proxy_ = nullptr;

    message_loop_for_current_thread = nullptr;
}


void MessageLoop::run(Dispatcher* dispatcher)
{
    assert(this == current());
#ifdef CLIENT_END
    if (dispatcher)
    {
        pumpWin()->runWithDispatcher(this, dispatcher);
        return;
    }
#endif
    pump_->run(this);
}


void MessageLoop::quit()
{
    assert(this == current());
    pump_->quit();
}


PendingTask::Callback MessageLoop::quitClosure()
{
    return std::bind(&MessageLoop::quit, this);
}


void MessageLoop::postTask(PendingTask::Callback callback)
{
    assert(callback != nullptr);
    addToIncomingQueue(std::move(callback), Milliseconds::zero());
}


void MessageLoop::postDelayedTask(PendingTask::Callback callback, const Milliseconds& delay)
{
    assert(callback != nullptr);
    addToIncomingQueue(std::move(callback), delay);
}

MessagePumpForUI* MessageLoop::pumpWin() const
{
    return static_cast<MessagePumpForUI*>(pump_.get());
}

MessagePumpForAsio* MessageLoop::pumpAsio() const
{
    return static_cast<MessagePumpForAsio*>(pump_.get());
}


std::shared_ptr<TaskRunner> MessageLoop::taskRunner() const
{
    return proxy_;
}

bool MessageLoop::doWork()
{
    for (;;)
    {
        reloadWorkQueue();

        if (work_queue_.empty())
            break;

        do
        {
            PendingTask pending_task = work_queue_.front();
            work_queue_.pop();

            if (pending_task.delayed_run_time != TimePoint())
            {
                const bool reschedule = delayed_work_queue_.empty();

                addToDelayedWorkQueue(&pending_task);

                if (reschedule)
                    pump_->scheduleDelayedWork(pending_task.delayed_run_time);
            }
            else
            {
                runTask(pending_task);
                return true;
            }
        } while (!work_queue_.empty());
    }

    return false;
}

bool MessageLoop::doDelayedWork(TimePoint* next_delayed_work_time)
{
    if (delayed_work_queue_.empty())
    {
        recent_time_ = *next_delayed_work_time = TimePoint();
        return false;
    }

    TimePoint next_run_time = delayed_work_queue_.top().delayed_run_time;

    if (next_run_time > recent_time_)
    {
        recent_time_ = Clock::now();
        if (next_run_time > recent_time_)
        {
            *next_delayed_work_time = next_run_time;
            return false;
        }
    }

    PendingTask pending_task = delayed_work_queue_.top();
    delayed_work_queue_.pop();

    if (!delayed_work_queue_.empty())
        *next_delayed_work_time = delayed_work_queue_.top().delayed_run_time;

    runTask(pending_task);
    return true;
}


void MessageLoop::addToIncomingQueue(PendingTask::Callback&& callback, const Milliseconds& delay)
{
    bool empty;

    {
        std::scoped_lock lock(incoming_queue_lock_);

        empty = incoming_queue_.empty();

        incoming_queue_.emplace(std::move(callback), calculateDelayedRuntime(delay));
    }

    if (!empty)
        return;

    std::shared_ptr<MessagePump> pump(pump_);
    pump->scheduleWork(); 
}

void MessageLoop::reloadWorkQueue()
{
    if (!work_queue_.empty())
        return;

    std::scoped_lock lock(incoming_queue_lock_);

    if (incoming_queue_.empty())
        return;

    incoming_queue_.Swap(&work_queue_);
}

void MessageLoop::addToDelayedWorkQueue(PendingTask* pending_task)
{
    delayed_work_queue_.emplace(std::move(pending_task->callback),
        pending_task->delayed_run_time,
        next_sequence_num_++);
}

void MessageLoop::runTask(const PendingTask& pending_task)
{
    pending_task.callback();
}

bool MessageLoop::deletePendingTasks()
{
    bool did_work = !work_queue_.empty();

    while (!work_queue_.empty())
    {
        PendingTask pending_task = work_queue_.front();
        work_queue_.pop();

        if (pending_task.delayed_run_time != TimePoint())
        {
            // We want to delete delayed tasks in the same order in which they would normally be
            // deleted in case of any funny dependencies between delayed tasks.
            addToDelayedWorkQueue(&pending_task);
        }
    }

    did_work |= !delayed_work_queue_.empty();

    while (!delayed_work_queue_.empty())
        delayed_work_queue_.pop();

    return did_work;
}

// static
MessageLoop::TimePoint MessageLoop::calculateDelayedRuntime(const Milliseconds& delay)
{
    TimePoint delayed_run_time;

    if (delay > Milliseconds::zero())
        delayed_run_time = Clock::now() + delay;

    return delayed_run_time;
}
