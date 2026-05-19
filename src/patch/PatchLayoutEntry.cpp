#include "PatchLayoutEntry.hpp"
#include "utils.hpp"
#include "name.hpp"

void PatchLayoutEntry::set_addr(ea_t addr)         { _addr   = addr;   }

ea_t     PatchLayoutEntry::ea()     const { return _addr;   }
ea_t     PatchLayoutEntry::end_ea() const { return _addr + _bytes.size();}

const std::vector<uint8_t>& PatchLayoutEntry::bytes() const { return _bytes; }
std::vector<uint8_t>&       PatchLayoutEntry::bytes()       { return _bytes; }
PLEType                     PatchLayoutEntry::type()  const { return _type;  }
