#pragma once
#include "AEEngine.h"
#include <cstdint>

namespace Graphics
{
    using ShapeId = uint32_t;

    enum class ShapeType : uint8_t
    {
        Rect,
        Circle,
        Sprite
    };

    // Create retained shapes (they persist until Destroy/DeleteSelected)
    ShapeId DrawRect(float cx, float cy, float w, float h,
        float r, float g, float b, float a);

    ShapeId DrawCircle(float cx, float cy, float radius, int segments,
        float r, float g, float b, float a);

    // NEW: Sprite (w/h = size in world pixels; UV defaults full texture)
    ShapeId DrawSprite(AEGfxTexture* tex,
        float cx, float cy, float w, float h,
        float r = 1, float g = 1, float b = 1, float a = 1,
        float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f, float v1 = 1.0f);

    // Modify later
    void SetPosition(ShapeId id, float cx, float cy);
    void SetScale(ShapeId id, float sx, float sy);
    void SetColor(ShapeId id, float r, float g, float b, float a);
    void SetUV(ShapeId id, float u0, float v0, float u1, float v1); // for sprite sheets
    void Destroy(ShapeId id);

    // Selection (single)
    ShapeId SelectAtScreen(int mouseX, int mouseY); // sets selected internally, returns id or 0
    void ClearSelection();
    ShapeId GetSelected();
    void DeleteSelected(); // destroys currently selected (if any)

    // Render all
    void RenderAll();

    // Cleanup
    void Shutdown();
}