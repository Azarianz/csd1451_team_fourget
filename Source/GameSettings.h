#pragma once
#include <cstdio>

namespace GameSettings
{
    struct Resolution { int width; int height; const char* label; };

    extern const Resolution RESOLUTIONS[3];
    extern const int        RESOLUTION_COUNT;

    extern int  resolutionIndex;
    extern int  masterVolume;
    extern bool pendingRestart;
    extern bool quitGame;

    inline void Save()
    {
        FILE* f = nullptr;
        fopen_s(&f, "Assets/settings.txt", "w");
        if (!f) return;
        fprintf(f, "%d\n%d\n", resolutionIndex, masterVolume);
        fclose(f);
    }

    inline void Load()
    {
        FILE* f = nullptr;
        fopen_s(&f, "Assets/settings.txt", "r");
        if (!f) return;
        fscanf_s(f, "%d\n%d\n", &resolutionIndex, &masterVolume);
        fclose(f);
        // clamp in case file is corrupt
        if (resolutionIndex < 0 || resolutionIndex >= RESOLUTION_COUNT) resolutionIndex = 1;
        if (masterVolume < 0 || masterVolume > 100) masterVolume = 80;
    }
}