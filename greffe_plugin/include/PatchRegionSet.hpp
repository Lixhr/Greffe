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
    ea_t cursor;
    ea_t old_cursor;

    PatchRegion(ea_t b, ea_t e) : base(b), end(e), cursor(b), old_cursor(b) {
        size_t size = end - base;

        std::vector<uint8_t> zeroes(size);
        write_data_patch(b, zeroes.data(), size, Color::PATCH_REGION);

        create_byte(b, e - b, true);

        for (ea_t i = b; i < e; i ++) {
            xrefblk_t xb;                                                                                                                                                                                 
            for ( bool ok = xb.first_to(i, XREF_ALL); ok; ok = xb.next_to() )                                                                                                                           
            {
                greffe_msg("%llx\n", xb.from);
                if (xb.iscode)
                    del_cref(xb.from, i, false);
                else
                    del_dref(xb.from, i);    
            } 
        }
    }

    uint64_t size()              const { return end - base; }
    uint64_t remaining()         const { return end - cursor; }
    ea_t     current_addr()      const { return cursor; }
    bool     fits(uint64_t size) const { return cursor + size <= end; }
    void     advance(uint64_t n)       { cursor += n; }

    bool overlaps(ea_t s, ea_t e) const { return s < end && e > base; }
    bool contains(ea_t addr)      const { return addr >= base && addr < end; }

    void refresh_data_items() {
        if (cursor < end)
            create_byte(cursor, end - cursor, true);
    }

    void commit_cursor() {
        old_cursor = cursor;
    }

    void reset_cursor() {
        cursor = old_cursor;
    }
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
        void commit();
        void rollback();

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
