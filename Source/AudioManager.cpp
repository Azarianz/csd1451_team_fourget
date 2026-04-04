#include "AudioManager.h"

bool AudioManager::s_initialized = false;

AEAudio      AudioManager::s_bgm = {};
AEAudioGroup AudioManager::s_bgmGroup = {};
bool         AudioManager::s_bgmPlaying = false;

float AudioManager::s_bgmVolume = 1.0f;
float AudioManager::s_sfxVolume = 1.0f;

AEAudioGroup AudioManager::s_sfxGroup = {};

void AudioManager::Init()
{
    if (s_initialized)
        return;

    s_bgmGroup = AEAudioCreateGroup();
    s_sfxGroup = AEAudioCreateGroup();
    s_initialized = true;
}

void AudioManager::Shutdown()
{
    if (!s_initialized)
        return;

    if (s_bgmPlaying)
    {
        AEAudioStopGroup(s_bgmGroup);
        s_bgmPlaying = false;
    }

    s_initialized = false;
}

void AudioManager::PlayBGM(const char* filepath, float volume, float pitch, bool loop)
{
    if (!s_initialized || !filepath)
        return;

    s_bgmVolume = volume;

    if (s_bgmPlaying)
    {
        AEAudioStopGroup(s_bgmGroup);
        s_bgmPlaying = false;
    }

    s_bgm = AEAudioLoadMusic(filepath);
    //PRINT("PlayBGM called: %s  volume=%f\n", filepath, s_bgmVolume);

    AEAudioPlay(s_bgm, s_bgmGroup, s_bgmVolume, pitch, loop ? -1 : 0);
    AEAudioSetGroupVolume(s_bgmGroup, s_bgmVolume);

    s_bgmPlaying = true;
}

void AudioManager::StopBGM()
{
    if (!s_initialized || !s_bgmPlaying)
        return;

    AEAudioStopGroup(s_bgmGroup);
    s_bgmPlaying = false;
}

void AudioManager::PauseBGM()
{
    if (!s_initialized || !s_bgmPlaying)
        return;

    AEAudioPauseGroup(s_bgmGroup);
}

void AudioManager::ResumeBGM()
{
    if (!s_initialized || !s_bgmPlaying)
        return;

    AEAudioResumeGroup(s_bgmGroup);
}

void AudioManager::SetBGMVolume(float volume)
{
    if (!s_initialized)
        return;

    s_bgmVolume = volume;
    AEAudioSetGroupVolume(s_bgmGroup, s_bgmVolume);
}

void AudioManager::PlaySFX(const char* filepath, float volume, float pitch)
{
    if (!s_initialized || !filepath)
        return;

    AEAudio sfx = AEAudioLoadSound(filepath);
    AEAudioPlay(sfx, s_sfxGroup, volume * s_sfxVolume, pitch, 0);
}

void AudioManager::SetSFXVolume(float volume)
{
    if (!s_initialized)
        return;

    s_sfxVolume = volume;
    AEAudioSetGroupVolume(s_sfxGroup, s_sfxVolume);
}