#include "Graphics.h"
#include <cmath>

namespace Graphics
{
    static AEGfxVertexList* s_RectMesh = nullptr;
    static AEGfxVertexList* s_CircleMesh = nullptr;
    static int s_CircleSegments = 0;

    // =========================
    // Init / Shutdown
    // =========================

    void Init()
    {
        // ---------- Rectangle mesh ----------
        AEGfxMeshStart();

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, 0.5f, 0xFFFFFFFF, 0, 0);

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, 0.5f, 0xFFFFFFFF, 0, 0,
            -0.5f, 0.5f, 0xFFFFFFFF, 0, 0);

        s_RectMesh = AEGfxMeshEnd();

        // Circle mesh will be built lazily
        s_CircleMesh = nullptr;
        s_CircleSegments = 0;
    }

    void Shutdown()
    {
        if (s_RectMesh)
        {
            AEGfxMeshFree(s_RectMesh);
            s_RectMesh = nullptr;
        }

        if (s_CircleMesh)
        {
            AEGfxMeshFree(s_CircleMesh);
            s_CircleMesh = nullptr;
        }
    }

    // =========================
    // Rect
    // =========================

    void DrawRect(float cx, float cy, float w, float h,
        float r, float g, float b, float a)
    {
        if (!s_RectMesh) return;

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(a);
        AEGfxSetColorToMultiply(r, g, b, 1.0f);
        AEGfxSetColorToAdd(0, 0, 0, 0);

        AEMtx33 scale, trans, m;
        AEMtx33Scale(&scale, w, h);
        AEMtx33Trans(&trans, cx, cy);
        AEMtx33Concat(&m, &trans, &scale);

        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(s_RectMesh, AE_GFX_MDM_TRIANGLES);

        AEGfxSetTransparency(1.0f);
    }

    // =========================
    // Circle
    // =========================

    static void BuildCircleMesh(int segments)
    {
        if (s_CircleMesh)
            AEGfxMeshFree(s_CircleMesh);

        AEGfxMeshStart();

        const float step = 2.0f * PI / segments;

        for (int i = 0; i < segments; ++i)
        {
            float a0 = step * i;
            float a1 = step * (i + 1);

            AEGfxTriAdd(
                0.0f, 0.0f, 0xFFFFFFFF, 0, 0,
                cosf(a0), sinf(a0), 0xFFFFFFFF, 0, 0,
                cosf(a1), sinf(a1), 0xFFFFFFFF, 0, 0
            );
        }

        s_CircleMesh = AEGfxMeshEnd();
        s_CircleSegments = segments;
    }

    void DrawCircle(float cx, float cy, float radius,
        int segments,
        float r, float g, float b, float a)
    {
        if (segments < 3) segments = 3;

        if (!s_CircleMesh || s_CircleSegments != segments)
            BuildCircleMesh(segments);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(a);
        AEGfxSetColorToMultiply(r, g, b, 1.0f);
        AEGfxSetColorToAdd(0, 0, 0, 0);

        AEMtx33 scale, trans, m;
        AEMtx33Scale(&scale, radius, radius);
        AEMtx33Trans(&trans, cx, cy);
        AEMtx33Concat(&m, &trans, &scale);

        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(s_CircleMesh, AE_GFX_MDM_TRIANGLES);

        AEGfxSetTransparency(1.0f);
    }
}