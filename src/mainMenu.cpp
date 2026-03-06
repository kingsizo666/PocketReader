#include <Arduino.h>
#include "display_shared.h"
#include "mainMenu.h"

static constexpr int BTN_PREV = 0;   // BOOT
static constexpr int BTN_NEXT = 21;  // GPIO21
static constexpr int PRESSED = LOW;

void mainMenuInit() {
    Serial.begin(115200);
    display.landscape(); 
    display.clearMemory();
    display.update();
}

void mainMenuDraw() {
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Select a book");
    display.setTextSize(1);
    display.update();
}