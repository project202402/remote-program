/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "common/scoped_task_runner.h"

ScopedTaskRunner::ScopedTaskRunner(std::shared_ptr<TaskRunner> task_runner):
        task_runner_(std::move(task_runner)), attached_(true){

}

ScopedTaskRunner::~ScopedTaskRunner() {
    attached_ = false;
}

void ScopedTaskRunner::postTask(const TaskRunner::Callback& callback) {
    auto self = shared_from_this();
    task_runner_->postTask([self, callback]()
    {
       if (!self->attached_)
           return;

       callback();
    });
}

void ScopedTaskRunner::postDelayedTask(const TaskRunner::Callback& callback, const TaskRunner::Milliseconds &delay) {
    auto self = shared_from_this();
    task_runner_->postDelayedTask([self, callback]()
    {
      if (!self->attached_)
          return;

      callback();
    }, delay);
}
