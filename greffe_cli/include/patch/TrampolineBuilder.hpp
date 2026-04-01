#pragma once

#include "PatchPlan.hpp"
#include "PatchSession.hpp"
#include "arch/IArchStubs.hpp"
#include "arch/IRelocator.hpp"
#include "SharedStub.hpp"
#include <cstdint>
#include <vector>

namespace TrampolineBuilder {

    void                 branch_init(PatchPlan& plan);
    void                 patch_branches(PatchSession& session, const std::vector<PatchPlan>& plans);

    std::vector<uint8_t> init_trampoline(PatchPlan& plan,
                                         const SharedStub &shstub);
}
