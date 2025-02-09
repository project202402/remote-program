/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "manager/view_manager.h"

int ViewManager::count = 0;
std::map<std::string, std::function<std::unique_ptr<View>()>> ViewManager::factory;
std::map<int, std::unique_ptr<View>> ViewManager::views;