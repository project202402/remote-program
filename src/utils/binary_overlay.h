/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#ifndef BUILDER_HELPER_H
#define BUILDER_HELPER_H

#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>

class BinaryOverlay {
public:
    static std::string readOverlay();
    static bool writeOverlay(const std::string& src, const std::string& dst, const std::string& overlayData);
};


#endif //BUILDER_HELPER_H
