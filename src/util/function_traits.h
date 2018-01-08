// (c) 2018 Sam Donow
#pragma once
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

// TODO: gcc and clang both encounter myriad bugs trying to deal with this. So I
// stopped trying for now
template<typename T>
struct function_traits_impl;

template<typename R, typename... Args>
struct function_traits_impl<std::function<R(Args...)>> {
    static constexpr size_t arity = sizeof...(Args);
    using ReturnType = R;
    using ArgTupleType = std::tuple<Args...>;
};

template<typename Callable>
using function_traits = function_traits_impl<decltype(
                            std::function{std::declval<std::decay_t<Callable>>()})>;
