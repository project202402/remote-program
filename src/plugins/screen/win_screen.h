/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include <functional>

class WinScreen {
public:
    WinScreen() = default;
    ~WinScreen();

    void start();
    void stop();
    void setQuality(int quality);
    std::vector<char> getScreenData();

private:

    int quality_ = 10;
};

