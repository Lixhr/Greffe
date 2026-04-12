#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include "ida.hpp"

struct PatchRegion {
    ea_t     base;
    ea_t     end;
    uint64_t size() const { return end - base; }
};

// Cursor over an ordered list of patch regions.
// Allocations are contiguous within a single region
// when a region fills up the cursor moves to the next one.
class RegionCursor {
public:
    explicit RegionCursor(const std::vector<PatchRegion>& regions);

    ea_t alloc(uint8_t alignment, uint64_t size);

    void align(uint8_t alignment);

    bool fits(uint64_t size) const;

    // (caller guarantees it fits)
    void advance(uint64_t size);

    // Throws std::runtime_error if no more regions are available.
    void next_region();

    ea_t current_addr() const;

    void reset();

private:
    uint64_t aligned_intra(uint8_t alignment) const;

    const std::vector<PatchRegion>& _regions;
    size_t   _region_idx   = 0;
    uint64_t _intra_offset = 0;
};