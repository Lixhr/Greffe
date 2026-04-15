#include "PatchLayoutEntry.hpp"
#include "utils.hpp"
#include "name.hpp"

void PatchLayoutEntry::set_offset(uint64_t offset) { _offset = offset; }
void PatchLayoutEntry::set_addr(ea_t addr)         { _addr   = addr;   }

uint64_t PatchLayoutEntry::offset() const { return _offset; }
ea_t     PatchLayoutEntry::addr()   const { return _addr;   }

const std::vector<uint8_t>& PatchLayoutEntry::bytes() const { return _bytes; }
std::vector<uint8_t>&       PatchLayoutEntry::bytes()       { return _bytes; }

void PatchLayoutEntry::add_label(ea_t ea, const char *str) {
    set_name(ea, str);
    _labels.push_back(ea);
}

void PatchLayoutEntry::add_label(const char *str) {
    set_name(_addr, str);
    _labels.push_back(_addr);
}

void PatchLayoutEntry::clear_labels() {
    for (auto l_ea: _labels) {
        del_global_name(l_ea);
    }
    _labels.clear();
}

PatchLayoutEntry::~PatchLayoutEntry() {
    clear_labels();
}