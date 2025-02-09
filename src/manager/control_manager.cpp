/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "manager/control_manager.h"
#include "common/control.h"
#include <memory>
#include <map>
#include <string>
#include <functional>

std::map<std::string, std::function<Control*()>> ControlManager::factory;
std::map<std::string, std::function<void(Control*)>> ControlManager::deletors;
std::map<int, UniqueControlPtr> ControlManager::controls;
