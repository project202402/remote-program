/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include <chrono>
#include <functional>
#include <memory>

class TaskRunner : public std::enable_shared_from_this<TaskRunner>
{
public:
    virtual ~TaskRunner() = default;

    using Callback = std::function<void()>;
    using Milliseconds = std::chrono::milliseconds;

    virtual bool belongsToCurrentThread() const = 0;
    virtual void postTask(Callback task) = 0;
    virtual void postDelayedTask(Callback callback, const Milliseconds& delay) = 0;
    virtual void postQuit() = 0;

    template <class T>
    static void doDelete(const void* object)
    {
        delete static_cast<const T*>(object);
    }

    template <class T>
    void deleteSoon(const T* object)
    {
        deleteSoonInternal(&TaskRunner::doDelete<T>, object);
    }

    template <class T>
    void deleteSoon(std::unique_ptr<T> object)
    {
        deleteSoon(object.release());
    }

private:
    void deleteSoonInternal(void(*deleter)(const void*), const void* object);
};