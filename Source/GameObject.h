#pragma once
#include "AEEngine.h"

// Declare functions you want to use
struct Color
{
    float r, g, b, a;
};

struct GameObject
{
    float x;
    float y;
    float _sizeX;
    float _sizeY;
    Color color;
    AEGfxVertexList* mesh;
    int segments;

    GameObject()
        : x(0.0f)
        , y(0.0f)
        , _sizeX(50.0f)
        , _sizeY(50.0f)
        , color{}
        , mesh(nullptr)
        , segments(64)
    {
    }

    void Init(float startX, float startY, float sX, float sY, Color c);
    void Update(float dt);
    void Draw();
    void Destroy();
};