/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "common/task_worker.h"

std::shared_ptr<TaskRunner> TaskWorker::taskRunner() {
    static TaskWorker task_worker;
    return task_worker.getTaskRunner();
}

TaskWorker::TaskWorker() {
    worker_thread_.start(MessageLoop::Type::ASIO);
    worker_task_runner_ = worker_thread_.taskRunner();
}

TaskWorker::~TaskWorker() {
    worker_thread_.stop();
}

std::shared_ptr<TaskRunner> TaskWorker::getTaskRunner() {
    return worker_task_runner_;
}
