#include "PatchSession.hpp"
#include <stdexcept>

void PatchSession::run(const std::vector<Target>&  /*targets*/,
                       const HandlerBin&           /*handler_bin*/,
                       uint64_t                    /*patch_base*/,
                       uint64_t                    /*bin_base*/,
                       const ProjectInfo&          /*pinfo*/,
                       const std::filesystem::path&/*outfile*/  ) {
    throw std::runtime_error("PatchSession::run: not implemented");
}
