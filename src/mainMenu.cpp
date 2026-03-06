#include <Arduino.h>
#include "display_shared.h"
#include "mainMenu.h"
#include "bookLibrary.h"
#include "appState.h"
#include "Button.h"

// Button definitions
static constexpr int BTN_PREV = 0;
static constexpr int BTN_NEXT = 21;

static Button btnPrev(BTN_PREV);
static Button btnNext(BTN_NEXT);

// Menu data
static inline int BOOK_COUNT() { 
    return BookLibrary::count(); 
}
static int selected = 0;

static void onSelect(int idx) {
  const BookInfo& book = BookLibrary::get(idx);
  currentBookPath = book.path;
  currentScreen = AppScreen::Reader;
}

void mainMenuInit() {
    display.landscape(); 
    display.clearMemory();
    display.update();

    btnPrev.begin();
    btnNext.begin();
}

void mainMenuDraw() {
    display.clearMemory();

    // Big title
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Select a book");
    display.setTextSize(1);

    const int bookCount = BookLibrary::count();
    if (bookCount <= 0) {
        display.setCursor(0, 28);
        display.print("No books found");
        display.update();
        return;
    }
    if (selected >= bookCount) selected = bookCount - 1;

    // List
    const int y0 = 28;
    const int lineH = 14;

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
  const int bookCount = BookLibrary::count();
  if (bookCount <= 0) return;
  if (selected >= bookCount) selected = bookCount - 1;

  btnPrev.update();
  btnNext.update();

  bool redraw = false;

  // Prev: on release (short)
  if (btnPrev.released()) {
    selected = (selected - 1 + bookCount) % bookCount;
    redraw = true;
  }

  // Next: long press = select (consumes)
  if (btnNext.longPress(650)) {
    onSelect(selected);
    return;
  }

  // Next: short press on release if not consumed by long
  if (btnNext.released() && btnNext.shortReleaseNotConsumed()) {
    selected = (selected + 1) % bookCount;
    redraw = true;
  }

  if (redraw) mainMenuDraw();
  delay(5);
}