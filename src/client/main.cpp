/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "client/client.h"
#include "manager/view_manager.h"
#include "plugins/file/file_view.h"
#include "plugins/host/host_view.h"
#include "plugins/screen/screen_view.h"
#include "plugins/shell/shell_view.h"

void register_plugins()
{
    ViewManager::regist<HostView>(kHostInfoPlugin);
    ViewManager::regist<FileView>(kFilePlugin);
    ViewManager::regist<ShellView>(kShellPlugin);
    ViewManager::regist<ScreenView>(kScreenPlugin);
}

int main()
{
    register_plugins();
    Client client;
    client.run();
}

