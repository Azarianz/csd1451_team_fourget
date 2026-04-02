#pragma once
#include "Scene.h"
#include "TutorialPopup.h"

class Scene_Controls : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    s8 m_uiFont = -1;
    TutorialPopup m_tutorialPopup;
};