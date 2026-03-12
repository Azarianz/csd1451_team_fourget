#include "Tower.h"
#include "Utility.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>

namespace TowerHandler {

    // --------------------------------------------------------
    //  File-scope data
    // --------------------------------------------------------
    static AEGfxTexture* g_TowerSheet = nullptr;
    int nextTowerID = 0;

    // Maps TowerType enum value -> spritesheet column
    // Order must match the TowerType enum: BASIC, SNIPER, SLOW, RAPID, BASE
    static const int TOWER_SPRITE_COLS[] = { 1, 5, 4, 2, 0 };

    // ============================================================
    //  Asset loading
    // ============================================================
    void LoadTowerAssets(){
        if (!g_TowerSheet)
            g_TowerSheet = AEGfxTextureLoad("Assets/spritesheet.png");
    }

    // ============================================================
    //  Tower - initialisation
    // ============================================================
    void Tower::TowerInit(float xPos, float yPos, float xSize, float ySize,
                          ShopTower shop, int seg_count){
		// GameObject data
        x        = xPos;
        y        = yPos;
        _sizeX   = xSize;
        _sizeY   = ySize;
        segments = seg_count;
        mesh     = nullptr;
 
		// Placement defaults
        tower_count  = 0;
        isDragging   = false;
        dragOffsetX  = 0;
        dragOffsetY  = 0;
 
		// Tower details defaults (overridden below and in ApplyLevelStats)
        details.level     = 1;
        details.ID        = nextTowerID++;
        details.towerType = shop.GetTowerType();
        details.fireTimer = 0.f;
 
		// Base-tower fields default to 0/false for non-base types, then overridden in the switch if needed
        details.health        = 0.f;
        details.maxHealth     = 0.f;
        details.contactDamage = 0.f;
        details.isBase        = false;
 
        ApplyLevelStats();
 
		// TowerType-specific overrides (color, base-tower stats, spritesheet column)
        switch (details.towerType)
        {
        case BASIC_TOWER:
        case SNIPER_TOWER:
        case SLOW_TOWER:
        case RAPID_TOWER:
            color = { 1.0f, 1.0f, 1.0f, 1.0f };
            break;
 
        case BASE_TOWER:
            color                 = { 0.2f, 0.9f, 0.9f, 1.0f };
            details.maxHealth     = 100.f;
            details.health        = 100.f;
            details.contactDamage = 10.f;
            details.isBase        = true;
            break;
 
        default:
            // white and weak stats to make bugs visible
            color                    = { 1.0f, 1.0f, 1.0f, 1.0f };
            details.range            = 100.f;
            details.fireCooldown     = 10.f;
            details.projectile.damage = 0.f;
            details.projectile.speed  = 100.f;
            break;
        }
 
        // Sprite setup
        details.spriteCol     = TOWER_SPRITE_COLS[(int)details.towerType];
        details.spriteBaseRow = 0;
 
        int col    = details.spriteCol;
        int row    = details.spriteBaseRow + (details.level - 1);
        UVRect uv  = GetSpriteUV(col, row, 13, 10);
 
        spriteId = Graphics::DrawSprite(g_TowerSheet,
            x, y, _sizeX, _sizeY,
            1.f, 1.f, 1.f, 1.f,
            uv.u0, uv.v0, uv.u1, uv.v1);
    }

    // ============================================================
    //  Tower - stats
    // ============================================================
    void Tower::ApplyLevelStats()
    {
        int typeIndex  = (int)details.towerType;
        int levelIndex = details.level - 1;  // level 1 -> index 0
 
        // Guard against out-of-bounds if table is not updated for a new type
        if (typeIndex  < 0 || typeIndex  >= 5) return;
        if (levelIndex < 0) levelIndex = 0;
        if (levelIndex > 2) levelIndex = 2;
 
        const LevelStats& ls     = TOWER_LEVEL_STATS[typeIndex][levelIndex];
        details.range             = ls.range;
        details.fireCooldown      = ls.fireCooldown;
        details.projectile.damage = ls.damage;
        details.projectile.speed  = ls.speed;
    }
 
    bool Tower::LevelUp()
    {
        if (details.level >= 3)
            return false; // already max level
 
        details.level++;
        ApplyLevelStats();
 
        // Update sprite to the next row
        int col   = details.spriteCol;
        int row   = details.spriteBaseRow + (details.level - 1);
        UVRect uv = GetSpriteUV(col, row, 13, 10);
        Graphics::SetUV(spriteId, uv.u0, uv.v0, uv.u1, uv.v1);
 
        return true;
    }
 
    // ============================================================
    //  Tower - base-tower health
    // ============================================================
    bool Tower::TakeDamage(float dmg)
    {
        if (!details.isBase) return false;
 
        details.health -= dmg;
        if (details.health < 0.0f) details.health = 0.0f;
 
        return details.health <= 0.0f; // true = tower died
    }
 
