// (c) Sam Donow 2017
#pragma once
#include <string>
#include <sstream>
// Random utilities that don't have logical places to go

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

// Note: this is probably really slow (when compared to optimized solutions like
// absl::strcat), but it isn't used in very performance sensitive places.
template<typename... Ts>
std::string stringConcat(Ts&&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}
