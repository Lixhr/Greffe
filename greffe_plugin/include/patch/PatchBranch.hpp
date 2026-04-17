#pragma once

#include "patch/PatchLayoutEntry.hpp"
#include <cstdint>
#include <vector>

class PatchBranch : public PatchLayoutEntry {
    public:
        PatchBranch() : PatchLayoutEntry(PLEType::entry_branch) {}
        PatchBranch(ea_t ea, std::vector<uint8_t> bytes, ea_t ret_addr)
            : PatchLayoutEntry(PLEType::entry_branch), trampoline_ret_addr(ret_addr) {
            _addr  = ea;
            _bytes = std::move(bytes);
        }

        ea_t trampoline_ret_addr = 0;
};
