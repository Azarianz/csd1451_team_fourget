#pragma once
#include "Scene.h"
#include "AEEngine.h"
#include <vector>
#include <string>

class Scene_Controls : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    int m_uiFont = -1;
    int m_currentSlide = 0;
    std::vector<std::string> m_slidePaths;
    std::vector<AEGfxTexture*> m_slides;

    void NextSlide();
    void PrevSlide();
};