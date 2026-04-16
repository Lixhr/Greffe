#include "PatchRegionSet.hpp"
#include "utils.hpp"
#include <bytes.hpp>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include "GreffeCTX.hpp"

void PatchRegionSet::order_insert(ea_t start, ea_t end) {
    auto pos = std::lower_bound(_regions.begin(), _regions.end(), start,
        [](const PatchRegion& p, ea_t val) { return p.base < val; });

    if (g_ctx && g_ctx->layout.overlaps_any(start, end)) {
        std::ostringstream ss;
        ss << "Patch region [0x" << std::hex << start
           << ", 0x" << end << ") overlaps an existing patch layout entry";
        throw std::runtime_error(ss.str());
    }

    _regions.insert(pos, PatchRegion(start, end));
}

void PatchRegionSet::interval_subtraction(std::vector<PatchRegion>::iterator it,
                                          ea_t start, ea_t end) {
    std::vector<std::pair<ea_t, ea_t>> to_insert;
    ea_t cursor = start;

    while (it != _regions.end() && it->base < end) {
        if (cursor < it->base)
            to_insert.emplace_back(cursor, it->base);
        if (it->end > cursor)
            cursor = it->end;
        ++it;
    }

    if (cursor < end)
        to_insert.emplace_back(cursor, end);

    for (auto& [s, e] : to_insert)
        order_insert(s, e);
}

void PatchRegionSet::merge_regions() {
    auto it = _regions.begin();
    while (it != _regions.end()) {
        auto next = std::next(it);
        if (next == _regions.end())
            break;
        if (it->end >= next->base) {
            it->end = std::max(it->end, next->end);
            _regions.erase(next);
        } else {
            ++it;
        }
    }
}

void PatchRegionSet::add_region(ea_t start, ea_t end) {
    auto it = std::lower_bound(_regions.begin(), _regions.end(), start,
        [](const PatchRegion& p, ea_t val) { return p.base < val; });

    if (it != _regions.begin() && std::prev(it)->end > start)
        --it;

    if (it != _regions.end() && it->base < end)
        interval_subtraction(it, start, end);
    else
        order_insert(start, end);

    merge_regions();
}




PatchRegion& PatchRegionSet::current_region() {
    if (_order_idx >= _order.size())
        throw std::runtime_error("PatchRegionSet: all patch regions exhausted");
    return _regions[_order[_order_idx]];
}

const PatchRegion& PatchRegionSet::current_region() const {
    if (_order_idx >= _order.size())
        throw std::runtime_error("PatchRegionSet: all patch regions exhausted");
    return _regions[_order[_order_idx]];
}

void PatchRegionSet::select_closest(ea_t target) {
    if (_regions.empty())
        throw std::runtime_error("PatchRegionSet: no patch regions defined");

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

ea_t PatchRegionSet::current_addr() const {
    return current_region().cursor;
}

bool PatchRegionSet::fits(uint64_t size) const {
    return current_region().fits(size);
}

void PatchRegionSet::advance(uint64_t size) {
    current_region().advance(size);
    _align_pending       = false;
    _cursor_before_align = 0;
}

void PatchRegionSet::align(uint8_t alignment) {
    auto& r              = current_region();
    _cursor_before_align = r.cursor;
    _align_pending       = true;

    if (alignment > 1)
        r.cursor = (r.cursor + alignment - 1) & ~static_cast<ea_t>(alignment - 1);

    if (r.cursor >= r.end) {
        r.cursor       = _cursor_before_align;
        _align_pending = false;
        next_region();
    }
}

void PatchRegionSet::next_region() {
    if (_align_pending) {
        _regions[_order[_order_idx]].cursor = _cursor_before_align;
        _align_pending       = false;
        _cursor_before_align = 0;
    }

    ++_order_idx;
    if (_order_idx >= _order.size())
        throw std::runtime_error("PatchRegionSet: all patch regions exhausted");
}

ea_t PatchRegionSet::alloc(uint8_t alignment, uint64_t size) {
    for (auto& r : _regions) {
        ea_t candidate = r.cursor;
        if (alignment > 1)
            candidate = (candidate + alignment - 1) & ~static_cast<ea_t>(alignment - 1);
        if (candidate + size <= r.end) {
            r.cursor = candidate + size;
            return candidate;
        }
    }
    throw std::runtime_error("PatchRegionSet: all patch regions exhausted");
}

bool PatchRegionSet::overlaps_any(ea_t s, ea_t e) const {
    for (const auto& r : _regions)
        if (r.overlaps(s, e) || (s < r.base && e > r.end))
            return true;
    return false;
}
