#pragma once

#include "AEEngine.h"
#include <string>
#include <vector>

class TutorialPopup
{
public:
    void Init();
    void Shutdown();

    void SetEnabled(bool enabled);
    void SetSlides(const std::vector<std::string>& imagePaths);

    void Start();
    void ForceStart();
    void Close();
    void Reset();

    void Update();
    void Draw(int fontId) const;

    bool IsActive() const;
    bool HasStarted() const;
    bool IsFinished() const;

private:
    void LoadSlides();
    void UnloadSlides();

private:
    bool m_enabled = false;
    bool m_active = false;
    bool m_startedOnce = false;
    bool m_finished = false;

    int m_currentSlide = 0;

    std::vector<std::string> m_slidePaths;
    std::vector<AEGfxTexture*> m_slideTextures;

    AEGfxVertexList* m_imageQuad = nullptr;
};