#pragma once

#include "Target.hpp"
#include "patch/arch/IArchStubs.hpp"
#include <cstdint>
#include <memory>
#include <vector>

struct PatchPlan {
    Target                      target;
    std::shared_ptr<IArchStubs> stubs;
    uint64_t                    trampoline_addr     = 0;
    uint64_t                    trampoline_ret_addr = 0;
    std::vector<uint8_t>        branch_instr          = {};
    std::vector<size_t>         relocd_instr_indices  = {}; // indices into target.context()
};