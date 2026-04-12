#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include "ida.hpp"

struct PatchRegion {
    ea_t base;
    ea_t end;
    ea_t cursor;
    PatchRegion(ea_t b, ea_t e) : base(b), end(e), cursor(b) {}
    uint64_t size()      const { return end - base; }
    uint64_t remaining() const { return end - cursor; }
};

// Cursor over an ordered list of patch regions.
class RegionCursor {
    public:
        explicit RegionCursor(std::vector<PatchRegion>& regions);

        void select_closest(ea_t target);

        ea_t alloc(uint8_t alignment, uint64_t size);

        void align(uint8_t alignment);

        bool fits(uint64_t size) const;

        void advance(uint64_t size);

        void next_region();

        ea_t current_addr() const;

        void reset();

    private:
        PatchRegion&       current_region();
        const PatchRegion& current_region() const;

        std::vector<PatchRegion>& _regions;
        std::vector<size_t>       _order;                // indices into _regions, proximity-sorted
        size_t                    _order_idx           = 0;
        bool                      _align_pending       = false;
        ea_t                      _cursor_before_align = 0;
};
