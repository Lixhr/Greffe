#pragma once

#include "ida.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
#include "bytes.hpp"
#include "utils.hpp"


struct PatchRegion {
    ea_t base;
    ea_t end;
    ea_t cursor;

    PatchRegion(ea_t b, ea_t e) : base(b), end(e), cursor(b) {
        size_t size = end - base;

        std::vector<uint8_t> zeroes(size);
        write_data_patch(b, zeroes.data(), size, Color::PATCH_REGION);
    }

    uint64_t size()              const { return end - base; }
    uint64_t remaining()         const { return end - cursor; }
    ea_t     current_addr()      const { return cursor; }
    bool     fits(uint64_t size) const { return cursor + size <= end; }
    void     advance(uint64_t n)       { cursor += n; }

    bool overlaps(ea_t s, ea_t e) const { return s < end && e > base; }
    bool contains(ea_t addr)      const { return addr >= base && addr < end; }
};


class PatchRegionSet {
    public:
        void add_region(ea_t start, ea_t end);
        bool has_regions() const { return !_regions.empty(); }

        const std::vector<PatchRegion>& regions() const { return _regions; }

        void select_closest(ea_t target);
        void next_region();
        void align(uint8_t alignment);
        bool fits(uint64_t size)      const;
        void advance(uint64_t size);
        bool overlaps_any(ea_t s, ea_t e) const;
        ea_t current_addr()       const;
        ea_t alloc(uint8_t alignment, uint64_t size);

    private:
        void order_insert(ea_t start, ea_t end);
        void interval_subtraction(std::vector<PatchRegion>::iterator it, ea_t start, ea_t end);
        void merge_regions();

        PatchRegion&       current_region();
        const PatchRegion& current_region() const;

        std::vector<PatchRegion> _regions;
        std::vector<size_t>      _order;
        size_t                   _order_idx           = 0;
        bool                     _align_pending       = false;
        ea_t                     _cursor_before_align = 0;
};
