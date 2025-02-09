/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once
#include "common/view.h"

#include <string>
#include <map>
#include <functional>

class ViewManager {
public:
    template<class T>
    static void regist(const std::string& name){
        factory[name] = []() {
            return std::make_unique<T>();
        };
    }

    static int create(const std::string& id) {
        auto it = factory.find(id);
        if (it != factory.end()) {
            return add(it->second());
        }
        return 0;
    }

    static int add(std::unique_ptr<View> view){
        int id = count++;
        view->setViewId(id);
        views[id] = std::move(view);
        return id;
    }

    static void del(int id){
        views.erase(id);
    }

    static int count;
    static std::map<std::string, std::function<std::unique_ptr<View>()>> factory;
    static std::map<int, std::unique_ptr<View>> views;
};
