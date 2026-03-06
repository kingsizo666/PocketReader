#include <Arduino.h>
#include <LittleFS.h>
#include "reader_storage.h"
#include "appState.h"

namespace ReaderStorage
{

    String bookFilePath()
    {
        return currentBookPath + "/book.txt";
    }

    String progressFilePath()
    {
        return currentBookPath + "/progress.txt";
    }

    void saveProgress(const ReaderState &state)
    {
        File f = LittleFS.open(progressFilePath(), "w");
        if (!f)
        {
            Serial.println("saveProgress: FAILED to open progress file");
            return;
        }

        f.println((long)state.currentOffset);
        f.println(state.pageNum);
        f.close();

        Serial.printf("saveProgress: path=%s offset=%ld page=%d\n",
                      progressFilePath().c_str(),
                      (long)state.currentOffset,
                      state.pageNum);
    }

    void loadProgress(ReaderState &state)
    {
        File f = LittleFS.open(progressFilePath(), "r");
        if (!f)
        {
            state.currentOffset = 0;
            state.pageNum = 1;
            Serial.printf("loadProgress: no file at %s, starting from page 1\n",
                          progressFilePath().c_str());
            return;
        }

        String offsetLine = f.readStringUntil('\n');
        String pageLine = f.readStringUntil('\n');
        f.close();

        state.currentOffset = offsetLine.toInt();
        state.pageNum = pageLine.toInt();

        if (state.currentOffset < 0)
            state.currentOffset = 0;
        if (state.pageNum < 1)
            state.pageNum = 1;

        Serial.printf("loadProgress: path=%s offset=%ld page=%d\n",
                      progressFilePath().c_str(),
                      (long)state.currentOffset,
                      state.pageNum);
    }

}