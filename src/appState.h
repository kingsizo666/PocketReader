#pragma once
#include <Arduino.h>

enum class AppScreen {
  MainMenu,
  Reader
};

extern AppScreen currentScreen;
extern String currentBookPath;