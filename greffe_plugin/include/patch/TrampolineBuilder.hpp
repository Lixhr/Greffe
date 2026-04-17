#pragma once

#include "PatchPlan.hpp"
#include "arch/IArchStubs.hpp"
#include "SharedStub.hpp"
#include "PatchBranch.hpp"
#include <cstdint>
#include <vector>

namespace TrampolineBuilder {

    PatchBranch          branch_to_trampoline(PatchPlan& plan);

    size_t               init_trampoline(PatchPlan& plan,
                                         const SharedStub &shstub);

    size_t               relocate_and_branch_back(PatchPlan& plan);
}
