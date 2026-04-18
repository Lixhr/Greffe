#include "PatchRegionSet.hpp"
#include "utils.hpp"
#include <bytes.hpp>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include "GreffeCTX.hpp"

PatchRegion::PatchRegion(ea_t b, ea_t e) : base(b), end(e) {
    size_t sz = e - b;
    std::vector<uint8_t> zeroes(sz);
    write_data_patch(b, zeroes.data(), sz);
    create_byte(b, e - b, true);
    for (ea_t i = b; i < e; i++) {
        xrefblk_t xb;
        for (bool ok = xb.first_to(i, XREF_ALL); ok; ok = xb.next_to())
            if (xb.iscode) del_cref(xb.from, i, false);
            else           del_dref(xb.from, i);
    }
}

void PatchRegion::refresh_data_items() {
    create_byte(base, end - base, true);
}

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

void PatchRegionSet::raw_insert(ea_t start, ea_t end) {
    auto pos = std::lower_bound(_regions.begin(), _regions.end(), start,
        [](const PatchRegion& p, ea_t val) { return p.base < val; });
    _regions.insert(pos, PatchRegion(start, end, PatchRegion::raw_t{}));
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
            it->refresh_data_items();
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

void PatchRegionSet::reclaim(ea_t start, ea_t end) {
    raw_insert(start, end);
    merge_regions();
}

bool PatchRegionSet::overlaps_any(ea_t s, ea_t e) const {
    for (const auto& r : _regions)
        if (r.overlaps(s, e) || (s < r.base && e > r.end))
            return true;
    return false;
}

ea_t PatchRegionSet::split_alloc(std::vector<PatchRegion>::iterator it,
                                  ea_t aligned, uint64_t size) {
    ea_t region_base = it->base;
    ea_t region_end  = it->end;
    _regions.erase(it);

    if (aligned > region_base)
        raw_insert(region_base, aligned);
    if (aligned + size < region_end)
        raw_insert(aligned + size, region_end);

    return aligned;
}

void PatchRegionSet::commit_alloc(ea_t addr, uint64_t size) {
    for (auto it = _regions.begin(); it != _regions.end(); ++it) {
        if (it->base <= addr && it->end > addr) {
            split_alloc(it, addr, size);
            return;
        }
    }
    throw std::runtime_error("PatchRegionSet: commit_alloc: region not found");
}

ea_t PatchRegionSet::alloc_best_fit(uint8_t alignment, uint64_t size) {
    auto best         = _regions.end();
    ea_t best_aligned = 0;

    for (auto it = _regions.begin(); it != _regions.end(); ++it) {
        ea_t candidate = it->base;
        if (alignment > 1)
            candidate = (candidate + alignment - 1) & ~static_cast<ea_t>(alignment - 1);
        if (candidate + size > it->end)
            continue;
        if (best == _regions.end() || it->size() < best->size()) {
            best         = it;
            best_aligned = candidate;
        }
    }

    if (best == _regions.end())
        throw std::runtime_error("PatchRegionSet: all patch regions exhausted");

    return split_alloc(best, best_aligned, size);
}

ea_t PatchRegionSet::alloc_largest(uint8_t alignment, uint64_t size) {
    auto best         = _regions.end();
    ea_t best_aligned = 0;

    for (auto it = _regions.begin(); it != _regions.end(); ++it) {
        ea_t candidate = it->base;
        if (alignment > 1)
            candidate = (candidate + alignment - 1) & ~static_cast<ea_t>(alignment - 1);
        if (candidate + size > it->end)
            continue;
        if (best == _regions.end() || it->size() > best->size()) {
            best         = it;
            best_aligned = candidate;
        }
    }

    if (best == _regions.end())
        throw std::runtime_error("PatchRegionSet: all patch regions exhausted");

    return split_alloc(best, best_aligned, size);
}