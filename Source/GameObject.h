#pragma once
#include "AEEngine.h"

// Declare functions you want to use
struct Color
{
    float r, g, b, a;
};

struct GameObject
{
    float x, y;
    float _sizeX = 50.0f, _sizeY = 50.0f;
    Color color{};

    AEGfxVertexList* mesh = nullptr;
    int segments;   //for circle

    void Init(float startX, float startY, float sX, float sY, Color c);
    void Update(float dt);
    void Draw();
    void Destroy();
};