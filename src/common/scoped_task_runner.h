/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task_runner.h"
#include "common/macros_magic.h"

class ScopedTaskRunner : public std::enable_shared_from_this<ScopedTaskRunner>
{
public:
    explicit ScopedTaskRunner(std::shared_ptr<TaskRunner> task_runner);
    ~ScopedTaskRunner();

    void postTask(const TaskRunner::Callback& task);
    void postDelayedTask(const TaskRunner::Callback& callback, const TaskRunner::Milliseconds& delay) ;

private:
    bool attached_ = true;
    std::shared_ptr<TaskRunner> task_runner_;
    DISALLOW_COPY_AND_ASSIGN(ScopedTaskRunner);
};
