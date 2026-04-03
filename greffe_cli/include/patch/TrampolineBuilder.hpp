#pragma once

#include "PatchPlan.hpp"
#include "PatchSession.hpp"
#include "arch/IArchStubs.hpp"
#include "SharedStub.hpp"
#include "Target.hpp"
#include <cstdint>
#include <vector>

namespace TrampolineBuilder {

    void                 branch_to_trampoline(PatchPlan& plan);
    void                 patch_branches(PatchSession& session, const std::vector<PatchPlan>& plans);

    size_t               init_trampoline(PatchPlan& plan,
                                         const SharedStub &shstub);

    size_t               relocate_instructions(PatchPlan& plan);
    size_t               branch_back(PatchPlan& plan);
}
