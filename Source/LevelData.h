#pragma once
#include <vector>
#include <string>
#include <cstdint>

enum class RegionFlag : uint8_t
{
    NONE = 0,
    BUILDABLE = 1,
    ENEMYPATH = 2
};

struct LevelData
{
    int width = 0;
    int height = 0;

    // MapLayer: tile ids (0 = empty)
    std::vector<int> map;

    // RegionLayer: flags stored as bytes
    std::vector<uint8_t> region;

    void Resize(int w, int h)
    {
        width = (w < 0) ? 0 : w;
        height = (h < 0) ? 0 : h;
        map.assign((size_t)width * (size_t)height, 0);
        region.assign((size_t)width * (size_t)height, (uint8_t)RegionFlag::NONE);
    }

    size_t Idx(int x, int y) const
    {
        return (size_t)y * (size_t)width + (size_t)x;
    }

    bool InBounds(int x, int y) const
    {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    bool Save(const std::string& path) const;
    bool Load(const std::string& path);
};