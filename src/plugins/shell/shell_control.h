/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/shell/shell_define.h"
#include "plugins/shell/win_shell.h"
#include "common/control.h"

class ShellControl : public Control {
public:
    ShellControl();

private:
    int startShell(int view_id, int session_id);
    void stopShell();
    int execShell(const std::string& data);

    WinShell win_shell_;
};
