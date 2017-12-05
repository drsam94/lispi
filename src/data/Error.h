// (c) 2017
#pragma once

#include "util/Util.h"

#include <stdexcept>
#include <string_view>

class LispError : public std::runtime_error {
  public:
    explicit LispError(const std::string& what_arg)
        : std::runtime_error{what_arg} {}
    explicit LispError(const char* what_arg) : std::runtime_error{what_arg} {}

    // stringstream seems pretty slow, but error generation isn't exactly the
    // sort of thing that needs to be fast
    template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) > 1)>>
    LispError(Ts&&... args)
        : LispError{stringConcat(std::forward<Ts>(args)...)} {}
};

class TypeError : public LispError {
  public:
    TypeError(std::string_view expected, std::string_view found)
        : LispError("TypeError: expected ",  expected, " found, ", found) {}
};
