#include <Arduino.h>
#include <LittleFS.h>
#include "reader_paginator.h"
#include "reader_state.h"
#include "display_shared.h"
#include <Fonts/FreeSans9pt7b.h>

using namespace ReaderLayout;

namespace
{

    int textWidthPx(const String &s)
    {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(s.c_str(), 0, 0, &x1, &y1, &w, &h);
        return (int)w;
    }

    void beginFreshPage()
    {
        display.clearMemory();
    }

    int32_t paginateFromOffset(const String &filePath, int32_t startOffset, bool draw, int pageNum)
    {
        File f = LittleFS.open(filePath, "r");
        if (!f)
        {
            if (draw)
            {
                beginFreshPage();
                display.setFont();
                display.setCursor(0, 16);
                display.print("[Missing book.txt]");
                display.setCursor(0, 32);
                display.print(filePath);
                display.update();
            }
            return startOffset;
        }

        if (!f.seek(startOffset))
        {
            startOffset = 0;
            f.seek(0);
        }

        if (draw)
        {
            beginFreshPage();

            display.setFont();
            display.setTextWrap(true);
            display.setCursor(MARGIN_X, HEADER_Y);
            display.print("Page ");
            display.print(pageNum);

            display.setFont(&FreeSans9pt7b);
            display.setTextWrap(false);
        }

        const int maxWidth = SCREEN_W - 2 * MARGIN_X - 2;
        const int lineH = (int)FreeSans9pt7b.yAdvance + 1;

        int y = TEXT_TOP_Y;
        String line;
        String word;

        int32_t lastBreakOffset = startOffset;
        int32_t wordStartOffset = startOffset;

        auto pageFull = [&]() -> bool
        {
            return y > (BOTTOM_Y - lineH);
        };

        auto flushLine = [&]()
        {
            if (line.isEmpty())
                return;
            if (draw)
            {
                display.setCursor(MARGIN_X, y);
                display.print(line);
            }
            y += lineH;
            line = "";
        };

        auto commitWord = [&](int32_t breakPos) -> int32_t
        {
            if (word.isEmpty())
                return -1;

            String candidate = line.length() ? (line + " " + word) : word;

            if (textWidthPx(candidate) <= maxWidth)
            {
                line = candidate;
                word = "";
                lastBreakOffset = breakPos;
                return -1;
            }

            flushLine();

            if (pageFull())
            {
                return wordStartOffset;
            }

            if (textWidthPx(word) > maxWidth)
            {
                String chunk;
                for (size_t i = 0; i < word.length(); i++)
                {
                    String cand = chunk + word[i];
                    if (textWidthPx(cand) <= maxWidth)
                    {
                        chunk = cand;
                    }
                    else
                    {
                        line = chunk;
                        flushLine();

                        if (pageFull())
                        {
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

        while (f.available())
        {
            if (pageFull())
                break;

            int c = f.read();
            int32_t pos = (int32_t)f.position();

            if (c == '\r')
                continue;

            if (c == ' ' || c == '\t' || c == '\n')
            {
                int32_t stopAt = commitWord(pos);
                if (stopAt >= 0)
                {
                    if (draw)
                    {
                        display.setFont();
                        display.setTextWrap(true);
                        display.update();
                    }
                    f.close();
                    return stopAt;
                }

                if (c == '\n')
                {
                    flushLine();
                    if (pageFull())
                    {
                        if (draw)
                        {
                            display.setFont();
                            display.setTextWrap(true);
                            display.update();
                        }
                        f.close();
                        return pos;
                    }
                    lastBreakOffset = pos;
                }
                else
                {
                    lastBreakOffset = pos;
                }
            }
            else
            {
                if (word.isEmpty())
                {
                    wordStartOffset = pos - 1;
                }
                word += (char)c;
            }
        }

        if (!pageFull())
        {
            int32_t stopAt = commitWord((int32_t)f.position());
            if (stopAt >= 0)
            {
                if (draw)
                {
                    display.setFont();
                    display.setTextWrap(true);
                    display.update();
                }
                f.close();
                return stopAt;
            }
            flushLine();
        }

        if (draw)
        {
            display.setFont();
            display.setTextWrap(true);
            display.update();
        }

        f.close();

        int32_t nextOffset = lastBreakOffset;
        if (nextOffset <= startOffset)
            nextOffset = startOffset + 1;
        return nextOffset;
    }

} // namespace

namespace ReaderPaginator
{

    int32_t renderPageFromOffset(const String &filePath, int32_t startOffset, int pageNum)
    {
        return paginateFromOffset(filePath, startOffset, true, pageNum);
    }

    int32_t computeNextOffsetFrom(const String &filePath, int32_t startOffset)
    {
        return paginateFromOffset(filePath, startOffset, false, 0);
    }

    void rebuildHistoryToCurrentPage(ReaderState &state, const String &filePath)
    {
        state.historySize = 0;

        if (state.currentOffset <= 0 || state.pageNum <= 1)
        {
            return;
        }

        int32_t offset = 0;
        int page = 1;

        while (page < state.pageNum && state.historySize < ReaderState::MAX_HISTORY)
        {
            int32_t nextOffset = computeNextOffsetFrom(filePath, offset);
            if (nextOffset <= offset)
                break;

            state.history[state.historySize++] = offset;
            offset = nextOffset;
            page++;
        }
    }

}