/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "common/task.h"

class TaskConsumer {
public:
    virtual ~TaskConsumer() = default;
    virtual void onTaskDone(Task task) = 0;
};