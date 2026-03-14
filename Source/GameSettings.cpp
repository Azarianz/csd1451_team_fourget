#include "GameSettings.h"

namespace GameSettings
{
    const Resolution RESOLUTIONS[3] = {
        { 1280, 720,  "1280 x 720"  },
        { 1600, 900,  "1600 x 900"  },
        { 1920, 1080, "1920 x 1080" },
    };
    const int RESOLUTION_COUNT = 3;

    int  resolutionIndex = 1;    // default: 1600x900
    int  masterVolume = 80;   // 0-100
    bool pendingRestart = false;
    bool quitGame = false;
}