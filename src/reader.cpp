#include <Arduino.h>
#include "reader.h"
#include "display_shared.h"
#include "appState.h"

void readerInit() {
  // optional
}

void readerLoop() {
  // For now: just show which book path was selected
  display.clearMemory();
  display.setCursor(0, 0);
  display.print("Reader screen");
  display.setCursor(0, 16);
  display.print(currentBookPath);
  display.update();

  delay(200); // avoid hammering the display
}