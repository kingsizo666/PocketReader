#include <Arduino.h>
#include <LittleFS.h>
#include "reader.h"
#include "display_shared.h"
#include "appState.h"
#include "Button.h"
#include <Fonts/FreeSans9pt7b.h>

// Buttons
static constexpr int BTN_PREV = 0;   // BOOT
static constexpr int BTN_NEXT = 21;  // GPIO21

static Button btnPrev(BTN_PREV);
static Button btnNext(BTN_NEXT);

// Screen geometry
static constexpr int SCREEN_W = 296;

// Layout
static constexpr int MARGIN_X   = 4;
static constexpr int HEADER_Y   = 4;   // baseline
static constexpr int TEXT_TOP_Y = 32;   // first text baseline
static constexpr int BOTTOM_Y   = 127;  // last usable baseline area
static constexpr uint32_t LONG_MS = 650;

// Reading state
static int pageNum = 1;
static int32_t currentOffset = 0;
static int32_t nextOffsetAfterCurrentPage = 0;

// Back stack
static constexpr int MAX_HISTORY = 200;
static int32_t history[MAX_HISTORY];
static int historySize = 0;

static inline String bookFilePath() {
  return currentBookPath + "/book.txt";
}

static int textWidthPx(const String& s) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(s.c_str(), 0, 0, &x1, &y1, &w, &h);
  return (int)w;
}

// Force a clean white frame first to reduce ghosting
static void beginFreshPage() {
  display.clearMemory();
}

static int32_t renderPageFromOffset(const String& filePath, int32_t startOffset) {
  File f = LittleFS.open(filePath, "r");
  if (!f) {
    beginFreshPage();
    display.setFont();
    display.setCursor(0, 16);
    display.print("[Missing book.txt]");
    display.setCursor(0, 32);
    display.print(filePath);
    display.update();
    return startOffset;
  }

  if (!f.seek(startOffset)) {
    startOffset = 0;
    f.seek(0);
  }

  beginFreshPage();

  display.setFont();
  display.setTextWrap(true);
  display.setCursor(MARGIN_X, HEADER_Y);
  display.print("Page ");
  display.print(pageNum);

  display.setFont(&FreeSans9pt7b);
  display.setTextWrap(false);

  const int maxWidth = SCREEN_W - 2 * MARGIN_X - 2;
  const int lineH = (int)FreeSans9pt7b.yAdvance + 1;

  int y = TEXT_TOP_Y;
  String line;
  String word;

  int32_t lastBreakOffset = startOffset;
  int32_t wordStartOffset = startOffset;

  auto pageFull = [&]() -> bool {
    return y > (BOTTOM_Y - lineH);
  };

  auto flushLine = [&]() {
    if (line.isEmpty()) return;
    display.setCursor(MARGIN_X, y);
    display.print(line);
    y += lineH;
    line = "";
  };

  auto commitWord = [&](int32_t breakPos) -> int32_t {
    if (word.isEmpty()) return -1;

    String candidate = line.length() ? (line + " " + word) : word;

    if (textWidthPx(candidate) <= maxWidth) {
      line = candidate;
      word = "";
      lastBreakOffset = breakPos;
      return -1;
    }

    flushLine();

    if (pageFull()) {
      return wordStartOffset;
    }

    if (textWidthPx(word) > maxWidth) {
      String chunk;
      for (size_t i = 0; i < word.length(); i++) {
        String cand = chunk + word[i];
        if (textWidthPx(cand) <= maxWidth) {
          chunk = cand;
        } else {
          line = chunk;
          flushLine();

          if (pageFull()) {
            return wordStartOffset;
          }

          chunk = String(word[i]);
        }
      }

      line = chunk;
      word = "";
      lastBreakOffset = breakPos;
      return -1;
    }

    line = word;
    word = "";
    lastBreakOffset = breakPos;
    return -1;
  };

  while (f.available()) {
    if (pageFull()) break;

    int c = f.read();
    int32_t pos = (int32_t)f.position();

    if (c == '\r') continue;

    if (c == ' ' || c == '\t' || c == '\n') {
      int32_t stopAt = commitWord(pos);
      if (stopAt >= 0) {
        display.setFont();
        display.update();
        f.close();
        return stopAt;
      }

      if (c == '\n') {
        flushLine();
        if (pageFull()) {
          display.setFont();
          display.update();
          f.close();
          return pos;
        }
        lastBreakOffset = pos;
      } else {
        lastBreakOffset = pos;
      }
    } else {
      if (word.isEmpty()) {
        wordStartOffset = pos - 1;
      }
      word += (char)c;
    }
  }

  if (!pageFull()) {
    int32_t stopAt = commitWord((int32_t)f.position());
    if (stopAt >= 0) {
      display.setFont();
      display.update();
      f.close();
      return stopAt;
    }
    flushLine();
  }

  display.setFont();
  display.setTextWrap(true);
  display.update();
  f.close();

  int32_t nextOffset = lastBreakOffset;
  if (nextOffset <= startOffset) nextOffset = startOffset + 1;
  return nextOffset;
}

static void openBookAndReset() {
  pageNum = 1;
  currentOffset = 0;
  nextOffsetAfterCurrentPage = 0;
  historySize = 0;
}

static void drawCurrentPage() {
  const String fp = bookFilePath();
  nextOffsetAfterCurrentPage = renderPageFromOffset(fp, currentOffset);
}

void readerInit() {
  btnPrev.begin();
  btnNext.begin();

  openBookAndReset();
  drawCurrentPage();
}

void readerLoop() {
  btnPrev.update();
  btnNext.update();

  if (btnPrev.longPress(LONG_MS)) {
    currentScreen = AppScreen::MainMenu;
    return;
  }

  if (btnPrev.released() && btnPrev.shortReleaseNotConsumed()) {
    if (historySize > 0) {
      currentOffset = history[--historySize];
      pageNum = max(1, pageNum - 1);
      drawCurrentPage();
    }
  }

  if (btnNext.released() && btnNext.shortReleaseNotConsumed()) {
    if (nextOffsetAfterCurrentPage > currentOffset) {
      if (historySize < MAX_HISTORY) {
        history[historySize++] = currentOffset;
      }

      currentOffset = nextOffsetAfterCurrentPage;
      pageNum++;
      drawCurrentPage();
    }
  }

  delay(5);
}