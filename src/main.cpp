#include <Arduino.h>
#include "mainMenu.h"
#include "appState.h"
#include "reader.h"

void setup() {
  mainMenuInit();
  mainMenuDraw();
}

void loop() {
  if (currentScreen == AppScreen::MainMenu) mainMenuLoop();
  else readerLoop();
}