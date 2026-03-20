#pragma once

#include "HandlerBin.hpp"
#include "ProjectInfo.hpp"
#include "Target.hpp"
#include <vector>

namespace HandlerCompiler {
    HandlerBin build(const std::vector<Target>& targets, const ProjectInfo& pinfo);
}
