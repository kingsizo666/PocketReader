#include <Arduino.h>
#include "mainMenu.h"
#include "appState.h"
#include "reader.h"

static AppScreen lastScreen = AppScreen::MainMenu;

void setup() {
  mainMenuInit();
  mainMenuDraw();
  lastScreen = currentScreen;
}

void loop() {
  // If screen changed, draw the new screen once
  if (currentScreen != lastScreen) {
    lastScreen = currentScreen;

    if (currentScreen == AppScreen::MainMenu) {
      mainMenuDraw();
    } else {
      // entering reader
      readerInit();
    }
  }

  if (currentScreen == AppScreen::MainMenu) {
    mainMenuLoop();
  } else {
    readerLoop();
  }
}