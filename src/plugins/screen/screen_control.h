/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/screen/screen_define.h"
#include "plugins/screen/win_screen.h"
#include "common/control.h"

class ScreenControl final : public Control {
public:
    ScreenControl();

private:
    std::unique_ptr<WinScreen> win_screen_;
};
