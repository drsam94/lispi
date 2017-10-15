// (c) Sam Donow 2017
#pragma once
#include<string>
// RAII file class
class FileOpen {
  private:
      FILE *fp;
  public:
    FileOpen(const char *path, const char *mode) :
        fp(fopen(path, mode)) {}

    ~FileOpen() { fclose(fp); }
    FILE *get() { return fp; }
};

namespace std {
    inline const char *operator+(const string &s) { return s.c_str(); }
}

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
