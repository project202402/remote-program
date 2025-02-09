/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include "codec.h"
#include "function_traits.h"

#include <functional>
#include <mutex>
#include <string>
#include <system_error>

class RpcClient {
 public:
    template <typename... Args>
    decltype(auto) call(const std::string& func_name,
            std::function<void(std::error_code, std::string_view)> cb, uint64_t& cb_id,
            Args &&...args) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            callback_id_++;
            cb_id = callback_id_;
            callbacks_.emplace(cb_id, std::move(cb));
        }

        json_codec codec;
        return codec.pack_args(func_name, std::forward<Args>(args)...);
    }

    void callback(uint64_t callback_id, std::string_view data)
    {
        std::function<void(std::error_code, std::string_view)> cb = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cb = std::move(callbacks_[callback_id]);
        }

        std::error_code ec{};
        cb(ec, data);

        {
            std::unique_lock<std::mutex> lock(mutex_);
            callbacks_.erase(callback_id);
        }
    }

 private:
    using Callback = std::function<void(std::error_code, std::string_view)>;
    using Callbacks = std::unordered_map<std::uint64_t, Callback>;
    std::mutex  mutex_;
    uint64_t	callback_id_ = 0;
    Callbacks	callbacks_;
};

#endif //RPC_CLIENT_H
