// Enemy.cpp
#include "Enemy.h"
#include "AEInput.h"
#include "AEMath.h"
#include <cmath> // For sqrtf
#include <fstream>
#include <string>

// --- Sprite Sheet Cache ---
static AEGfxTexture* g_EnemySpriteSheet = nullptr;
static const int SHEET_COLS = 13;
static const int SHEET_ROWS = 10;
static AEGfxVertexList* g_FrameMeshes[SHEET_ROWS][SHEET_COLS] = { nullptr };

// Helper to load the sheet and build UV meshes
static AEGfxVertexList* GetEnemyFrameMesh(int col, int row)
{
    // Check if sprite sheet loaded
    if (!g_EnemySpriteSheet) {
        g_EnemySpriteSheet = AEGfxTextureLoad("Assets/Sprites/spritesheet.png");
    }

    if (g_FrameMeshes[row][col]) return g_FrameMeshes[row][col];

    // Invert row index
    int ty = (SHEET_ROWS - 1) - row;
    // Calculates the left U texture coordinate
    float u0 = (float)col / (float)SHEET_COLS;
    // Calculates the right U texture coordinate
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

    // Finalise mesh generation
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
    facingX = 1.0f; // Initialize facing right
}

void Enemy::SetSprite(int row, int col)
{
    spriteRow = row;
    spriteCol = col;
}

void Enemy::Scale(int waveNumber)
{
    int scaleTier = (waveNumber - 1) / 5;

    float hpMultiplier = 1.0f + (scaleTier * 0.15f);
    float dmgMultiplier = 1.0f + (scaleTier * 0.05f);

    maxhealth *= hpMultiplier;
    health = maxhealth;
    damage *= dmgMultiplier;
}

void Enemy::Update(float dt, const std::vector<Point>& path)
{
    if (health > 0.0f && health < maxhealth && healthRegenRate > 0.0f)
    {
        health += healthRegenRate * dt;
        if (health > maxhealth)
        {
            health = maxhealth; // Clamp to prevent over-healing
        }
    }

    if (flashTimer > 0.0f)
    {
        flashTimer -= dt;
        if (flashTimer < 0.0f)
        {
            flashTimer = 0.0f;
        }
    }

    // Tick slow timer
    if (slowTimer > 0.0f)
    {
        slowTimer -= dt;
        if (slowTimer <= 0.0f)
        {
            slowTimer = 0.0f;
            slowMultiplier = 1.0f; // slow expired
        }
    }
    // Check for path
    if (path.empty() || reachedEnd) return;
    // Check if path is beyond available path points
    if (pathIndex >= (int)path.size()) {
        reachedEnd = true;
        return;
    }

    // Retrieve the coordinates of the current target waypoint
    Point target = path[pathIndex];
    float dx = target.x - x;
    float dy = target.y - y;
    float distSq = dx * dx + dy * dy;

    float effectiveSpeed = speed * slowMultiplier;
    float moveDist = effectiveSpeed * dt;
    float dist = sqrtf(distSq);

    // If the distance to the target is greater than the distance we move this frame
    if (dist > moveDist)
    {
        x += (dx / dist) * moveDist;
        y += (dy / dist) * moveDist;

        // Flip sprite based on horizontal movement
        if (dx > 0.1f) facingX = 1.0f;       // Moving right
        else if (dx < -0.1f) facingX = -1.0f; // Moving left
    }
    else
    {
        // We will reach or pass the target this frame, so snap to it and move to next
        x = target.x;
        y = target.y;

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
    AEGfxVertexList* spriteMesh = GetEnemyFrameMesh(spriteCol, spriteRow);
    if (!spriteMesh || !g_EnemySpriteSheet) return;

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(g_EnemySpriteSheet, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(color.a);

    // Use a single if-else chain to prevent colors from overwriting each other
    // (This also fixes the bug where the flashTimer was being overwritten by the old slowTimer check)
    if (flashTimer > 0.0f)
    {
        // Tint red (similar to the slow tint) instead of a harsh solid flash
        AEGfxSetColorToMultiply(1.0f, 0.4f, 0.4f, 1.0f);
    }
    else if (slowTimer > 0.0f)
    {
        // Tint blue if slowed
        AEGfxSetColorToMultiply(0.5f, 0.7f, 1.0f, 1.0f);
    }
    else
    {
        // Normal color
        AEGfxSetColorToMultiply(color.r, color.g, color.b, 1.0f);
    }

    AEMtx33 scaleM, rotM, transM, transform;

    // Apply facingX to the scale matrix to flip the sprite
    AEMtx33Scale(&scaleM, _sizeX * facingX, _sizeY);

    AEMtx33Rot(&rotM, 0.0f);
    AEMtx33Trans(&transM, x, y);

    AEMtx33Concat(&transform, &rotM, &scaleM);
    AEMtx33Concat(&transform, &transM, &transform);

    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(spriteMesh, AE_GFX_MDM_TRIANGLES);

    DrawHealthBar();
}

// --- Specific Enemies ---

void ZombieV1::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 50.0f, 10.0f, 125.0f);
    SetSprite(0, 11);
}

