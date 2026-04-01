#include "PatchLayoutEntry.hpp"

void PatchLayoutEntry::set_offset(uint64_t offset) { _offset = offset; }

uint64_t PatchLayoutEntry::offset() const { return _offset; }

uint64_t PatchLayoutEntry::addr() const { return _addr; }
