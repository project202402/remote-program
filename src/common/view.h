/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task_consumer.h"
#include "common/task_producer.h"
#include "common/task_runner.h"
#include "rpc/rpc_client.h"

class View : public TaskConsumer
{
public:
    ~View() override = default;
    virtual void draw() = 0;

    virtual void start() {}
    virtual void stop() {}

    void setViewId(int view_id){ view_id_ = view_id;}
    int  getViewId() const {return view_id_;}

    void setSessionId(int session_id){ session_id_ = session_id;}
    int  getSessionId() const {return session_id_;}

    void setTaskProducer(TaskProducer* task_producer) { task_producer_ = task_producer;}
    TaskProducer* getTaskProducer() {return task_producer_;}

    void setTaskRunner(std::shared_ptr<TaskRunner> task_runner) { task_runner_ = std::move(task_runner);}
    std::shared_ptr<TaskRunner> getTaskRunner() { return task_runner_;}

    template <typename... Args>
    void call(int session_id, const std::string& plugin, const std::string& func_name,
            std::function<void(std::error_code, std::string_view)> cb,
            Args &&...args) {
        uint64_t cb_id = 0;
        auto buffer = rpc_client_.call(func_name, cb, cb_id, std::forward<Args>(args)...);
        auto task = TaskFactory::rpc(getViewId(), session_id, plugin, RPC, cb_id, buffer);
        getTaskProducer()->doTask(task);
    }

    void onTaskDone(Task task) override {
        uint64_t id = task["cb_id"];
        std::string data = task["data"];
        rpc_client_.callback(id, data);
    }

protected:
    bool open_ = true;

private:
    int view_id_= 0;
    int session_id_ = 0;
    TaskProducer* task_producer_ = nullptr;
    std::shared_ptr<TaskRunner> task_runner_;
    RpcClient rpc_client_;
};
