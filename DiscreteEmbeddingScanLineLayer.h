#pragma once

#include <iostream>
#include <opencv2/imgproc.hpp>
#include "PatternDatabase.h"

class DiscreteEmbeddingScanLineLayer {
public:
    DiscreteEmbeddingScanLineLayer(
        int height,
        float decayRate,
        float clusteringThreshold,
        bool isLearningMode,
        size_t memoryKernelSize = 5
    )
        : memory(height, 0.0f),
        decayRate(decayRate),
        kernelSize(memoryKernelSize),
        database(clusteringThreshold),
        isLearning(isLearningMode)
    {
        classification.assign(height, -1);
    }

    // Receives cluster IDs from previous layer's scan step
    void step(const std::vector<int>& clusterIds, const PatternDatabase& lowerDB) 
    {
        decayMemory();

        int halfK = kernelSize / 2;

        for (int y = halfK; y < height() - halfK; ++y) 
        {
            int clusterId = clusterIds[y];
            if (clusterId != -1) {
                memory[y] = 1.0f;

                std::vector<float> memKernel;
                for (int k = -halfK; k <= halfK; ++k)
                    memKernel.push_back(memory[y + k]);

                const auto& embedding = lowerDB.getDiscreteEmbedding(clusterId);
                memKernel.insert(memKernel.end(), embedding.begin(), embedding.end());

                int result = database.classify(memKernel);
                if (isLearning)
                {
                    database.addPattern(memKernel);
                }
                else
                {
                    classification[y] = result;
                }
            }
        }
    }

    int classifyAt(int y) const 
    {
        return (y >= 0 && y < classification.size()) ? classification[y] : -1;
    }

    const PatternDatabase& getDatabase() const
    {
        return database;
    }

    void saveClusters(const std::string& filename) const 
    {
        database.saveToFile(filename);
    }

    void loadClusters(const std::string& filename)
    {
        database.loadFromFile(filename);
    }

private:
    std::vector<float> memory;
    std::vector<int> classification;
    float decayRate;
    size_t kernelSize;
    PatternDatabase database;
    bool isLearning;

    void decayMemory() 
    {
        for (float& val : memory)
            val *= decayRate;
        classification.assign(memory.size(), -1);
    }

    int height() const 
    {
        return static_cast<int>(memory.size());
    }
};


