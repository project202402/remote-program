/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include "codec.h"
#include "function_traits.h"
#include <string>


class RpcServer {
 public:
    template <typename Function>
    void bind(std::string const& name, Function f)
    {
        invokers_[name] = [f](const char* data, size_t size, buffer_type& result) {
            using args_tuple = typename function_traits<Function>::args_tuple;
            json_codec codec;
            try {
                auto tp = codec.unpack<args_tuple>(data, size);
                call(f, result, std::move(tp));
            } catch (const std::invalid_argument& e) {
                result = json_codec::pack_args_str(FAIL, e.what());
            }
        };
    }

    decltype(auto) route(std::string_view data)
    {
        buffer_type result;
        json_codec codec;
        try {
            auto p = codec.unpack<std::tuple<std::string>>(data.data(), data.size());
            auto& func_name = std::get<0>(p);
            auto it = invokers_.find(func_name);
            assert(it != invokers_.end());
            it->second(data.data(), data.size(), result);
        }
        catch (const std::exception& ex) {
            result = json_codec::pack_args_str(FAIL, ex.what());
        }
        return result;
    }

 private:
    using Invoke = std::function<void(const char* data, size_t size, buffer_type&)>;
    using Invokes = std::unordered_map<std::string, Invoke>;
    Invokes invokers_;
};
#endif //RPC_SERVER_H
