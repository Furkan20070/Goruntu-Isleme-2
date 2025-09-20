#pragma once

#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include<unordered_map>

struct PatternCluster 
{
    std::vector<float> centroid;
    std::vector<float> embedding;
    int embeddingId; // For now, just set to cluster index
    int count;

    PatternCluster(const std::vector<float>& initial, int id)
        : centroid(initial), count(1), embedding(createInitialEmbedding(initial)), embeddingId(id) {}

    void update(const std::vector<float>& new_vector) 
    {
        for (size_t i = 0; i < centroid.size(); ++i) 
        {
            centroid[i] = (centroid[i] * count + new_vector[i]) / (count + 1);
        }

        // Update embedding similarly
        for (size_t i = 0; i < embedding.size(); ++i)
        {
            embedding[i] = (embedding[i] * count + new_vector[i]) / (count + 1);
        } 

        count++;
    }

    static std::vector<float> createInitialEmbedding(const std::vector<float>& pattern) 
    {
        // Simple initial embedding: first N dims of pattern (or padded if shorter)
        constexpr size_t EMBEDDING_DIM = 5;
        std::vector<float> emb(EMBEDDING_DIM, 0.0f);
        for (size_t i = 0; i < std::min(pattern.size(), EMBEDDING_DIM); ++i)
            emb[i] = pattern[i];
        return emb;
    }
};

class PatternDatabase 
{
public:
    PatternDatabase(float threshold) : distanceThreshold(threshold) {}

    const std::vector<float>& getDiscreteEmbedding(int clusterId) const 
    {
        int id = clusters[clusterId].embeddingId;
        return embeddingTable.at(id);
    }


    // Add a new vector: assign to a cluster or create a new one
    void addPattern(const std::vector<float>& pattern) 
    {
        int bestIndex = findClosestCluster(pattern);
        if (bestIndex != -1) 
        {
            clusters[bestIndex].update(pattern);
        }
        else 
        {
            clusters.emplace_back(pattern, nextEmbeddingId);
            std::vector<float> defaultEmbedding = generateEmbedding(nextEmbeddingId);
            embeddingTable[nextEmbeddingId] = defaultEmbedding;
            nextEmbeddingId++;
        }
    }

    // Find the best matching cluster index, or -1 if no match
    int classify(const std::vector<float>& pattern) const
    {
        int bestIndex = -1;
        float bestDist = std::numeric_limits<float>::max();

        for (size_t i = 0; i < clusters.size(); ++i) 
        {
            float dist = euclideanDistance(clusters[i].centroid, pattern);
            if (dist < bestDist) 
            {
                bestDist = dist;
                bestIndex = static_cast<int>(i);
            }
        }

        return (bestDist <= distanceThreshold) ? bestIndex : -1;
    }

    const std::vector<PatternCluster>& getClusters() const 
    {
        return clusters;
    }

    size_t size() const 
    {
        return clusters.size();
    }

    void saveToFile(const std::string& filename) const
    {
        std::ofstream out(filename);
        if (!out.is_open()) 
        {
            std::cerr << "Failed to open file for saving clusters.\n";
            return;
        }

        out << clusters.size() << "\n";

        for (const auto& cluster : clusters) 
        {
            out << cluster.count;

            // Save centroid
            for (float val : cluster.centroid)
                out << " " << val;

            // Save embeddingId
            out << " " << cluster.embeddingId;

            // Save embedding vector
            const auto& emb = embeddingTable.at(cluster.embeddingId);
            out << " " << emb.size(); // save embedding length just in case
            for (float val : emb)
                out << " " << val;

            out << "\n";
        }

        out.close();
    }


    void loadFromFile(const std::string& filename) 
    {
        std::ifstream in(filename);
        if (!in.is_open()) 
        {
            std::cerr << "Failed to open file for loading clusters.\n";
            return;
        }

        size_t numClusters;
        in >> numClusters;
        clusters.clear();
        embeddingTable.clear();
        nextEmbeddingId = 0; // reset ID counter

        for (size_t i = 0; i < numClusters; ++i) {
            int count, embId, embSize;
            std::vector<float> centroid;

            in >> count;

            // Read centroid until we reach the embeddingId (assume known pattern size)
            for (int j = 0; j < 5; ++j) 
            { // or use a stored pattern size
                float val;
                in >> val;
                centroid.push_back(val);
            }

            in >> embId >> embSize;
            std::vector<float> emb(embSize);
            for (int j = 0; j < embSize; ++j)
                in >> emb[j];

            clusters.emplace_back(centroid, embId);
            clusters.back().count = count;
            embeddingTable[embId] = emb;

            if (embId >= nextEmbeddingId)
                nextEmbeddingId = embId + 1;
        }

        in.close();
    }

    const std::vector<float>& getClusterEmbedding(int clusterIndex) const 
    {
        return clusters[clusterIndex].embedding;
    }


private:
    std::vector<PatternCluster> clusters;
    float distanceThreshold;
    int nextEmbeddingId = 0; // Add this to PatternDatabase
    std::unordered_map<int, std::vector<float>> embeddingTable;

    int findClosestCluster(const std::vector<float>& pattern) const 
    {
        float bestDist = std::numeric_limits<float>::max();
        int bestIndex = -1;

        for (size_t i = 0; i < clusters.size(); ++i) 
        {
            float dist = euclideanDistance(clusters[i].centroid, pattern);
            if (dist < bestDist) 
            {
                bestDist = dist;
                bestIndex = static_cast<int>(i);
            }
        }

        return (bestDist <= distanceThreshold) ? bestIndex : -1;
    }

    static float euclideanDistance(const std::vector<float>& a, const std::vector<float>& b)
    {
        float sum = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) 
        {
            float d = a[i] - b[i];
            sum += d * d;
        }
        return std::sqrt(sum);
    }

    std::vector<float> generateEmbedding(int id) 
    {
        // Simple: one-hot encoding for up to N clusters
        std::vector<float> emb(4, 0.0f);
        emb[id % 4] = 1.0f;
        return emb;

        // Or: define your own layout, e.g., gradual shifts for angles
    }
};


