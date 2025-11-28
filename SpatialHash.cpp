#include "SpatialHash.h"

// convert 2d point to 1d spatial hash key
long long SpatialHash2D::hashCell(int x, int y) const
{
    return (static_cast<long long>(x) << 32) | (y & 0xffffffff);
}

// insert to broad phase matrix
void SpatialHash2D::insert(const cv::Point2f& pt, float value)
{
    int cx = static_cast<int>(std::floor(pt.x / cellSize));
    int cy = static_cast<int>(std::floor(pt.y / cellSize));

    grid[hashCell(cx, cy)].push_back({ pt, value });
}

void SpatialHash2D::clear()
{
    grid.clear();
}

std::vector<NeighborResult> SpatialHash2D::queryNeighbors(const cv::Point2f& queryPoint, float radius, float sigma) const
{
    std::vector<NeighborResult> results;

    int centerX = static_cast<int>(queryPoint.x / cellSize);
    int centerY = static_cast<int>(queryPoint.y / cellSize);

    int cellRadius = static_cast<int>(std::ceil(radius / cellSize));
    float radius2 = radius * radius;

    // check the cell and around of it
    for (int dx = -cellRadius; dx <= cellRadius; dx++)
    {
        for (int dy = -cellRadius; dy <= cellRadius; dy++)
        {
            long long h = hashCell(centerX + dx, centerY + dy);

            auto it = grid.find(h);
            if (it == grid.end()) continue;

            const auto& cell = it->second;

            for (const auto& p : cell)
            {
                cv::Point2f diff = p.pos - queryPoint;
                float dist2 = diff.dot(diff);

                if (dist2 > radius2) continue;

                // Gaussian weight for noise robustness -- some weird math here but it seems to be working
                float dist = std::sqrt(dist2);
                float weight = std::exp(-dist2 / (2.0f * sigma * sigma));

                results.push_back({ p, dist, weight });
            }
        }
    }

    return results;
}
