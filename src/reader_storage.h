#pragma once
#include <Arduino.h>
#include "reader_state.h"

namespace ReaderStorage {
  String bookFilePath();
  String progressFilePath();

  void saveProgress(const ReaderState& state);
  void loadProgress(ReaderState& state);
}