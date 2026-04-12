#include "patch/RegionCursor.hpp"
#include <sstream>
#include <stdexcept>

RegionCursor::RegionCursor(const std::vector<PatchRegion>& regions)
    : _regions(regions) {}

uint64_t RegionCursor::aligned_intra(uint8_t alignment) const {
    if (alignment <= 1)
        return _intra_offset;
    return (_intra_offset + alignment - 1) & ~static_cast<uint64_t>(alignment - 1);
}

ea_t RegionCursor::current_addr() const {
    if (_region_idx >= _regions.size())
        throw std::runtime_error("RegionCursor: no patch regions defined");
    return _regions[_region_idx].base + _intra_offset;
}

void RegionCursor::align(uint8_t alignment) {
    if (_region_idx >= _regions.size())
        throw std::runtime_error("RegionCursor: no patch regions defined");
    uint64_t aligned = aligned_intra(alignment);
    if (aligned >= _regions[_region_idx].size())
        next_region();  // alignment pushed past region end, spill into next
    else
        _intra_offset = aligned;
}

bool RegionCursor::fits(uint64_t size) const {
    if (_region_idx >= _regions.size())
        return false;
    return _intra_offset + size <= _regions[_region_idx].size();
}

void RegionCursor::advance(uint64_t size) {
    _intra_offset += size;
}

void RegionCursor::next_region() {
    if (_region_idx + 1 >= _regions.size())
        throw std::runtime_error("RegionCursor: all patch regions exhausted");
    ++_region_idx;
    _intra_offset = 0;
}

ea_t RegionCursor::alloc(uint8_t alignment, uint64_t size) {
    while (_region_idx < _regions.size()) {
        uint64_t offset = aligned_intra(alignment);
        if (offset + size <= _regions[_region_idx].size()) {
            _intra_offset  = offset;
            ea_t addr      = _regions[_region_idx].base + offset;
            _intra_offset += size;
            return addr;
        }
        ++_region_idx;
        _intra_offset = 0;
    }
    throw std::runtime_error("RegionCursor: all patch regions exhausted");
}

void RegionCursor::reset() {
    _region_idx   = 0;
    _intra_offset = 0;
}