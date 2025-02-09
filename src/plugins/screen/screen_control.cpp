/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/screen/screen_control.h"
#include "common/logger.h"

EXPORT_CLASS(ScreenControl)
EXPORT_DELETEOBJECT

ScreenControl::ScreenControl() {
    bind("startCapture", [this] {
        win_screen_ = std::make_unique<WinScreen>();
        win_screen_->start();
        return 1;
    });
    bind("stopCapture", [this] {
        win_screen_->stop();
        win_screen_.reset();
        return 1;
    });
    bind("captureScreen", [this](int quality) {
        win_screen_->setQuality(quality);
        return win_screen_->getScreenData();
    });

}