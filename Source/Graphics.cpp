#include "Graphics.h"
#include <vector>
#include <cmath>

namespace Graphics
{
    struct Shape
    {
        ShapeId id = 0;
        ShapeType type = ShapeType::Rect;

        // Transform (world space)
        float cx = 0, cy = 0;
        float sx = 1, sy = 1;

        // Color/alpha
        float r = 1, g = 1, b = 1, a = 1;

        // Circle settings
        int circleSegments = 32;

        // Sprite settings
        AEGfxTexture* tex = nullptr;
        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;

        bool alive = true;
    };

    static std::vector<Shape> g_Shapes;
    static ShapeId g_NextId = 1;
    static ShapeId g_SelectedId = 0;

    // Shared geometry (created lazily)
    static AEGfxVertexList* g_RectMesh = nullptr;

    struct CircleMesh
    {
        int segs = 0;
        AEGfxVertexList* mesh = nullptr;
    };
    static std::vector<CircleMesh> g_CircleMeshes;

    // We'll reuse ONE sprite mesh too (quad with UVs),
    // but since your rect mesh already has UVs, we’ll build a dedicated sprite quad
    static AEGfxVertexList* g_SpriteMesh = nullptr;

    // ---------------------------
    // Mesh helpers (internal)
    // ---------------------------

