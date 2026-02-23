#include "Shop.h"
#include "Utility.h"
#include <iostream>

namespace TowerHandler {
    // Helper to build a unit circle mesh
    static AEGfxVertexList* BuildCircleMesh(int segments)
    {
        AEGfxMeshStart();

        float cx = 0.0f, cy = 0.0f;
        float step = 2.0f * 3.14159f / (float)segments;

        for (int i = 0; i < segments; ++i)
        {
            float a0 = step * (float)i;
            float a1 = step * (float)(i + 1);

            float x0 = cosf(a0);
            float y0 = sinf(a0);
            float x1 = cosf(a1);
            float y1 = sinf(a1);

            unsigned int col = 0xFFFFFFFF; // Pure white 

            AEGfxTriAdd(
                cx, cy, col, 0.5f, 0.5f,
                x0, y0, col, 0.5f + 0.5f * x0, 0.5f - 0.5f * y0,
                x1, y1, col, 0.5f + 0.5f * x1, 0.5f - 0.5f * y1
            );
        }

        return AEGfxMeshEnd();
    }

    void Shop::Init() {
        float windowWidth = (float)AEGfxGetWindowWidth();
        float windowHeight = (float)AEGfxGetWindowHeight();

        // Calculate layout: Center slots at the bottom
        float totalWidth = (TOTAL_SLOTS * slotSize) + ((TOTAL_SLOTS - 1) * padding);
        float startX = -(totalWidth / 2.0f) + (slotSize / 2.0f);
        float posY = -(windowHeight / 2.0f) + (slotSize / 2.0f) + 30.0f;

        Color uniqueColors[TOWER_SLOTS] = {
        { 0.1f, 0.9f, 0.2f, 1.0f }, // Green
        { 1.0f, 0.9f, 0.1f, 1.0f }, // Yellow
        { 0.7f, 0.2f, 0.9f, 1.0f }, // Purple
        { 1.0f, 0.5f, 0.1f, 1.0f }, // Orange
        { 0.1f, 0.8f, 0.9f, 1.0f }  // Cyan
        };

        for (int i = 0; i < TOTAL_SLOTS; ++i) {
            slots[i].x = startX + (i * (slotSize + padding));
            slots[i].y = posY;
            slots[i].size = slotSize;
            slots[i].isRefreshButton = (i == 5);

            if (!slots[i].isRefreshButton) {
                slots[i].typeContained = (i % 2 == 0) ? BASIC_TOWER : SNIPER_TOWER;
                slots[i].slotColor = uniqueColors[i];
            }
        }

        // Create a Circle Mesh
        pMesh = BuildCircleMesh(64);

        if (!pMesh) {
            PRINT("Shop Init: Failed to create circle mesh!\n");
        }
    }

    void Shop::Update(std::vector<Tower>& activeTowers) {
        float mouseX, mouseY;
        Utility::GetWorldMousePos(mouseX, mouseY);

        if (AEInputCheckTriggered(AEVK_LBUTTON)) {
            for (int i = 0; i < TOTAL_SLOTS; ++i) {
                // Check if mouse click is within the shop slot
                if (Utility::IsCircleClicked(slots[i].x, slots[i].y, slots[i].size / 2.0f, mouseX, mouseY)) {

                    if (slots[i].isRefreshButton) {
                        RefreshSlots();
                        return;
                    }

                    // Drag Asset Out Logic
                    Tower newTower;
                    newTower.TowerInit(mouseX, mouseY, 55.0f, 55.0f, slots[i].slotColor);
                    newTower.details.towerType = slots[i].typeContained;

                    newTower.isSelected = true;
                    newTower.isDragging = true;
                    newTower.dragOffsetX = 0;
                    newTower.dragOffsetY = 0;

                    activeTowers.push_back(newTower);
                    break;
                }
            }
        }
    }

    void Shop::Draw() {
        if (!pMesh) return;

        // Set the state ONCE before the loop to prevent GL state collisions
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        for (int i = 0; i < TOTAL_SLOTS; ++i) {
            AEMtx33 scale, trans, transform;
            AEMtx33Scale(&scale, slots[i].size, slots[i].size);
            AEMtx33Trans(&trans, slots[i].x, slots[i].y);
            AEMtx33Concat(&transform, &trans, &scale);

            AEGfxSetTransform(transform.m);

            // Refresh button
            if (slots[i].isRefreshButton) {
                AEGfxSetColorToMultiply(0.4f, 0.4f, 0.4f, 1.0f); // Dark Grey button
            }
            else {
                AEGfxSetColorToMultiply(
                    slots[i].slotColor.r,
                    slots[i].slotColor.g,
                    slots[i].slotColor.b,
                    1.0f );   
            }

            AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    void Shop::RefreshSlots() {
        // Define a larger pool of colors to pick from for variety
        Color colorPool[] = {
            { 0.1f, 0.9f, 0.2f, 1.0f }, // Green
            { 1.0f, 0.9f, 0.1f, 1.0f }, // Yellow
            { 0.7f, 0.2f, 0.9f, 1.0f }, // Purple
            { 1.0f, 0.5f, 0.1f, 1.0f }, // Orange
            { 0.1f, 0.8f, 0.9f, 1.0f }, // Cyan
            { 0.9f, 0.1f, 0.5f, 1.0f }, // Pink
            { 0.5f, 1.0f, 0.0f, 1.0f }  // Lime
        };
        int poolSize = sizeof(colorPool) / sizeof(Color);

        for (int i = 0; i < TOWER_SLOTS; ++i) {
            // Randomize Type
            slots[i].typeContained = (rand() % 2 == 0) ? BASIC_TOWER : SNIPER_TOWER;

            // Randomize Color from the pool
            slots[i].slotColor = colorPool[rand() % poolSize];
        }
    }

    void Shop::Exit() {
        if (pMesh) {
            AEGfxMeshFree(pMesh);
        }
    }
}