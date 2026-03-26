#pragma once
#include <cstdio>
#include <string>

namespace GameSettings
{
    enum class Difficulty { Easy, Hard };
    extern Difficulty currentDifficulty;
    struct Resolution { int width; int height; const char* label; };

    extern const Resolution RESOLUTIONS[3];
    extern const int        RESOLUTION_COUNT;

    extern int  resolutionIndex;
    extern int  masterVolume;
    extern bool pendingRestart;
    extern bool quitGame;

    extern std::string selectedLevelFile;

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

        if (resolutionIndex < 0 || resolutionIndex >= RESOLUTION_COUNT) resolutionIndex = 1;
        if (masterVolume < 0 || masterVolume > 100) masterVolume = 80;
    }
}