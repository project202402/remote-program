
/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#ifndef CODEC_H
#define CODEC_H


#include <nlohmann/json.hpp>

using buffer_type = std::string;

struct json_codec {
    template <typename... Args>
    static buffer_type pack_args(Args&&... args) {
        nlohmann::json j = std::make_tuple(std::forward<Args>(args)...);
        return j.dump();
    }

    template <typename Arg, typename... Args>
    static std::string pack_args_str(Arg arg, Args&&... args) {
        nlohmann::json j = std::make_tuple(static_cast<int>(arg), std::forward<Args>(args)...);
        return j.dump();
    }

    template <typename T>
    buffer_type pack(T&& t) const {
        nlohmann::json j = std::forward<T>(t);
        return j.dump();
    }

    template <typename T>
    T unpack(const char* data, size_t length) {
        try {
            auto j = nlohmann::json::parse(data, data + length);
            return j.get<T>();
        } catch (...) {
            throw std::invalid_argument("unpack failed: Args not match!");
        }
    }
};

template <typename T>
T as(std::string_view result) {
    json_codec codec;
    auto tp = codec.unpack<std::tuple<int, T>>(result.data(), result.size());
    return std::get<1>(tp);
}

inline bool has_error(std::string_view result) {
    if (result.empty()) {
        return true;
    }

    json_codec codec;
    auto tp = codec.unpack<std::tuple<int>>(result.data(), result.size());
    return std::get<0>(tp) != 0;
}

inline std::string get_error_msg(std::string_view result) {
    json_codec codec;
    auto tp =
        codec.unpack<std::tuple<int, std::string>>(result.data(), result.size());
    return std::get<1>(tp);
}


enum {
    OK,
    FAIL
};

template <typename F, typename Tuple, size_t... I>
decltype(auto) call_helper_impl(const F& f, Tuple&& tup, std::index_sequence<I...>) {
    return f(std::get<I + 1>(std::forward<Tuple>(tup))...);
}

template <typename F, typename Arg, typename... Args>
decltype(auto) call_helper(const F& f, std::tuple<Arg, Args...> tup) {
    return call_helper_impl(f, std::move(tup), std::make_index_sequence<sizeof...(Args)>{});
}

template <typename F, typename Arg, typename... Args>
std::enable_if_t<std::is_void_v<std::invoke_result_t<F, Args...>>>
call(const F& f, std::string& result, std::tuple<Arg, Args...> tp) {
    call_helper(f, std::move(tp));
    result = json_codec::pack_args_str(OK);
}

template <typename F, typename Arg, typename... Args>
std::enable_if_t<!std::is_void_v<std::invoke_result_t<F, Args...>>>
call(const F& f, std::string& result, std::tuple<Arg, Args...> tp) {
    auto r = call_helper(f, std::move(tp));
    result = json_codec::pack_args_str(OK, r);
}


#endif //CODEC_H
