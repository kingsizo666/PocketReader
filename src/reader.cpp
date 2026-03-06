#include <Arduino.h>
#include "reader.h"
#include "appState.h"
#include "Button.h"
#include "reader_state.h"
#include "reader_storage.h"
#include "reader_paginator.h"

static constexpr int BTN_PREV = 0;   // BOOT
static constexpr int BTN_NEXT = 21;  // GPIO21

static Button btnPrev(BTN_PREV);
static Button btnNext(BTN_NEXT);

static ReaderState state;

static void openBookAndRestore() {
  state.nextOffsetAfterCurrentPage = 0;
  ReaderStorage::loadProgress(state);
  ReaderPaginator::rebuildHistoryToCurrentPage(state, ReaderStorage::bookFilePath());
}

static void drawCurrentPage() {
  state.nextOffsetAfterCurrentPage =
      ReaderPaginator::renderPageFromOffset(
          ReaderStorage::bookFilePath(),
          state.currentOffset,
          state.pageNum
      );
}

void readerInit() {
  btnPrev.begin();
  btnNext.begin();

  openBookAndRestore();
  drawCurrentPage();
}

void readerLoop() {
  btnPrev.update();
  btnNext.update();

  if (btnPrev.longPress(ReaderLayout::LONG_MS)) {
    ReaderStorage::saveProgress(state);
    currentScreen = AppScreen::MainMenu;
    return;
  }

  if (btnPrev.released() && btnPrev.shortReleaseNotConsumed()) {
    if (state.historySize > 0) {
      state.currentOffset = state.history[--state.historySize];
      state.pageNum = max(1, state.pageNum - 1);
      drawCurrentPage();
    }
  }

  if (btnNext.released() && btnNext.shortReleaseNotConsumed()) {
    if (state.nextOffsetAfterCurrentPage > state.currentOffset) {
      if (state.historySize < ReaderState::MAX_HISTORY) {
        state.history[state.historySize++] = state.currentOffset;
      }

      state.currentOffset = state.nextOffsetAfterCurrentPage;
      state.pageNum++;
      drawCurrentPage();
    }
  }

  delay(5);
}