/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <string_view>

template <typename T> struct function_traits;

template <typename Ret, typename Arg, typename... Args>
struct function_traits<Ret(Arg, Args...)> {
    static constexpr size_t arity = sizeof...(Args) + 1;
    using function_type = Ret(Arg, Args...);
    using return_type = Ret;
    using stl_function_type = std::function<function_type>;
    using pointer = Ret(*)(Arg, Args...);

    using tuple_type = std::tuple<Arg, Args...>;
    using bare_tuple_type = std::tuple<std::remove_cvref_t<Arg>, std::remove_cvref_t<Args>...>;

    using args_tuple = std::tuple<std::string, std::remove_cvref_t<Arg>, std::remove_cvref_t<Args>...>;
};

template <typename Ret> struct function_traits<Ret()> {
    static constexpr size_t arity = 0;
    using function_type = Ret();
    using return_type = Ret;
    using stl_function_type = std::function<function_type>;
    using pointer = Ret(*)();

    using tuple_type = std::tuple<>;
    using bare_tuple_type = std::tuple<>;
    using args_tuple = std::tuple<std::string>;
};

template <typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Ret, typename... Args>
struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)> {};

template <typename ReturnType, typename ClassType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...)> : function_traits<ReturnType(Args...)> {};

template <typename ReturnType, typename ClassType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> : function_traits<ReturnType(Args...)> {};

template <typename Callable>
struct function_traits : function_traits<decltype(&Callable::operator())> {};
