#pragma once
#include "AEEngine.h"

namespace Graphics
{
    // Must be called once at startup
    void Init();

    // Must be called once at shutdown
    void Shutdown();

    // =========================
    // Primitive Drawing
    // =========================

    // Filled rectangle (world-space, centered)
    void DrawRect(
        float cx, float cy,
        float width, float height,
        float r, float g, float b, float a = 1.0f
    );

    // Square helper
    inline void DrawSquare(
        float cx, float cy,
        float size,
        float r, float g, float b, float a = 1.0f
    )
    {
        DrawRect(cx, cy, size, size, r, g, b, a);
    }

    // Filled circle (triangle fan approximation)
    void DrawCircle(
        float cx, float cy,
        float radius,
        int segments,
        float r, float g, float b, float a = 1.0f
    );
}