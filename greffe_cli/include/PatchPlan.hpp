#pragma once

#include "Target.hpp"
#include "patch/arch/IArchStubs.hpp"
#include "patch/PatchLayoutEntry.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class PatchPlan : public PatchLayoutEntry {
    public:
        PatchPlan(Target t, std::shared_ptr<IArchStubs> s)
            : target(std::move(t)) { stubs = std::move(s); }

        Target                              target;
        uint64_t                            trampoline_ret_addr   = 0;
        std::vector<uint8_t>                branch_instr          = {};
        std::vector<const ContextEntry*>    relocd_instr          = {};
        size_t                              handler_offset = 0;
};