/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task_runner.h"
#include "common/thread.h"

class TaskWorker {
public:
    static std::shared_ptr<TaskRunner> taskRunner();
private:
    TaskWorker();
    ~TaskWorker();

    std::shared_ptr<TaskRunner> getTaskRunner();

    Thread                          worker_thread_;
    std::shared_ptr<TaskRunner>     worker_task_runner_;
};

