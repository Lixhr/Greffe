#pragma once

#include "HandlerBin.hpp"
#include "ProjectInfo.hpp"
#include "PatchPlan.hpp"
#include <vector>

namespace HandlerCompiler {
    HandlerBin build(const std::vector<PatchPlan *> plans, const ProjectInfo& pinfo);
}
