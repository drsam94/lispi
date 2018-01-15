// (c) 2018 Sam Donow
#pragma once
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

// TODO: add support for more kinds of function types
template <typename T> struct function_traits;

template <typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
    static constexpr size_t arity = sizeof...(Args);
    using ReturnType = R;
    using ArgTupleType = std::tuple<Args...>;
};

template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
    static constexpr size_t arity = sizeof...(Args);
    using ReturnType = R;
    using ArgTupleType = std::tuple<Args...>;
};