void SkeletonV1::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 30.0f, 5.0f, 200.0f);
    SetSprite(0, 5);
}

void TrollV1::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 150.0f, 15.0f, 80.0f);
    SetSprite(0, 3);
}

void GolemV1::Init()
{
    Enemy::Init(60.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 300.0f, 40.0f, 50.0f);
    SetSprite(0, 4);
    healthRegenRate = 7.5f;
}

void TitanV1::Init()
{
    Enemy::Init(50.0f, 50.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 400.0f, 30.0f, 75.0f);
    SetSprite(0, 7);
    healthRegenRate = 2.5f;
}

void Zombie::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 75.0f, 15.0f, 125.0f);
    SetSprite(1, 11);
}

void Skeleton::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 50.0f, 10.0f, 200.0f);
    SetSprite(1, 5);
}

void Troll::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 200.0f, 30.0f, 80.0f);
    SetSprite(1, 3);
}

void Golem::Init()
{
    Enemy::Init(60.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 500.0f, 60.0f, 50.0f);
    SetSprite(1, 4);
    healthRegenRate = 15.0f;
}

void Titan::Init()
{
    Enemy::Init(50.0f, 50.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 650.0f, 40.0f, 75.0f);
    SetSprite(1, 7);
    healthRegenRate = 5.0f;
}

void wavestarter::Init()
{
    Enemy::Init(40.0f, 40.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 50.0f, 10.0f, 125.0f);
    SetSprite(3, 11);
}

// --- Wave System Implementation ---

bool WaveManager::LoadLevel(int levelNumber)
{
    std::string filename = "Assets/wave" + std::to_string(levelNumber) + ".txt";
    return LoadFromFile(filename);
}

bool WaveManager::LoadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    waves.clear();
    WaveData wd{};
    totalWavestarters = 0; // Reset total counter

    // File format expectation: [EnemyType] [Count] [SpawnDelay]
    while (file >> wd.enemyType >> wd.count >> wd.spawnDelay) {
        waves.push_back(wd);

        // Count total wavestarters in the file (Type 5)
        if (wd.enemyType == 5) {
            totalWavestarters += wd.count;
        }
    }

    currentWaveIndex = 0;
    spawnedInCurrentWave = 0;
    spawnTimer = 0.0f;
    wavestarterCount = 0; // Resets current spawned tracker
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
        case 0: e = new Zombie();       static_cast<Zombie*>(e)->Init(); break;
        case 1: e = new Skeleton();     static_cast<Skeleton*>(e)->Init(); break;
        case 2: e = new Troll();        static_cast<Troll*>(e)->Init(); break;
        case 3: e = new Golem();        static_cast<Golem*>(e)->Init(); break;
        case 4: e = new Titan();        static_cast<Titan*>(e)->Init(); break;
        case 5: e = new wavestarter();  static_cast<wavestarter*>(e)->Init(); wavestarterCount++; break;
        case 6: e = new Zombie();       static_cast<ZombieV1*>(e)->Init(); break;
        case 7: e = new Skeleton();     static_cast<SkeletonV1*>(e)->Init(); break;
        case 8: e = new Troll();        static_cast<TrollV1*>(e)->Init(); break;
        case 9: e = new Golem();        static_cast<GolemV1*>(e)->Init(); break;
        case 10: e = new Titan();        static_cast<TitanV1*>(e)->Init(); break;
        default: e = new Zombie();      static_cast<Zombie*>(e)->Init(); break;
        }

        e->x = path[0].x;
        e->y = path[0].y;

        e->Scale(currentWaveIndex + 1);

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