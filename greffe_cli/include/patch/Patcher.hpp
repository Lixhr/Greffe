#pragma once

#include "Target.hpp"
#include <vector>
#include <cstdint>

namespace Patcher {
    std::vector<uint8_t> collect_input(const Target& t);
}
