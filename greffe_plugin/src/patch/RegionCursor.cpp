#include "patch/RegionCursor.hpp"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>

RegionCursor::RegionCursor(std::vector<PatchRegion>& regions)
    : _regions(regions)
{}

PatchRegion& RegionCursor::current_region() {
    if (_order_idx >= _order.size())
        throw std::runtime_error("RegionCursor: all patch regions exhausted");
    return _regions[_order[_order_idx]];
}

const PatchRegion& RegionCursor::current_region() const {
    if (_order_idx >= _order.size())
        throw std::runtime_error("RegionCursor: all patch regions exhausted");
    return _regions[_order[_order_idx]];
}

void RegionCursor::select_closest(ea_t target) {
    if (_regions.empty())
        throw std::runtime_error("RegionCursor: no patch regions defined");

    _order.resize(_regions.size());
    std::iota(_order.begin(), _order.end(), 0);
    std::stable_sort(_order.begin(), _order.end(), [&](size_t a, size_t b) {
        ea_t ca = _regions[a].cursor, cb = _regions[b].cursor;
        auto dist = [target](ea_t v) { return v > target ? v - target : target - v; };
        return dist(ca) < dist(cb);
    });

    _order_idx           = 0;
    _align_pending       = false;
    _cursor_before_align = 0;
}

ea_t RegionCursor::current_addr() const {
    return current_region().cursor;
}

void RegionCursor::align(uint8_t alignment) {
    auto& r              = current_region();
    _cursor_before_align = r.cursor;
    _align_pending       = true;

    if (alignment > 1)
        r.cursor = (r.cursor + alignment - 1) & ~static_cast<ea_t>(alignment - 1);

    if (r.cursor >= r.end) {
        r.cursor       = _cursor_before_align; // restore before leaving region
        _align_pending = false;
        next_region();
    }
}

bool RegionCursor::fits(uint64_t size) const {
    const auto& r = current_region();
    return r.cursor + size <= r.end;
}

void RegionCursor::advance(uint64_t size) {
    current_region().cursor += size;
    _align_pending       = false;
    _cursor_before_align = 0;
}

void RegionCursor::next_region() {
    // Roll back any speculative alignment on the region we're leaving.
    if (_align_pending) {
        _regions[_order[_order_idx]].cursor = _cursor_before_align;
        _align_pending       = false;
        _cursor_before_align = 0;
    }

    ++_order_idx;
    if (_order_idx >= _order.size())
        throw std::runtime_error("RegionCursor: all patch regions exhausted");
}

ea_t RegionCursor::alloc(uint8_t alignment, uint64_t size) {
    for (auto& r : _regions) {
        ea_t candidate = r.cursor;
        if (alignment > 1)
            candidate = (candidate + alignment - 1) & ~static_cast<ea_t>(alignment - 1);
        if (candidate + size <= r.end) {
            r.cursor = candidate + size;
            return candidate;
        }
    }
    throw std::runtime_error("RegionCursor: all patch regions exhausted");
}

void RegionCursor::reset() {
    if (_regions.empty())
        throw std::runtime_error("RegionCursor: no patch regions defined");
    for (auto& r : _regions)
        r.cursor = r.base;
    _order.clear();
    _order_idx           = 0;
    _align_pending       = false;
    _cursor_before_align = 0;
}