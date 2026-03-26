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

    inline std::string GetSettingsPath()
    {
        return "../../Assets/settings.txt";
    }

    inline void Save()
    {
        std::string p = GetSettingsPath();
        FILE* f = nullptr;
        fopen_s(&f, p.c_str(), "w");
        if (!f) return;
        fprintf(f, "%d\n%d\n", resolutionIndex, masterVolume);
        fclose(f);
    }

    inline void Load()
    {
        std::string p = GetSettingsPath();
        FILE* f = nullptr;
        fopen_s(&f, p.c_str(), "r");
        if (!f) return;
        fscanf_s(f, "%d\n%d\n", &resolutionIndex, &masterVolume);
        fclose(f);

        if (resolutionIndex < 0 || resolutionIndex >= RESOLUTION_COUNT) resolutionIndex = 1;
        if (masterVolume < 0 || masterVolume > 100) masterVolume = 80;
    }
}