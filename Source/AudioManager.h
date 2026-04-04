#pragma once
#include "AEEngine.h"

class AudioManager
{
public:
    static void Init();
    static void Shutdown();

    static void PlayBGM(const char* filepath, float volume = 1.0f, float pitch = 1.0f, bool loop = true);
    static void StopBGM();
    static void PauseBGM();
    static void ResumeBGM();
    static void SetBGMVolume(float volume);

    static void PlaySFX(const char* filepath, float volume = 1.0f, float pitch = 1.0f);
    static void SetSFXVolume(float volume);

private:
    static bool s_initialized;

    static AEAudio      s_bgm;
    static AEAudioGroup s_bgmGroup;
    static bool         s_bgmPlaying;

    static float s_bgmVolume;
    static float s_sfxVolume;

    static AEAudioGroup s_sfxGroup;
};