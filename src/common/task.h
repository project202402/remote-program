/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include "nlohmann/json.hpp"
using Task = nlohmann::json;

#define RPC 0xff

struct TaskFactory {
 static Task task(int view_id, int session_id, const std::string& plugin, int cmd) {
  Task task;
  task["view_id"] = view_id;
  task["session_id"] = session_id;
  task["plugin"] = plugin;
  task["command"] = cmd;
  return std::move(task);
 }

 static Task rpc(int view_id, int session_id, const std::string& plugin, int cmd, uint64_t cb_id, const std::string& data) {
  Task task = TaskFactory::task(view_id, session_id, plugin, cmd);
  task["cb_id"] = cb_id;
  task["data"] = data;
  return std::move(task);
 }
};
