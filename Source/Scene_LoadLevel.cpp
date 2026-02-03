#include "Scene_LoadLevel.h"
#include "AEEngine.h"
#include "AEInput.h"
#include <cstdio>  // sprintf_s
#include <cmath>   // sqrtf

bool Scene_LoadLevel::LoadLevel(int idx)
{
    // Build file path: Assets/Levels/level_01.txt, level_02.txt, ...
    // (adjust naming here if your files are Level_01.txt etc.)
    sprintf_s(levelPath, "Assets/Levels/Level_%02d.txt", idx);

    // Clean previous state if any
    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }

    level.Shutdown();

    if (!level.LoadFromText(levelPath))
    {
        PRINT("FAILED to load level text: %s\n", levelPath);
        return false;
    }

    grid = new GridSystem::Grid(level.width, level.height, 1.0f);
    grid->Init();

    occupied.assign((size_t)level.width * level.height, 0);

    BuildXMeshIfNeeded();
    return true;
}

void Scene_LoadLevel::Init()
{
    LoadLevel(levelIndex);
}

void Scene_LoadLevel::Update(float /*dt*/)
{
    if (!grid) return;

    // Toggle build mode
    if (AEInputCheckTriggered(AEVK_B))
        buildMode = !buildMode;

    // Hot reload current level while testing
    if (AEInputCheckTriggered(AEVK_F5))
        LoadLevel(levelIndex);

    if (!buildMode) return;

    // Demo placement
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);

    GridSystem::GridCoord c;
    if (grid->ScreenToGrid(mx, my, c))
    {
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            if (IsPlaceable(c.x, c.y))
                occupied[Idx(c.x, c.y)] = 1;
        }
    }
}

void Scene_LoadLevel::Draw()
{
    if (!grid) return;

    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    // Draw the map
    level.Draw(*grid, 1.0f);

    // Build overlay
    if (buildMode)
        DrawBuildOverlay();
}

void Scene_LoadLevel::Exit()
{
    if (xMesh)
    {
        AEGfxMeshFree(xMesh);
        xMesh = nullptr;
    }

    level.Shutdown();

    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }
}

// --------------------------
// Overlay helpers
// --------------------------

void Scene_LoadLevel::BuildXMeshIfNeeded()
{
    if (xMesh) return;

    const float half = 0.45f;
    const float thick = 0.06f;

    auto AddQuad = [](float x0, float y0, float x1, float y1, float t)
        {
            float dx = x1 - x0, dy = y1 - y0;
            float len = sqrtf(dx * dx + dy * dy);
            if (len <= 0.0001f) return;

            dx /= len; dy /= len;
            float px = -dy * t;
            float py = dx * t;

            float ax = x0 + px, ay = y0 + py;
            float bx = x0 - px, by = y0 - py;
            float cx = x1 - px, cy = y1 - py;
            float dx2 = x1 + px, dy2 = y1 + py;

            AEGfxTriAdd(ax, ay, 0xFFFFFFFF, 0, 0, bx, by, 0xFFFFFFFF, 0, 0, cx, cy, 0xFFFFFFFF, 0, 0);
            AEGfxTriAdd(ax, ay, 0xFFFFFFFF, 0, 0, cx, cy, 0xFFFFFFFF, 0, 0, dx2, dy2, 0xFFFFFFFF, 0, 0);
        };

    AEGfxMeshStart();
    AddQuad(-half, -half, half, half, thick);
    AddQuad(-half, half, half, -half, thick);
    xMesh = AEGfxMeshEnd();
}

void Scene_LoadLevel::DrawXAtCell(int x, int y, float alpha)
{
    if (!xMesh || !grid) return;

    float wx, wy, tileSize;
    grid->GetCellWorldCenter({ x, y }, wx, wy, tileSize);

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToAdd(0, 0, 0, 0);
    AEGfxSetColorToMultiply(1, 1, 1, 1);
    AEGfxSetTransparency(alpha);

    AEMtx33 scale, trans, m;
    AEMtx33Scale(&scale, tileSize, tileSize);
    AEMtx33Trans(&trans, wx, wy);
    AEMtx33Concat(&m, &trans, &scale);

    AEGfxSetTransform(m.m);
    AEGfxMeshDraw(xMesh, AE_GFX_MDM_TRIANGLES);

    AEGfxSetTransparency(1.0f);
}

void Scene_LoadLevel::DrawBuildOverlay()
{
    if (!grid) return;

    grid->Draw();

    for (int y = 0; y < level.height; ++y)
    {
        for (int x = 0; x < level.width; ++x)
        {
            if (!IsPlaceable(x, y))
                DrawXAtCell(x, y, 0.75f);
        }
    }

    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);

    GridSystem::GridCoord c;
    if (grid->ScreenToGrid(mx, my, c))
    {
        if (IsPlaceable(c.x, c.y))
            grid->DrawTileTinted(c, 0.2f, 1.0f, 0.2f, 0.55f);
        else
            grid->DrawTileTinted(c, 1.0f, 0.2f, 0.2f, 0.55f);
    }
}