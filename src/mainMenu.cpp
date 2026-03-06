#include <Arduino.h>
#include "display_shared.h"
#include "mainMenu.h"
#include "bookLibrary.h"
#include "appState.h"

// Button definitions
static constexpr int BTN_PREV = 0;
static constexpr int BTN_NEXT = 21;
static constexpr int PRESSED = LOW;

// Menu data
static inline int BOOK_COUNT() { 
    return BookLibrary::count(); 
}
static int selected = 0;

// long press button state
struct Button {
  bool isDown = false;
  bool lastRaw = false;
  uint32_t lastRawChangeMs = 0;

  uint32_t pressStartMs = 0;
  bool consumed = false;
};

static Button prevBtn, nextBtn;

static bool rawDown(int pin) { 
    return digitalRead(pin) == PRESSED; 
}

static bool updateDebounced(int pin, Button& b, uint32_t debounceMs = 35) {
  bool raw = rawDown(pin);
  uint32_t t = millis();

  if (raw != b.lastRaw) {
    b.lastRaw = raw;
    b.lastRawChangeMs = t;
  }

  // If raw has been stable long enough, accept it
  if ((t - b.lastRawChangeMs) >= debounceMs) {
    if (b.isDown != raw) {
      b.isDown = raw;
      return true;
    }
  }
  return false;
}

static void onSelect(int idx) {
  const BookInfo& book = BookLibrary::get(idx);
  currentBookPath = book.path;
  currentScreen = AppScreen::Reader;
}

void mainMenuInit() {
    Serial.begin(115200);
    display.landscape(); 
    display.clearMemory();
    display.update();

    pinMode(BTN_PREV, INPUT_PULLUP);
    pinMode(BTN_NEXT, INPUT_PULLUP);
}

void mainMenuDraw() {
    display.clearMemory();

    // Big title
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Select a book");
    display.setTextSize(1);

    // List
    const int y0 = 28;
    const int lineH = 14;

    const int bookCount = BookLibrary::count();
    if (bookCount <= 0) {
        display.setCursor(0, 28);
        display.print("No books found");
        display.update();
        return;
    }
    if (selected >= bookCount) selected = bookCount - 1;

    for (int i = 0; i < bookCount; i++) {
        display.setCursor(0, y0 + i * lineH);
        display.print(i == selected ? "> " : "  ");
        display.print(BookLibrary::get(i).title);
    }

    // Hint
    display.setCursor(0, y0 + bookCount * lineH + 2);
    display.print("BOOT=Prev, 21=Next (hold=Select)");

    display.update();
}

void mainMenuLoop() {
  const uint32_t LONG_MS = 650;
  bool redraw = false;

  const int bookCount = BookLibrary::count();
  if (bookCount <= 0) return;
  if (selected >= bookCount) selected = bookCount - 1;

  // PREV: short press on release
  bool prevChanged = updateDebounced(BTN_PREV, prevBtn);
  if (prevChanged && prevBtn.isDown) {
    prevBtn.pressStartMs = millis();
    prevBtn.consumed = false;
  }
  if (prevChanged && !prevBtn.isDown) { 
    selected = (selected - 1 + bookCount) % bookCount;
    redraw = true;
  }

  // NEXT/SELECT: short = next, long = select
  bool nextChanged = updateDebounced(BTN_NEXT, nextBtn);

  if (nextChanged && nextBtn.isDown) {
    nextBtn.pressStartMs = millis();
    nextBtn.consumed = false;
  }
  // Long press logic
  if (nextBtn.isDown && !nextBtn.consumed) {
    if (millis() - nextBtn.pressStartMs >= LONG_MS) {
      nextBtn.consumed = true;
      onSelect(selected);
      Serial.println("Selected book");
    }
  }

  // If long wasn't consumed, treat as short
  if (nextChanged && !nextBtn.isDown) {
    if (!nextBtn.consumed) {
      selected = (selected + 1) % bookCount;
      redraw = true;
    }
  }

  if (redraw) mainMenuDraw();
  delay(5);
}