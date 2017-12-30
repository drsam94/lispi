// (c) Sam Donow 2017
#pragma once
#include <limits>
#include <string>
#include <sstream>
#include <type_traits>
// Random utilities that don't have logical places to go
// Probaby should be broken into files once it gets large enough
//
// RAII file class
class FileOpen {
  private:
      FILE *fp;
  public:
    FileOpen(const char *path, const char *mode) :
        fp(fopen(path, mode)) {}
    FileOpen(FileOpen&) = delete;
    FileOpen& operator=(FileOpen&) = delete;
    FileOpen(FileOpen&&) = delete;
    FileOpen& operator=(FileOpen&&) = delete;
    ~FileOpen() { fclose(fp); }
    FILE *get() { return fp; }
};

namespace std {
    inline const char *operator+(const string &s) { return s.c_str(); }
}

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

// A helper for conveniently using std::visit in a pattern-matching like way
template<class... Ts>
struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts> Visitor(Ts...) -> Visitor<Ts...>;

// A scopeguard, better than traditional macro or function implementations due to
// C++17 class template parameter deduction
template<class Functor>
class ScopeGuard {
    Functor f;
  public:
    explicit ScopeGuard(Functor&& func) : f(std::move(func)) {}
    ~ScopeGuard() { f(); }
};

// Note: this is probably really slow (when compared to optimized solutions like
// absl::strcat), but it isn't used in very performance sensitive places.
template<typename... Ts>
std::string stringConcat(Ts&&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}

// std::algorithm type utility functions
namespace util {
template<typename ForwardIterator,
         typename Functor,
         typename OpType>
OpType foldr(ForwardIterator first, ForwardIterator last, OpType val, Functor f) {
    if (first == last) {
        return val;
    } else {
        const auto& elem = *first;
        return f(elem, foldr(++first, last, val, f));
    }
}

// SearchFunction is a function IntegralT -> <some integral type>
// such that it is == 0 for the answer, < 0 if the target is too low and > 0
// if too high
template<typename IntegralT, typename SearchFunction>
IntegralT binSearch(SearchFunction search) {
    IntegralT low = std::numeric_limits<IntegralT>::min();
    IntegralT high = std::numeric_limits<IntegralT>::max();
    // TODO debug this / test this more
    while (high > low + 1) {
        // safely compute (high + low) / 2, accounting for overflow
        IntegralT mid = low + ((high - low) / static_cast<IntegralT>(2));
        const int val = search(mid);
        if (val == 0) {
            return mid;
        } else if (val < 0) {
            low = mid;
        } else {
            high = mid;
        }
    }
    return low;
}
}
