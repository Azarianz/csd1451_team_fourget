#pragma once
#include "AEEngine.h"
#include <vector>

// ============================================================
//  ParticleSystem
//  Manages simple burst particles for hit effects.
//  Call SpawnBurst() when a bullet hits an enemy.
//  Call Update() and Draw() each frame.
// ============================================================
namespace ParticleSystem
{
    enum class BurstSize
    {
        SMALL,   // Rapid Tower
        MEDIUM,  // Basic Tower
        LARGE,   // Sniper Tower
    };

    struct Particle
    {
        float x, y;           // world position
        float velX, velY;     // movement direction and speed
        float lifetime;       // seconds remaining
        float maxLifetime;    // starting lifetime (used to fade alpha)
        float size;           // world size of the particle
        float r, g, b;        // color matching the bullet
    };

    // Spawns a burst of particles at the given position
    void SpawnBurst(float x, float y, float r, float g, float b, BurstSize size);

    // Updates all active particles
    void Update(float dt);

    // Draws all active particles
    void Draw();

    // Clears all active particles (call on scene exit)
    void Shutdown();
}