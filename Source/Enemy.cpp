// Enemy.cpp
#include "Enemy.h"
#include "AEInput.h"
#include "AEMath.h"
#include <cmath> // For sqrtf

// --- Sprite Sheet Cache ---
static AEGfxTexture* g_EnemySpriteSheet = nullptr;
static const int SHEET_COLS = 13;
static const int SHEET_ROWS = 10;
static AEGfxVertexList* g_FrameMeshes[SHEET_ROWS][SHEET_COLS] = { nullptr };

// Helper to load the sheet and build UV meshes
static AEGfxVertexList* GetEnemyFrameMesh(int col, int row)
{
    if (!g_EnemySpriteSheet) {
        g_EnemySpriteSheet = AEGfxTextureLoad("Assets/spritesheet.png");
    }

    if (g_FrameMeshes[row][col]) return g_FrameMeshes[row][col];

    int ty = (SHEET_ROWS - 1) - row;
    float u0 = (float)col / (float)SHEET_COLS;
    float u1 = (float)(col + 1) / (float)SHEET_COLS;
    float v0 = (float)ty / (float)SHEET_ROWS;
    float v1 = (float)(ty + 1) / (float)SHEET_ROWS;

    AEGfxMeshStart();
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, u0, v1,
        0.5f, -0.5f, 0xFFFFFFFF, u1, v1,
        0.5f, 0.5f, 0xFFFFFFFF, u1, v0
    );
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, u0, v1,
        0.5f, 0.5f, 0xFFFFFFFF, u1, v0,
        -0.5f, 0.5f, 0xFFFFFFFF, u0, v0
    );

    g_FrameMeshes[row][col] = AEGfxMeshEnd();
    return g_FrameMeshes[row][col];
}

void Enemy::Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed)
{
    GameObject::Init(0.0f, 0.0f, sizeX, sizeY, c);
    maxhealth = _hp;
    health = _hp;
    damage = _damage;
    speed = _speed;
    pathIndex = 0;
    reachedEnd = false;
}

void Enemy::SetSprite(int row, int col)
{
    spriteRow = row;
    spriteCol = col;
}

void Enemy::Update(float dt, const std::vector<Point>& path)
{
    if (path.empty() || reachedEnd) return;
    if (pathIndex >= (int)path.size()) {
        reachedEnd = true;
        return;
    }

    Point target = path[pathIndex];
    float dx = target.x - x;
    float dy = target.y - y;
    float distSq = dx * dx + dy * dy;

    if (distSq > 25.0f)
    {
        float dist = sqrtf(distSq);
        x += (dx / dist) * speed * dt;
        y += (dy / dist) * speed * dt;
    }
    else
    {
        pathIndex++;
        if (pathIndex >= (int)path.size()) {
            reachedEnd = true;
        }
    }
}

void Enemy::Draw()
{
    AEGfxVertexList* mesh = GetEnemyFrameMesh(spriteCol, spriteRow);
    if (!mesh || !g_EnemySpriteSheet) return;

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(g_EnemySpriteSheet, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(color.a);
    AEGfxSetColorToMultiply(color.r, color.g, color.b, 1.0f);

    AEMtx33 scaleM, rotM, transM, transform;
    AEMtx33Scale(&scaleM, _sizeX, _sizeY);
    AEMtx33Rot(&rotM, 0.0f);
    AEMtx33Trans(&transM, x, y);

    AEMtx33Concat(&transform, &rotM, &scaleM);
    AEMtx33Concat(&transform, &transM, &transform);

    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
}

// --- Specific Enemies ---

void Zombie::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 100.0f, 10.0f, 150.0f);
    SetSprite(1, 11);
}

void Skeleton::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 60.0f, 5.0f, 250.0f);
    SetSprite(1, 5);
}

void Troll::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 300.0f, 15.0f, 100.0f);
    SetSprite(1, 3);
}

void Golem::Init()
{
    Enemy::Init(60.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 300.0f, 40.0f, 50.0f);
    SetSprite(1, 4);
}

void Titan::Init()
{
    Enemy::Init(50.0f, 50.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 250.0f, 30.0f, 75.0f);
    SetSprite(1, 7);
}