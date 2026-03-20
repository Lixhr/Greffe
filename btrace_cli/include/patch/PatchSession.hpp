#pragma once

#include "Target.hpp"
#include "HandlerBin.hpp"
#include "ProjectInfo.hpp"
#include <filesystem>
#include <vector>

namespace PatchSession {

    void run(const std::vector<Target>&    targets,
             const HandlerBin&             handler_bin,
             uint64_t                      patch_base,
             uint64_t                      bin_base,
             const ProjectInfo&            pinfo,
             const std::filesystem::path&  outfile);

}
