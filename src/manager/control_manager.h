/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */


#pragma once

#include "common/control.h"
#include <string>
#include <map>
#include <memory>
#include <utility>


using UniqueControlPtr = std::unique_ptr<Control, std::function<void(Control*)>>;

class ControlManager {
 public:

    static void regist(const std::string& name,
        std::function<Control*()> creator,
        std::function<void(Control*)> deletor) {
        factory[name] = std::move(creator);
        deletors[name] = std::move(deletor);
    }

    static bool create(int id, const std::string &name) {
        auto it = ControlManager::factory.find(name);
        if(it != ControlManager::factory.end()) {
            auto creator = it->second;
            auto deletor = deletors[name];
            std::unique_ptr<Control, std::function<void(Control*)>> ptr(creator(), deletor);
            if (controls.find(id) == controls.end()) {
                controls[id] = std::move(ptr);
                return true;
            }
        }
        return false;
    }

    static void remove(int id) { controls.erase(id); }

    static int count;
    static std::map<std::string, std::function<Control*()>> factory;
    static std::map<std::string, std::function<void(Control*)>> deletors;
    static std::map<int, UniqueControlPtr> controls;
};
