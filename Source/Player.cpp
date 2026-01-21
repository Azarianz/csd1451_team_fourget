#include "Player.h"
#include "AEInput.h"
#include "AEMath.h"

// Build mesh ONCE
static AEGfxVertexList* BuildCircle(int segments)
{
    AEGfxMeshStart();

    float cx = 0.0f, cy = 0.0f;

    for (int i = 0; i < segments; ++i)
    {
        float a0 = TWO_PI * (float)i / segments;
        float a1 = TWO_PI * (float)(i + 1) / segments;

        float x0 = cosf(a0);
        float y0 = sinf(a0);
        float x1 = cosf(a1);
        float y1 = sinf(a1);

        unsigned int col = 0xFFFFFFFF;

        float uC = 0.5f, vC = 0.5f;
        float u0 = 0.5f + 0.5f * x0;
        float v0 = 0.5f - 0.5f * y0;
        float u1 = 0.5f + 0.5f * x1;
        float v1 = 0.5f - 0.5f * y1;

        AEGfxTriAdd(
            cx, cy, col, uC, vC,
            x0, y0, col, u0, v0,
            x1, y1, col, u1, v1
        );
    }

    return AEGfxMeshEnd();
}

void Player::Init(float startX, float startY, float sX, float sY, Color c)
{
    x = startX;
    y = startY;
    _sizeX = sX;
    _sizeY = sY;
    speed = 400.0f;
    segments = 64;
    color = c;
    mesh = nullptr; // don't build here
}

void Player::Update(float dt)
{
    if (AEInputCheckCurr(AEVK_W)) y += speed * dt;
    if (AEInputCheckCurr(AEVK_S)) y -= speed * dt;
    if (AEInputCheckCurr(AEVK_A)) x -= speed * dt;
    if (AEInputCheckCurr(AEVK_D)) x += speed * dt;
}