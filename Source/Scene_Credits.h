#pragma once
#include "Scene.h"
#include "AEEngine.h"
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
    struct ButtonRect
    {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };

    void NextPage();
    void PrevPage();
    void DrawImagePage();
    void DrawTextPage();
    void DrawNavButtons() const;
    void UpdateMouseInput();

    bool IsPointInRect(float mx, float my, float x, float y, float w, float h) const;

    ButtonRect GetLeftArrowRect() const;
    ButtonRect GetRightArrowRect() const;

    int m_uiFont = -1;
    int m_page = 0;

    std::vector<std::vector<std::string>> m_pages;

    AEGfxVertexList* m_firstPageMesh = nullptr;
    AEGfxTexture* m_firstPageTex = nullptr;
};