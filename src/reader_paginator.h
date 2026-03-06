#pragma once
#include <Arduino.h>
#include "reader_state.h"

namespace ReaderPaginator {
  int32_t renderPageFromOffset(const String& filePath, int32_t startOffset, int pageNum);
  int32_t computeNextOffsetFrom(const String& filePath, int32_t startOffset);
  void rebuildHistoryToCurrentPage(ReaderState& state, const String& filePath);
}