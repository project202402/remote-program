/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/shell/shell_control.h"
#include "plugins/shell/shell_define.h"

EXPORT_CLASS(ShellControl)
EXPORT_DELETEOBJECT

ShellControl::ShellControl() {
    bind("startShell", [this](int view_id, int session_id) {
        return startShell(view_id, session_id);
    });

    bind("stopShell", [this] {
        return stopShell();
    });

    bind("execShell", [this](const std::string& cmd) {
        return execShell(cmd);
    });
}

int ShellControl::startShell(int view_id, int session_id){
    win_shell_.setCallback([this, view_id, session_id](const std::string& buf) {
        auto task = TaskFactory::task(view_id, session_id, kShellPlugin, SHELL_COMMAND);
        task["status"] = 1;
        task["output"] = buf;
        getTaskConsumer()->onTaskDone(task);
    });
    int status = win_shell_.start();
    return status;
}

void ShellControl::stopShell() {
    win_shell_.stop();
}

int ShellControl::execShell(const std::string& data) {
    int status = win_shell_.write(data);
    return status;
}