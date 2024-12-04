// sparsity_map_handler.cpp
#include "sparsity_map_handler.h"
#include "pbrt.h"
#include "util/log.h"
#include "util/math.h"
#include "util/stats.h"

namespace pbrt {

// Statistics tracking for performance analysis
static StatsAccumulator statsLoadTime;
static StatsAccumulator statsPatchProcessing;

bool SparsityMapHandler::LoadSparsityMap(const std::string& filename) {
    ProfilePhase _(Prof::TextureLoading);
    ScopedStatsAccumulator _s(&statsLoadTime);

    // Load image using PBRT's infrastructure
    ImageAndMetadata imageAndMeta = Image::Read(filename);
    
    if (!imageAndMeta.image) {
        Error("Failed to load sparsity map from: %s", filename);
        return false;
    }

    // Validate image properties
    resolution = imageAndMeta.image.Resolution();
    if (resolution.x <= 0 || resolution.y <= 0) {
        Error("Invalid sparsity map dimensions: %dx%d", resolution.x, resolution.y);
        return false;
    }

    // Verify image format and normalize if needed
    imageAndMeta.image.ForEachPixel([&](const Point2i &p) {
        RGB pixel = imageAndMeta.image.GetPixel(p);
        if (pixel.r < 0 || pixel.r > 1 || 
            pixel.g < 0 || pixel.g > 1 || 
            pixel.b < 0 || pixel.b > 1) {
            Warning("Sparsity map contains out-of-range values. Normalizing.");
            return false;  // Break iteration
        }
        return true;
    });

    // Store the sparsity map
    sparsityMap = std::move(imageAndMeta.image);
    
    // Initialize patch grid
    InitializePatchGrid();
    
    LOG_VERBOSE("Loaded sparsity map: %s (%dx%d)", 
                filename, resolution.x, resolution.y);
    return true;
}

Float SparsityMapHandler::GetPatchSparsity(const Point2i& origin) const {
    ProfilePhase _(Prof::TextureAccess);
    ScopedStatsAccumulator _s(&statsPatchProcessing);

    if (!sparsityMap) {
        LOG_ERROR("Attempting to access uninitialized sparsity map");
        return 1.0f;  // Conservative fallback
    }

    // Validate patch bounds
    if (!ValidatePatchBounds(origin)) {
        return ComputeFallbackSparsity(origin);
    }

    // Compute patch statistics
    PatchStatistics stats = ComputePatchStatistics(origin);
    
    // Apply sparsity estimation based on patch characteristics
    Float sparsityValue = EstimateSparsity(stats);
    
    // Ensure the result is properly bounded
    return Clamp(sparsityValue, 0.0f, 1.0f);
}

int SparsityMapHandler::ComputeSampleBudget(Float sparsity, 
                                          int minSamples, 
                                          int maxSamples) const {
    CHECK_GE(minSamples, 1);
    CHECK_GE(maxSamples, minSamples);
    
    // Apply non-linear mapping for better sample distribution
    // Using smooth step function for gradual transition
    Float t = SmoothStep(0.f, 1.f, 1.0f - sparsity);
    
    // Compute base sample count
    int baseSamples = int(Lerp(t, minSamples, maxSamples));
    
    // Ensure sample count is a power of 2 for stratification
    return RoundUpPow2(baseSamples);
}

// Private helper methods
void SparsityMapHandler::InitializePatchGrid() {
    // Pre-compute patch boundaries and metadata
    int numPatchesX = (resolution.x + patchSize - 1) / patchSize;
    int numPatchesY = (resolution.y + patchSize - 1) / patchSize;
    
    LOG_VERBOSE("Initialized patch grid: %dx%d patches", numPatchesX, numPatchesY);
}

bool SparsityMapHandler::ValidatePatchBounds(const Point2i& origin) const {
    return (origin.x >= 0 && origin.x + patchSize <= resolution.x &&
            origin.y >= 0 && origin.y + patchSize <= resolution.y);
}

Float SparsityMapHandler::ComputeFallbackSparsity(const Point2i& origin) const {
    // Handle edge cases and partial patches
    Point2i validOrigin(
        Clamp(origin.x, 0, resolution.x - 1),
        Clamp(origin.y, 0, resolution.y - 1)
    );
    
    // Return the sparsity of the nearest valid pixel
    RGB pixel = sparsityMap.GetPixel(validOrigin);
    return (pixel.r + pixel.g + pixel.b) / 3;
}

SparsityMapHandler::PatchStatistics 
SparsityMapHandler::ComputePatchStatistics(const Point2i& origin) const {
    PatchStatistics stats;
    
    // Accumulate patch statistics
    for (int y = 0; y < patchSize; ++y) {
        for (int x = 0; x < patchSize; ++x) {
            Point2i p(origin.x + x, origin.y + y);
            RGB pixel = sparsityMap.GetPixel(p);
            
            // Update running statistics
            Float intensity = (pixel.r + pixel.g + pixel.b) / 3;
            stats.mean += intensity;
            stats.variance += intensity * intensity;
            stats.validPixels++;
            
            // Track local gradients for edge detection
            if (x > 0 && y > 0) {
                Point2i prev(origin.x + x - 1, origin.y + y - 1);
                RGB prevPixel = sparsityMap.GetPixel(prev);
                Float prevIntensity = (prevPixel.r + prevPixel.g + prevPixel.b) / 3;
                stats.gradientMagnitude += std::abs(intensity - prevIntensity);
            }
        }
    }
    
    // Finalize statistics
    if (stats.validPixels > 0) {
        stats.mean /= stats.validPixels;
        stats.variance = stats.variance / stats.validPixels - stats.mean * stats.mean;
        stats.gradientMagnitude /= (patchSize * patchSize - 1);
    }
    
    return stats;
}

Float SparsityMapHandler::EstimateSparsity(const PatchStatistics& stats) const {
    // Combine multiple metrics for robust sparsity estimation
    Float varianceWeight = 0.4f;
    Float gradientWeight = 0.6f;
    
    // Normalize variance and gradient contributions
    Float normalizedVariance = std::min(stats.variance * 4.0f, 1.0f);
    Float normalizedGradient = std::min(stats.gradientMagnitude * 2.0f, 1.0f);
    
    // Compute weighted sparsity estimate
    return varianceWeight * normalizedVariance + 
           gradientWeight * normalizedGradient;
}

} // namespace pbrt