#include "GameSettings.h"

namespace GameSettings
{
    Difficulty currentDifficulty = Difficulty::Easy;
    const Resolution RESOLUTIONS[3] = {
        { 1280, 720,  "1280 x 720"  },
        { 1600, 900,  "1600 x 900"  },
        { 1920, 1080, "1920 x 1080" },
    };
    const int RESOLUTION_COUNT = 3;

    int  resolutionIndex = 1;
    int  masterVolume = 80;
    bool pendingRestart = false;
    bool quitGame = false;

    std::string selectedLevelFile = "";
}