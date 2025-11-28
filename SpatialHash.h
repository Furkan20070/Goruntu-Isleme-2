#pragma once

#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>

// all my image processing adventures end up with a need for fast lookups. 
// this is a simple 2d spatial hash class. also comes with noise reduction perk.
// hash maps, fast lookups and incremental memory seems criminally underrated tbh.

struct HashPoint 
{
    cv::Point2f pos;
    float value; // impulse strength, edge confidence, etc.
};

struct NeighborResult 
{
    HashPoint point;
    float distance;
    float weight;
};

class SpatialHash2D
{
public:
    SpatialHash2D(float cellSize, float maxDist) : cellSize(cellSize), maxDist(maxDist), maxDist2(maxDist* maxDist)
    {

    }

    void insert(const cv::Point2f& pt, float value = 1.0f);
    void clear();

    std::vector<NeighborResult> queryNeighbors(const cv::Point2f& queryPoint, float radius, float sigma) const;

private:
    float cellSize;
    float maxDist;
    float maxDist2;

    std::unordered_map<long long, std::vector<HashPoint>> grid;

    long long hashCell(int x, int y) const;
};

