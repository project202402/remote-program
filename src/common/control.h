/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task_consumer.h"
#include "common/task_producer.h"
#include "rpc/rpc_server.h"
#include "task_runner.h"

#ifndef MEMORYLOAD
#define EXPORT_CLASS(X)
#define EXPORT_DELETEOBJECT
#else
#define EXPORT_CLASS(X) extern "C" __declspec(dllexport) Control* __stdcall createControlObject() { \
return new X(); \
}

#define EXPORT_DELETEOBJECT extern "C" __declspec(dllexport) void __stdcall deleteControlObject(Control* obj) { \
delete obj; \
}
#endif

class Control : public TaskProducer
{
 public:
    ~Control() override = default;

    void setTaskConsumer(TaskConsumer* task_consumer) { task_consumer_ = task_consumer;}
    TaskConsumer* getTaskConsumer() {return task_consumer_;}

    void setTaskRunner(std::shared_ptr<TaskRunner> task_runner) { task_runner_ = std::move(task_runner);}
    std::shared_ptr<TaskRunner> getTaskRunner() { return task_runner_;}

    template <typename Function>
    void bind(std::string const& name, Function f)
    {
        return rpc_server_.bind(name, f);
    }

    void doTask(Task task) override {
        std::string data = task["data"];
        auto response = rpc_server_.route(data);
        task["data"] = response;
        getTaskConsumer()->onTaskDone(task);
    }

 private:
    TaskConsumer* task_consumer_ = nullptr;
    std::shared_ptr<TaskRunner> task_runner_;
    RpcServer rpc_server_;
};

using ControlObjectPtr = Control* (__stdcall *)();
using DeleteControlObjectPtr = void (__stdcall *)(Control*);
