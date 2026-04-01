#pragma once
#include "Scene.h"
#include <vector>
#include <string>

class Scene_Credits : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    int m_uiFont = -1;
    int m_page = 0;
    std::vector<std::vector<std::string>> m_pages;

    void NextPage();
    void PrevPage();
};