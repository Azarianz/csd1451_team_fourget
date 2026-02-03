#include "GameObject.h"
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

void GameObject::Init(float startX, float startY, float sX, float sY, Color c)
{
    x = startX;
    y = startY;
	_sizeX = sX;
	_sizeY = sY;
    segments = 64;
    color = c;
    mesh = nullptr; // don't build here
}

void GameObject::Update(float dt)
{
}

void GameObject::Draw()
{
    if (!mesh)
        mesh = BuildCircle(segments); // build during frame (safe)

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);   // 🔴 THIS WAS MISSING
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // Set color using struct
    AEGfxSetColorToMultiply(
        color.r,
        color.g,
        color.b,
        color.a
    );


    // --- TRANSFORM ---
    AEMtx33 scaleM, rotM, transM, transform;
    AEMtx33Scale(&scaleM, _sizeX, _sizeY);
    AEMtx33Rot(&rotM, 0.0f);
    AEMtx33Trans(&transM, x, y);

    AEMtx33Concat(&transform, &rotM, &scaleM);
    AEMtx33Concat(&transform, &transM, &transform);

    AEGfxSetTransform(transform.m);

    // --- DRAW ---
    AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);

}

void GameObject::Destroy()
{
    if (mesh)
    {
        AEGfxMeshFree(mesh);
        mesh = nullptr;
    }
}