    bool Tower::IsDead()      const { return details.isBase && details.health <= 0.0f; }
    bool Tower::IsBaseTower() const { return details.isBase; }
 
    // ============================================================
    //  Tower - rendering
    // ============================================================
    void Tower::Draw()
    {
        // Pulse sprite scale when selected
        if (isSelected)
        {
            pulseTimer += (float)AEFrameRateControllerGetFrameTime();
            float pulse = 1.0f + 0.12f * sinf(pulseTimer * 6.0f);
            Graphics::SetScale(spriteId, _sizeX * pulse, _sizeY * pulse);
        }
        else
        {
            pulseTimer = 0.0f;
            Graphics::SetScale(spriteId, _sizeX, _sizeY);
        }
 
        Graphics::SetPosition(spriteId, x, y);
 
        // Range ring (only while dragging or selected)
        if (isDragging || isSelected)
        {
            float savedSX    = _sizeX;
            float savedSY    = _sizeY;
            Color savedColor = color;
 
            _sizeX = _sizeY = details.range;
            color = { 1.f, 1.f, 1.f, 0.4f };
            GameObject::Draw();
 
            _sizeX = savedSX;
            _sizeY = savedSY;
            color  = savedColor;
        }
 
        DrawHealthBar(); // no-op for non-base towers
    }
 
