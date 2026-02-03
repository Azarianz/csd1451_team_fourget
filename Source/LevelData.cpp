#include "LevelData.h"
#include <fstream>
#include <sstream>

bool LevelData::Save(const std::string& path) const
{
    std::ofstream out(path);
    if (!out.is_open()) return false;

    out << "MERGE_DEFENDERS_LEVEL_V1\n";
    out << width << " " << height << "\n";

    out << "MAP\n";
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            out << map[Idx(x, y)];
            if (x + 1 < width) out << " ";
        }
        out << "\n";
    }

    out << "REGION\n";
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            out << (int)region[Idx(x, y)];
            if (x + 1 < width) out << " ";
        }
        out << "\n";
    }

    return true;
}

bool LevelData::Load(const std::string& path)
{
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string header;
    std::getline(in, header);
    if (header != "MERGE_DEFENDERS_LEVEL_V1") return false;

    int w = 0, h = 0;
    in >> w >> h;
    if (w <= 0 || h <= 0) return false;

    Resize(w, h);

    std::string tag;

    // MAP
    in >> tag;
    if (tag != "MAP") return false;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            in >> map[Idx(x, y)];

    // REGION
    in >> tag;
    if (tag != "REGION") return false;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int v = 0;
            in >> v;
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            region[Idx(x, y)] = (uint8_t)v;
        }
    }

    return true;
}