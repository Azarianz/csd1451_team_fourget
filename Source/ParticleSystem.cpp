#include "ParticleSystem.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

//cmath should have M_PI, but just in case:
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ParticleSystem
{
    // --------------------------------------------------------
    //  File scope data
    // --------------------------------------------------------
    static std::vector<Particle> g_Particles;
    static AEGfxVertexList* g_QuadMesh = nullptr;

    // Returns a random float between min and max
    static float RandRange(float min, float max)
    {
        return min + ((float)rand() / (float)RAND_MAX) * (max - min);
    }

    static AEGfxVertexList* GetQuadMesh()
    {
        if (g_QuadMesh) return g_QuadMesh;

        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, -0.5f, 0xFFFFFFFF, 1, 0,
            0.5f, 0.5f, 0xFFFFFFFF, 1, 1);
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 0,
            0.5f, 0.5f, 0xFFFFFFFF, 1, 1,
            -0.5f, 0.5f, 0xFFFFFFFF, 0, 1);
        g_QuadMesh = AEGfxMeshEnd();
        return g_QuadMesh;
    }

    // ============================================================
    //  SpawnBurst
    //  Spawns a burst of particles at position (x, y).
    //  Count, size and speed scale with BurstSize.
    // ============================================================
    void SpawnBurst(float x, float y, float r, float g, float b, BurstSize burstSize)
    {
        int   count;
        float minSize, maxSize;
        float minSpeed, maxSpeed;
        float minLife, maxLife;

        switch (burstSize)
        {
        case BurstSize::SMALL:
            count = 6;
            minSize = 8.0f;  maxSize = 15.0f;
            minSpeed = 40.0f; maxSpeed = 90.0f;
            minLife = 0.2f;  maxLife = 0.4f;
            break;

        case BurstSize::LARGE:
            count = 30;
            minSize = 15.0f;  maxSize = 25.0f;
            minSpeed = 80.0f; maxSpeed = 200.0f;
            minLife = 0.4f;  maxLife = 0.8f;
            break;

        case BurstSize::MEDIUM:
        default:
            count = 18;
            minSize = 10.0f;  maxSize = 20.0f;
            minSpeed = 60.0f; maxSpeed = 140.0f;
            minLife = 0.3f;  maxLife = 0.6f;
            break;
        }

        for (int i = 0; i < count; ++i)
        {
            // Spread particles in a random direction
            float angle = RandRange(0.0f, 2.0f * (float)M_PI);
            float speed = RandRange(minSpeed, maxSpeed);
            float life = RandRange(minLife, maxLife);
            float size = RandRange(minSize, maxSize);

            Particle p;
            p.x = x;
            p.y = y;
            p.velX = cosf(angle) * speed;
            p.velY = sinf(angle) * speed;
            p.lifetime = life;
            p.maxLifetime = life;
            p.size = size;
            p.r = r;
            p.g = g;
            p.b = b;

            g_Particles.push_back(p);
        }
    }

    // ============================================================
    //  Update
    //  Moves particles and removes expired ones.
    // ============================================================
    void Update(float dt)
    {
        for (auto& p : g_Particles)
        {
            p.x += p.velX * dt;
            p.y += p.velY * dt;
            p.lifetime -= dt;

            // Slow down over time
            p.velX *= 0.99f;
            p.velY *= 0.99f;
        }

        // Remove dead particles
        g_Particles.erase(
            std::remove_if(g_Particles.begin(), g_Particles.end(),
                [](const Particle& p) { return p.lifetime <= 0.0f; }),
            g_Particles.end());
    }

    // ============================================================
    //  Draw
    //  Draws all active particles, fading out as they expire.
    // ============================================================
    void Draw()
    {
        AEGfxVertexList* mesh = GetQuadMesh();
        if (!mesh) return;

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        for (const auto& p : g_Particles)
        {
            // Alpha fades from 1 to 0 as lifetime runs out
            float alpha = p.lifetime / p.maxLifetime;

            AEGfxSetTransparency(alpha);
            AEGfxSetColorToMultiply(p.r, p.g, p.b, 1.0f);

            AEMtx33 s, t, m;
            AEMtx33Scale(&s, p.size, p.size);
            AEMtx33Trans(&t, p.x, p.y);
            AEMtx33Concat(&m, &t, &s);
            AEGfxSetTransform(m.m);
            AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
        }

        AEGfxSetTransparency(1.0f);
    }

    // ============================================================
    //  Shutdown
    //  Clears all particles and frees the shared mesh.
    // ============================================================
    void Shutdown()
    {
        g_Particles.clear();

        if (g_QuadMesh)
        {
            AEGfxMeshFree(g_QuadMesh);
            g_QuadMesh = nullptr;
        }
    }
}