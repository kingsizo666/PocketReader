#pragma once
#include <Arduino.h>

struct BookInfo {
  const char* title;
  const char* path;
};

namespace BookLibrary {
  int count();
  const BookInfo& get(int index);
}