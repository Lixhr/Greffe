#include "PatchLayoutEntry.hpp"
#include "utils.hpp"

void PatchLayoutEntry::set_offset(uint64_t offset) { _offset = offset; }
void PatchLayoutEntry::set_addr(ea_t addr)         { _addr   = addr;   }

uint64_t PatchLayoutEntry::offset() const { return _offset; }
ea_t     PatchLayoutEntry::addr()   const { return _addr;   }

const std::vector<uint8_t>& PatchLayoutEntry::bytes() const { return _bytes; }
std::vector<uint8_t>&       PatchLayoutEntry::bytes()       { return _bytes; }

void PatchLayoutEntry::set_color(bgcolor_t color) const {
    set_range_color(_addr, _addr + _bytes.size(), color);
}

