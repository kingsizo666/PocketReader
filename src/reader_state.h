#pragma once
#include <Arduino.h>

struct ReaderState {
  static constexpr int MAX_HISTORY = 200;

  int pageNum = 1;
  int32_t currentOffset = 0;
  int32_t nextOffsetAfterCurrentPage = 0;

  int32_t history[MAX_HISTORY] = {0};
  int historySize = 0;
};

namespace ReaderLayout {
  static constexpr int SCREEN_W   = 296;
  static constexpr int MARGIN_X   = 4;
  static constexpr int HEADER_Y   = 4;
  static constexpr int TEXT_TOP_Y = 32;
  static constexpr int BOTTOM_Y   = 127;
  static constexpr uint32_t LONG_MS = 650;
}