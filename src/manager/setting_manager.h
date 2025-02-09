/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#ifndef SETTING_MANAGER_H
#define SETTING_MANAGER_H

#include "nlohmann/json.hpp"
#include <shared_mutex>

using json = nlohmann::json;

class SettingManager {
public:
    static SettingManager& getInstance();
    SettingManager(const SettingManager&) = delete;
    SettingManager& operator=(const SettingManager&) = delete;

    json& getConfig();

    template<typename T>
    T getField(const std::string& fieldName, const T& defaultValue = T()) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        if (config_.contains(fieldName)) {
            return config_[fieldName].get<T>();
        }
        return defaultValue;
    }

    template<typename T>
    void setField(const std::string& fieldName, const T& value) {
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            config_[fieldName] = value;
        }
        saveConfig();
    }

    void saveConfig();
    void reloadConfig();

private:
    SettingManager();
    ~SettingManager();

    static std::string GetExecutablePath();
    static json ReadJsonFile(const std::string& filePath);
    static void WriteJsonFile(const std::string& filePath, const json& j);

    json config_;
    std::string file_path_;
    mutable std::shared_mutex mutex_;

};

#endif //SETTING_MANAGER_H
