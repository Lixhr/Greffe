#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

struct PatchRegion {
    uint64_t base;
    uint64_t end;
    uint64_t size() const { return end - base; }
};

// Cursor over an ordered list of patch regions.
// Allocations are contiguous within a single region; when a region fills up
// the cursor moves to the next one automatically.
class RegionCursor {
public:
    explicit RegionCursor(const std::vector<PatchRegion>& regions);

    // Align then reserve `size` bytes; return start address.
    // Skips to the next region if the current one lacks space.
    // Throws std::runtime_error if all regions are exhausted.
    uint64_t alloc(uint8_t alignment, uint64_t size);

    // Align the cursor position within the current region.
    void     align(uint8_t alignment);

    // True if `size` bytes fit at the current (already aligned) position.
    bool     fits(uint64_t size) const;

    // Advance the cursor by `size` bytes (caller guarantees it fits).
    void     advance(uint64_t size);

    // Skip to the beginning of the next region.
    // Throws std::runtime_error if no more regions are available.
    void     next_region();

    // Current absolute address.
    uint64_t current_addr() const;

    // Reset to the beginning of the first region.
    void     reset();

private:
    uint64_t aligned_intra(uint8_t alignment) const;

    const std::vector<PatchRegion>& _regions;
    size_t   _region_idx   = 0;
    uint64_t _intra_offset = 0;
};