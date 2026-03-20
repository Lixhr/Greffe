#pragma once

#include "Target.hpp"
#include "ProjectInfo.hpp"
#include "arch/IRelocator.hpp"
#include <filesystem>
#include <vector>
#include <cstdint>

struct PatchResult {
    RelocatedCode  reloc;
    uint64_t       trampoline_addr;
    uint64_t       file_offset;
};

namespace Patcher {
    std::vector<uint8_t> collect_input(const Target& t);
    PatchResult apply(const Target&                t,
                      uint64_t                     trampoline_addr,
                      uint64_t                     file_offset,
                      const ProjectInfo&           pinfo,
                      const std::filesystem::path& outfile);

}
