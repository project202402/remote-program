/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "manager/setting_manager.h"
#include <Windows.h>
#include <fstream>
#include <iostream>

SettingManager & SettingManager::getInstance() {
 static SettingManager instance;
 return instance;
}

json & SettingManager::getConfig()  {
 std::shared_lock<std::shared_mutex> lock(mutex_);
 return config_;
}


void SettingManager::saveConfig()  {
 std::shared_lock<std::shared_mutex> lock(mutex_);
 WriteJsonFile(file_path_, config_);
}

void SettingManager::reloadConfig()  {
 std::unique_lock<std::shared_mutex> lock(mutex_);
 config_ = ReadJsonFile(file_path_);
}

SettingManager::SettingManager()  {
 file_path_ = GetExecutablePath() + "\\config.json";
 config_ = ReadJsonFile(file_path_);
 if (config_.empty()) {
  config_["listen_ip"] = std::string("0.0.0.0");
  config_["listen_port"] = 8888;
  config_["server_ip"] = std::string("127.0.0.1");
  config_["server_port"] = 8888;
  config_["server_path"] = std::string("server.dat");
 }
}

SettingManager::~SettingManager() {
 WriteJsonFile(file_path_, config_);
}

std::string SettingManager::GetExecutablePath()  {
 char buffer[MAX_PATH];
 GetModuleFileNameA(NULL, buffer, MAX_PATH);
 std::string::size_type pos = std::string(buffer).find_last_of("\\/");
 return std::string(buffer).substr(0, pos);
}


json SettingManager::ReadJsonFile(const std::string &filePath)  {
 std::ifstream file(filePath);
 json j;
 if (file.is_open()) {
  file >> j;
 }
 return j;
}

void SettingManager::WriteJsonFile(const std::string &filePath, const json &j)  {
 std::ofstream file(filePath);
 if (file.is_open()) {
  file << j.dump(4);
 }
}
