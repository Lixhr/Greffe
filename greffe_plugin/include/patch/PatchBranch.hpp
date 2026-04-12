#pragma once

#include "patch/PatchLayoutEntry.hpp"
#include <cstdint>
#include <vector>

class PatchBranch : public PatchLayoutEntry {
    public:
        PatchBranch() = default;
        PatchBranch(ea_t ea, std::vector<uint8_t> bytes) {
            _addr  = ea;
            _bytes = std::move(bytes);
        }
};
