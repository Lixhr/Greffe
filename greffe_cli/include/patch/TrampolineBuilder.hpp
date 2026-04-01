#pragma once

#include "PatchPlan.hpp"
#include "PatchSession.hpp"
#include "arch/IArchStubs.hpp"
#include "arch/IRelocator.hpp"
#include <cstdint>
#include <vector>

namespace TrampolineBuilder {

    void                 branch_init(PatchPlan& plan);
    uint64_t             patch_branches(PatchSession& session, const std::vector<PatchPlan>& plans);

    std::vector<uint8_t> build(const PatchPlan& plan,
                               uint64_t         handler_addr,
                               IRelocator&      relocator);

}
