#pragma once

#include "ida.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
#include "bytes.hpp"
#include "utils.hpp"
#include "xref.hpp"
#include "name.hpp"
#include "auto.hpp"

struct PatchRegion {
    ea_t base;
    ea_t end;

    struct raw_t {};

    PatchRegion(ea_t b, ea_t e);
    PatchRegion(ea_t b, ea_t e, raw_t) : base(b), end(e) {}

    uint64_t size()              const { return end - base; }
    bool     overlaps(ea_t s, ea_t e) const { return s < end && e > base; }
    bool     contains(ea_t addr) const { return addr >= base && addr < end; }
    void     clear_region();
};


class PatchRegionSet {
    public:
        void add_region(ea_t start, ea_t end);
        void reclaim(ea_t start, ea_t end);
        bool has_regions() const { return !_regions.empty(); }

        const std::vector<PatchRegion>& regions()         const { return _regions; }
        std::vector<PatchRegion>&       regions_mutable()       { return _regions; }

        bool overlaps_any(ea_t s, ea_t e) const;
        ea_t alloc_best_fit(uint8_t alignment, uint64_t size);
        ea_t alloc_largest(uint8_t alignment, uint64_t size);
        void commit_alloc(ea_t addr, uint64_t size);

    private:
        void order_insert(ea_t start, ea_t end);
        void raw_insert(ea_t start, ea_t end);
        void interval_subtraction(std::vector<PatchRegion>::iterator it, ea_t start, ea_t end);
        void merge_regions();
        ea_t split_alloc(std::vector<PatchRegion>::iterator it, ea_t aligned, uint64_t size);

        std::vector<PatchRegion> _regions;
};