    // ============================================================
    //  Tower - health bar (base tower only)
    // ============================================================
    static AEGfxVertexList* GetHealthBarQuad()
    {
        static AEGfxVertexList* quad = nullptr;
        if (!quad)
        {
            AEGfxMeshStart();
            AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
                         0.5f, -0.5f, 0xFFFFFFFF, 1, 0,
                         0.5f,  0.5f, 0xFFFFFFFF, 1, 1);
            AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
                         0.5f,  0.5f, 0xFFFFFFFF, 1, 1,
                        -0.5f,  0.5f, 0xFFFFFFFF, 0, 1);
            quad = AEGfxMeshEnd();
        }
        return quad;
    }
 
    void Tower::DrawHealthBar() const
    {
        if (!details.isBase || details.maxHealth <= 0.0f) return;
 
        float percent = details.health / details.maxHealth;
        percent = (percent < 0.0f) ? 0.0f : (percent > 1.0f) ? 1.0f : percent;
 
        const float barWidth  = _sizeX;
        const float barHeight = 8.0f;
        const float barX      = x;
        const float barY      = y - (_sizeY * 0.5f) - 10.0f;
 
        AEGfxVertexList* quad = GetHealthBarQuad();
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
 
        auto DrawQuad = [&](float cx, float w, float r, float g, float b)
        {
            AEGfxSetColorToMultiply(r, g, b, 1.0f);
            AEMtx33 s, t, m;
            AEMtx33Scale(&s, w, barHeight);
            AEMtx33Trans(&t, cx, barY);
            AEMtx33Concat(&m, &t, &s);
            AEGfxSetTransform(m.m);
            AEGfxMeshDraw(quad, AE_GFX_MDM_TRIANGLES);
        };
 
        // Background (red) — full width
        DrawQuad(barX, barWidth, 1.0f, 0.0f, 0.0f);
 
        // Foreground (green) — scaled by health percent, left-anchored
        float fillWidth = barWidth * percent;
        float fillX     = barX - (barWidth * 0.5f) + (fillWidth * 0.5f);
        DrawQuad(fillX, fillWidth, 0.0f, 1.0f, 0.0f);
    }
 
    // ============================================================
    //  ShopTower
    // ============================================================
    void ShopTower::ShopTowerInit(float xPos, float yPos, float xSize, float ySize,
                                   TowerType towerType, int seg_count)
    {
        x        = xPos;
        y        = yPos;
        _sizeX   = xSize;
        _sizeY   = ySize;
        segments = seg_count;
        mesh     = nullptr;
 
        shopTowerType = towerType;
        color = (towerType == BASE_TOWER)
            ? Color{ 0.2f, 0.9f, 0.9f, 1.0f }
            : Color{ 1.0f, 1.0f, 1.0f, 1.0f };
 
        int    col = TOWER_SPRITE_COLS[(int)towerType];
        UVRect uv  = GetSpriteUV(col, 0, 13, 10);
        spriteId   = Graphics::DrawSprite(g_TowerSheet,
            x, y, xSize, ySize,
            1.f, 1.f, 1.f, 1.f,
            uv.u0, uv.v0, uv.u1, uv.v1);
    }
 
    void ShopTower::SetType(TowerType newType)
    {
        shopTowerType = newType;
        int    col = TOWER_SPRITE_COLS[(int)newType];
        UVRect uv  = GetSpriteUV(col, 0, 13, 10);
        Graphics::SetUV(spriteId, uv.u0, uv.v0, uv.u1, uv.v1);
    }
 
    // ============================================================
    //  UpdateTowerSystem  (placement & dragging)
    // ============================================================
    void UpdateTowerSystem(float mouseX, float mouseY,
                           ShopTower& shop, std::vector<Tower>& activeTowers)
    {
        Utility::GetWorldMousePos(mouseX, mouseY);
 
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // -- Find the topmost (highest ID) tower under the cursor --
            Tower* topMostTower = nullptr;
            int    highestID    = -1;
 
            for (Tower& t : activeTowers)
            {
                if (Utility::IsCircleClicked(t.x, t.y, t._sizeX, mouseX, mouseY))
                {
                    if (t.details.ID > highestID)
                    {
                        highestID    = t.details.ID;
                        topMostTower = &t;
                    }
                }
            }
 
            // Deselect everything first
            for (Tower& t : activeTowers)
                t.isSelected = false;
 
            // Drag the topmost hit tower
            if (topMostTower)
            {
                topMostTower->isSelected   = true;
                topMostTower->isDragging   = true;
                topMostTower->dragOffsetX  = topMostTower->x - mouseX;
                topMostTower->dragOffsetY  = topMostTower->y - mouseY;
                return;
            }
 
            // Otherwise spawn from shop if shop is clicked
            if (Utility::IsCircleClicked(shop.x, shop.y, shop._sizeX, mouseX, mouseY))
            {
                Tower newTower;
                newTower.TowerInit(shop.x, shop.y, 55.0f, 55.0f, shop);
                newTower.isSelected  = true;
                newTower.isDragging  = true;
                newTower.dragOffsetX = 0; // center on cursor
                newTower.dragOffsetY = 0;
                activeTowers.push_back(newTower);
                return;
            }
        }
 
        // -- Drag selected tower --
        for (Tower& t : activeTowers)
        {
            if (!t.isDragging) continue;
 
            t.x = mouseX + t.dragOffsetX;
            t.y = mouseY + t.dragOffsetY;
 
            if (!AEInputCheckCurr(AEVK_LBUTTON))
                t.isDragging = false;
        }
    }
 
    // ============================================================
    //  TowerShoot
    // ============================================================
    bool TowerShoot(Tower& tower, Enemy& enemy, std::vector<ActiveBullet>& bullets)
    {
        if (tower.details.fireTimer > 0.0f) return false;
 
        if (!CircleCircleCollision(tower.x, tower.y, tower.details.range,
                                   enemy.x,  enemy.y,  enemy._sizeX))
            return false;
 
        // Build bullet
        ActiveBullet b;
        b.x        = tower.x;
        b.y        = tower.y;
        b._sizeX   = 10.0f;
        b._sizeY   = 10.0f;
        b.damage   = tower.details.projectile.damage;
        b.speed    = tower.details.projectile.speed;
        b.color    = tower.color;
        b.segments = 20;
        b.mesh     = nullptr;
        b.target   = &enemy;
 
        // Initial direction toward target
        float dx     = enemy.x - tower.x;
        float dy     = enemy.y - tower.y;
        float length = sqrtf(dx * dx + dy * dy);
        if (length > 0.0f)
        {
            b.dirX = dx / length;
            b.dirY = dy / length;
        }
 
        bullets.push_back(b);
        tower.details.fireTimer = tower.details.fireCooldown;
        return true;
    }
 
    // ============================================================
    //  UpdateProjectiles  (movement + hit detection + cleanup)
    // ============================================================
    void UpdateProjectiles(float dt, std::vector<Enemy*>& enemies,
                           std::vector<ActiveBullet>& activeBullets)
    {
        for (auto& b : activeBullets)
        {
            b.Update(dt);
 
            // Hit detection
            for (Enemy* e : enemies)
            {
                if (!e || e->health <= 0.0f) continue;
 
                if (CircleCircleCollision(b.x, b.y, b._sizeX, e->x, e->y, e->_sizeX))
                {
                    e->health     -= b.damage;
                    b.shouldRemove = true;
                    break;
                }
            }
 
            // Out-of-bounds cull
            if (std::fabs(b.x) > 2000.f || std::fabs(b.y) > 2000.f)
                b.shouldRemove = true;
        }
 
        activeBullets.erase(
            std::remove_if(activeBullets.begin(), activeBullets.end(),
                [](const ActiveBullet& b) { return b.shouldRemove; }),
            activeBullets.end());
    }
 
    // ============================================================
    //  CircleCircleCollision  (shared by towers, bullets, enemies)
    // ============================================================
    bool CircleCircleCollision(float x1, float y1, float r1,
                               float x2, float y2, float r2)
    {
        float dx        = x2 - x1;
        float dy        = y2 - y1;
        float sqrDist   = dx * dx + dy * dy;
        float sqrRadius = (r1 + r2) * (r1 + r2);
        return sqrDist <= sqrRadius;
    }
}
