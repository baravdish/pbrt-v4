// sparsity_map_handler.h
#pragma once

#include <pbrt/pbrt.h>
#include <pbrt/util/image.h>
#include <pbrt/util/math.h>
#include <pbrt/util/vecmath.h>

namespace pbrt {

class SparsityMapHandler {

private:
    // Define the struct before it's used in method declarations
    struct PatchStatistics {
        Float mean = 0;
        Float variance = 0;
        Float gradientMagnitude = 0;
        int validPixels = 0;
    };

public:
    // Constructor taking the patch size as parameter
    SparsityMapHandler(int patchSize = 8) 
        : patchSize(patchSize) {}

    // Load sparsity map from file
    bool LoadSparsityMap(const std::string& filename);

    // Get sparsity value for a specific patch
    Float GetPatchSparsity(const Point2i& origin) const;

    // Compute number of samples needed based on sparsity
    int ComputeSampleBudget(Float sparsity, 
                           int minSamples = 16, 
                           int maxSamples = 256) const;

    void InitializePatchGrid();

    bool ValidatePatchBounds(const Point2i& origin) const;

    Float ComputeFallbackSparsity(const Point2i& origin) const;

    Float EstimateSparsity(const PatchStatistics& stats) const;

    // Get patch size
    int GetPatchSize() const { return patchSize; }

    // Check if handler has valid map
    bool HasValidMap() const { return (bool)sparsityMap; }

    // Get map resolution
    Point2i GetResolution() const { return resolution; }

private:
    Image sparsityMap;            // The sparsity map image
    Point2i resolution{0, 0};     // Map resolution
    const int patchSize;          // Size of processing patches

};

} // namespace pbrt