    static AEGfxVertexList* GetRectMesh()
    {
        if (g_RectMesh) return g_RectMesh;

        AEGfxMeshStart();

        // Rect (UV not used in RM_COLOR, but fine)
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, 0.5f, 0xFFFFFFFF, 0, 0);

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, 0.5f, 0xFFFFFFFF, 0, 0,
            -0.5f, 0.5f, 0xFFFFFFFF, 0, 0);

        g_RectMesh = AEGfxMeshEnd();
        return g_RectMesh;
    }

    static AEGfxVertexList* GetSpriteMesh()
    {
        if (g_SpriteMesh) return g_SpriteMesh;

        AEGfxMeshStart();

        // Unit quad with full UVs (we will offset/scale UV using AEGfxTextureSet's uvOffset)
        // Easiest: bake UVs as 0..1 and use AEGfxTextureSet with offsets 0,0.
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

        g_SpriteMesh = AEGfxMeshEnd();
        return g_SpriteMesh;
    }

    static AEGfxVertexList* GetCircleMesh(int segments)
    {
        if (segments < 3) segments = 3;

        for (auto& cm : g_CircleMeshes)
            if (cm.segs == segments)
                return cm.mesh;

        CircleMesh cm;
        cm.segs = segments;

        AEGfxMeshStart();
        const float step = 2.0f * PI / (float)segments;

        for (int i = 0; i < segments; ++i)
        {
            float a0 = step * (float)i;
            float a1 = step * (float)(i + 1);

            AEGfxTriAdd(
                0.0f, 0.0f, 0xFFFFFFFF, 0, 0,
                cosf(a0), sinf(a0), 0xFFFFFFFF, 0, 0,
                cosf(a1), sinf(a1), 0xFFFFFFFF, 0, 0
            );
        }

        cm.mesh = AEGfxMeshEnd();
        g_CircleMeshes.push_back(cm);
        return cm.mesh;
    }

    static Shape* Find(ShapeId id)
    {
        for (auto& s : g_Shapes)
            if (s.alive && s.id == id)
                return &s;
        return nullptr;
    }

    // ---------------------------
    // Public API: create shapes
    // ---------------------------

    ShapeId DrawRect(float cx, float cy, float w, float h,
        float r, float g, float b, float a)
    {
        Shape s;
        s.id = g_NextId++;
        s.type = ShapeType::Rect;
        s.cx = cx; s.cy = cy;
        s.sx = w;  s.sy = h;
        s.r = r; s.g = g; s.b = b; s.a = a;
        s.alive = true;

        g_Shapes.push_back(s);
        return s.id;
    }

    ShapeId DrawCircle(float cx, float cy, float radius, int segments,
        float r, float g, float b, float a)
    {
        Shape s;
        s.id = g_NextId++;
        s.type = ShapeType::Circle;
        s.cx = cx; s.cy = cy;
        s.sx = radius; s.sy = radius;
        s.circleSegments = segments;
        s.r = r; s.g = g; s.b = b; s.a = a;
        s.alive = true;

        g_Shapes.push_back(s);
        return s.id;
    }

    ShapeId DrawSprite(AEGfxTexture* tex,
        float cx, float cy, float w, float h,
        float r, float g, float b, float a,
        float u0, float v0, float u1, float v1)
    {
        Shape s;
        s.id = g_NextId++;
        s.type = ShapeType::Sprite;
        s.cx = cx; s.cy = cy;
        s.sx = w;  s.sy = h;
        s.r = r; s.g = g; s.b = b; s.a = a;
        s.tex = tex;
        s.u0 = u0; s.v0 = v0; s.u1 = u1; s.v1 = v1;
        s.alive = true;

        g_Shapes.push_back(s);
        return s.id;
    }

    // ---------------------------
    // Modify later
    // ---------------------------

    void SetPosition(ShapeId id, float cx, float cy)
    {
        if (auto* s = Find(id)) { s->cx = cx; s->cy = cy; }
    }

    void SetScale(ShapeId id, float sx, float sy)
    {
        if (auto* s = Find(id)) { s->sx = sx; s->sy = sy; }
    }

    void SetColor(ShapeId id, float r, float g, float b, float a)
    {
        if (auto* s = Find(id)) { s->r = r; s->g = g; s->b = b; s->a = a; }
    }

    void SetUV(ShapeId id, float u0, float v0, float u1, float v1)
    {
        if (auto* s = Find(id))
        {
            s->u0 = u0; s->v0 = v0; s->u1 = u1; s->v1 = v1;
        }
    }

    void Destroy(ShapeId id)
    {
        if (auto* s = Find(id)) s->alive = false;
        if (g_SelectedId == id) g_SelectedId = 0;
    }

    // ---------------------------
    // Selection / deletion
    // ---------------------------

    static void ScreenToWorld(int mx, int my, float& wx, float& wy)
    {
        float screenW = (float)AEGfxGetWindowWidth();
        float screenH = (float)AEGfxGetWindowHeight();
        wx = (float)mx - screenW * 0.5f;
        wy = screenH * 0.5f - (float)my;
    }

    static bool HitTest(const Shape& s, float wx, float wy)
    {
        if (!s.alive) return false;

        if (s.type == ShapeType::Circle)
        {
            float dx = wx - s.cx;
            float dy = wy - s.cy;
            float rr = s.sx; // radius stored in sx
            return (dx * dx + dy * dy) <= (rr * rr);
        }
        else
        {
            // Rect / Sprite AABB
            float hx = s.sx * 0.5f;
            float hy = s.sy * 0.5f;
            return (wx >= s.cx - hx && wx <= s.cx + hx &&
                wy >= s.cy - hy && wy <= s.cy + hy);
        }
    }

    ShapeId SelectAtScreen(int mouseX, int mouseY)
    {
        float wx, wy;
        ScreenToWorld(mouseX, mouseY, wx, wy);

        // Pick "topmost" = last created alive shape that hits
        for (int i = (int)g_Shapes.size() - 1; i >= 0; --i)
        {
            const Shape& s = g_Shapes[(size_t)i];
            if (HitTest(s, wx, wy))
            {
                g_SelectedId = s.id;
                return g_SelectedId;
            }
        }

        g_SelectedId = 0;
        return 0;
    }

    void ClearSelection() { g_SelectedId = 0; }
    ShapeId GetSelected() { return g_SelectedId; }

    void DeleteSelected()
    {
        if (g_SelectedId != 0)
            Destroy(g_SelectedId);
    }

    // ---------------------------
    // Render
    // ---------------------------

    void RenderAll()
    {
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToAdd(0, 0, 0, 0);

        for (const auto& s : g_Shapes)
        {
            if (!s.alive) continue;

            AEMtx33 scale, trans, m;
            AEMtx33Scale(&scale, s.sx, s.sy);
            AEMtx33Trans(&trans, s.cx, s.cy);
            AEMtx33Concat(&m, &trans, &scale);

            // Selected outline hint (optional): brighten selected
            float mul = (s.id == g_SelectedId) ? 1.0f : 1.0f;

            if (s.type == ShapeType::Sprite)
            {
                AEGfxVertexList* mesh = GetSpriteMesh();
                if (!mesh || !s.tex) continue;

                AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                AEGfxTextureSet(s.tex, 0, 0);

                // NOTE: AEEngine’s demo uses mesh UVs directly.
                // If you want sprite-sheet UV subrects, there are two common approaches:
                // 1) build a unique mesh per sprite with its UVs (more memory), OR
                // 2) if your AE build supports texture UV offset/scale, use it.
                //
                // Since we can't assume UV-offset API exists, we'll do approach (1) later if needed.
                // For now: u0..u1 are stored but unused unless you want per-sprite UV mesh.

                AEGfxSetTransparency(s.a);
                AEGfxSetColorToMultiply(s.r * mul, s.g * mul, s.b * mul, 1.0f);
                AEGfxSetTransform(m.m);
                AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
            }
            else if (s.type == ShapeType::Rect)
            {
                AEGfxVertexList* mesh = GetRectMesh();
                if (!mesh) continue;

                AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                AEGfxSetTransparency(s.a);
                AEGfxSetColorToMultiply(s.r * mul, s.g * mul, s.b * mul, 1.0f);
                AEGfxSetTransform(m.m);
                AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
            }
            else // Circle
            {
                AEGfxVertexList* mesh = GetCircleMesh(s.circleSegments);
                if (!mesh) continue;

                AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                AEGfxSetTransparency(s.a);
                AEGfxSetColorToMultiply(s.r * mul, s.g * mul, s.b * mul, 1.0f);
                AEGfxSetTransform(m.m);
                AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
            }
        }

        AEGfxSetTransparency(1.0f);
    }

    // ---------------------------
    // Cleanup
    // ---------------------------

    void Shutdown()
    {
        if (g_RectMesh) { AEGfxMeshFree(g_RectMesh); g_RectMesh = nullptr; }
        if (g_SpriteMesh) { AEGfxMeshFree(g_SpriteMesh); g_SpriteMesh = nullptr; }

        for (auto& cm : g_CircleMeshes)
        {
            if (cm.mesh) AEGfxMeshFree(cm.mesh);
            cm.mesh = nullptr;
        }
        g_CircleMeshes.clear();

        g_Shapes.clear();
        g_NextId = 1;
        g_SelectedId = 0;
    }
}