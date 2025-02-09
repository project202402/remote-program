/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include "common/task.h"

class TaskProducer {
public:
    virtual ~TaskProducer() = default;
    virtual void doTask(Task task) = 0;
};

