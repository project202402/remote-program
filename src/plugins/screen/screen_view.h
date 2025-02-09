/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "plugins/screen/screen_define.h"
#include "common/view.h"

class ScreenView final : public View {
public:
    void draw() override;
    void start() override;

private:
    void startCapture();
    void stopCapture();
    void captureScreen();

    int screen_quality_ = 10;
    int screen_width_ = 0;
    int screen_height_ = 0;
    unsigned int screen_texture_ = 0;
    std::vector<char> data_ {};
};