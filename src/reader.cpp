#include <Arduino.h>
#include "reader.h"
#include "display_shared.h"
#include "appState.h"
#include "Button.h"

static constexpr int BTN_BACK = 0; 
static Button btnBack(BTN_BACK);

static void drawReaderOnce() {
  display.clearMemory();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Reader");
  display.setTextSize(1);

  display.setCursor(0, 28);
  display.print("Book path:");
  display.setCursor(0, 42);
  display.print(currentBookPath);

  display.setCursor(0, 110);
  display.print("Hold BOOT to go back");
  display.update();
}

void readerInit() {
  btnBack.begin();
  drawReaderOnce();
}

void readerLoop() {
  btnBack.update();

  if (btnBack.longPress(650)) {
    currentScreen = AppScreen::MainMenu;
    return;
  }

  delay(5);
}