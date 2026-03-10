// Enemy.cpp
#include "Enemy.h"
#include "AEInput.h"
#include "AEMath.h"
#include <cmath> // For sqrtf
#include <fstream>

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

// Helper for Health Bar quad
static AEGfxVertexList* GetQuadMesh()
{
    static AEGfxVertexList* quad = nullptr;
    if (!quad) {
        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0, 0.5f, -0.5f, 0xFFFFFFFF, 1, 0, 0.5f, 0.5f, 0xFFFFFFFF, 1, 1);
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0, 0.5f, 0.5f, 0xFFFFFFFF, 1, 1, -0.5f, 0.5f, 0xFFFFFFFF, 0, 1);
        quad = AEGfxMeshEnd();
    }
    return quad;
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

void Enemy::DrawHealthBar() const
{
    if (health <= 0) return;

    float percent = health / maxhealth;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;

    float width = _sizeX;
    float height = 6.0f;
    float barX = x;
    float barY = y - (_sizeY * 0.5f) - 8.0f;

    AEGfxVertexList* quad = GetQuadMesh();
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // Red Background
    AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
    AEMtx33 scaleBg, transBg, transformBg;
    AEMtx33Scale(&scaleBg, width, height);
    AEMtx33Trans(&transBg, barX, barY);
    AEMtx33Concat(&transformBg, &transBg, &scaleBg);
    AEGfxSetTransform(transformBg.m);
    AEGfxMeshDraw(quad, AE_GFX_MDM_TRIANGLES);

    // Green Foreground
    float currentWidth = width * percent;
    float currentX = barX - (width * 0.5f) + (currentWidth * 0.5f);

    AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
    AEMtx33 scaleFg, transFg, transformFg;
    AEMtx33Scale(&scaleFg, currentWidth, height);
    AEMtx33Trans(&transFg, currentX, barY);
    AEMtx33Concat(&transformFg, &transFg, &scaleFg);
    AEGfxSetTransform(transformFg.m);
    AEGfxMeshDraw(quad, AE_GFX_MDM_TRIANGLES);
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

    DrawHealthBar();
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

// --- Wave System Implementation ---

bool WaveManager::LoadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    waves.clear();
    WaveData wd;

    // File format expectation: [EnemyType] [Count] [SpawnDelay]
    while (file >> wd.enemyType >> wd.count >> wd.spawnDelay) {
        waves.push_back(wd);
    }

    currentWaveIndex = 0;
    spawnedInCurrentWave = 0;
    spawnTimer = 0.0f;
    waveComplete = waves.empty();

    return true;
}

Enemy* WaveManager::UpdateAndSpawn(float dt, const std::vector<Point>& path)
{
    if (waveComplete || path.empty()) return nullptr;

    spawnTimer += dt;
    WaveData& currentWave = waves[currentWaveIndex];

    if (spawnTimer >= currentWave.spawnDelay) {
        spawnTimer = 0.0f;
        spawnedInCurrentWave++;

        Enemy* e = nullptr;
        switch (currentWave.enemyType) {
        case 0: e = new Zombie();   static_cast<Zombie*>(e)->Init(); break;
        case 1: e = new Skeleton(); static_cast<Skeleton*>(e)->Init(); break;
        case 2: e = new Troll();    static_cast<Troll*>(e)->Init(); break;
        case 3: e = new Golem();    static_cast<Golem*>(e)->Init(); break;
        case 4: e = new Titan();    static_cast<Titan*>(e)->Init(); break;
        default: e = new Zombie();  static_cast<Zombie*>(e)->Init(); break;
        }

        e->x = path[0].x;
        e->y = path[0].y;

        if (spawnedInCurrentWave >= currentWave.count) {
            currentWaveIndex++;
            spawnedInCurrentWave = 0;
            if (currentWaveIndex >= (int)waves.size()) {
                waveComplete = true;
            }
        }

        return e;
    }
    return nullptr;
}