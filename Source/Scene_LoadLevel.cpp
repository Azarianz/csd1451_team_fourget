#include "Scene_LoadLevel.h"
#include "AEEngine.h"
#include "AEInput.h"

void Scene_LoadLevel::Init()
{
    if (!level.LoadFromText(levelPath))
    {
        PRINT("FAILED to load level text: %s\n", levelPath);
        return;
    }

    grid = new GridSystem::Grid(level.width, level.height, 1.0f);
    grid->Init();

    occupied.assign((size_t)level.width * level.height, 0);

    BuildXMeshIfNeeded();
}

void Scene_LoadLevel::Update(float /*dt*/)
{
    if (!grid) return;

    // Toggle build mode
    if (AEInputCheckTriggered(AEVK_B))
        buildMode = !buildMode;

    if (!buildMode) return;

    // Optional: click to place (demo)
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);

    GridSystem::GridCoord c;
    if (grid->ScreenToGrid(mx, my, c))
    {
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            if (IsPlaceable(c.x, c.y))
            {
                // Mark occupied (spawn tower here in your system)
                occupied[Idx(c.x, c.y)] = 1;
            }
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

    // Two thin diagonal rectangles would look nicer, but simplest is 2 lines made from skinny triangles.
    // We'll approximate with two skinny triangles per diagonal (total 4 tris).

    const float half = 0.45f;     // X size inside a cell (in local mesh space)
    const float thick = 0.06f;    // thickness

    auto AddQuad = [](float x0, float y0, float x1, float y1, float t)
        {
            // build a quad around a line segment (x0,y0)->(x1,y1) with thickness t
            float dx = x1 - x0, dy = y1 - y0;
            float len = sqrtf(dx * dx + dy * dy);
            if (len <= 0.0001f) return;

            dx /= len; dy /= len;
            // perpendicular
            float px = -dy * t;
            float py = dx * t;

            // quad points
            float ax = x0 + px, ay = y0 + py;
            float bx = x0 - px, by = y0 - py;
            float cx = x1 - px, cy = y1 - py;
            float dx2 = x1 + px, dy2 = y1 + py;

            AEGfxTriAdd(ax, ay, 0xFFFFFFFF, 0, 0, bx, by, 0xFFFFFFFF, 0, 0, cx, cy, 0xFFFFFFFF, 0, 0);
            AEGfxTriAdd(ax, ay, 0xFFFFFFFF, 0, 0, cx, cy, 0xFFFFFFFF, 0, 0, dx2, dy2, 0xFFFFFFFF, 0, 0);
        };

    AEGfxMeshStart();
    // Diagonal 1: bottom-left to top-right
    AddQuad(-half, -half, half, half, thick);
    // Diagonal 2: top-left to bottom-right
    AddQuad(-half, half, half, -half, thick);
    xMesh = AEGfxMeshEnd();
}

void Scene_LoadLevel::DrawXAtCell(int x, int y, float alpha)
{
    if (!xMesh) return;

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
    // show grid while building
    grid->Draw();

    // draw X on every non-placeable cell
    for (int y = 0; y < level.height; ++y)
    {
        for (int x = 0; x < level.width; ++x)
        {
            if (!IsPlaceable(x, y))
                DrawXAtCell(x, y, 0.75f);
        }
    }

    // hover feedback
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);

    GridSystem::GridCoord c;
    if (grid->ScreenToGrid(mx, my, c))
    {
        if (IsPlaceable(c.x, c.y))
            grid->DrawTileTinted(c, 0.2f, 1.0f, 0.2f, 0.55f); // green-ish
        else
            grid->DrawTileTinted(c, 1.0f, 0.2f, 0.2f, 0.55f); // red-ish
    }
}