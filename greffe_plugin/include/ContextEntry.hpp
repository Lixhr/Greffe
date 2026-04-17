#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "ida.hpp"

struct ContextEntry {
    ea_t                 ea;
    std::vector<uint8_t> raw;
    std::string          mode;
    bool                 is_xref_target = false;
};
