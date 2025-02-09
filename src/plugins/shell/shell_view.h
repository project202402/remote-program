/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/shell/shell_define.h"
#include "common/view.h"

class ShellView : public View {
public:
    void draw() override;
    void start() override;
    void stop() override;
    void onTaskDone(Task task) override;

private:
    std::string shell_result_;
};