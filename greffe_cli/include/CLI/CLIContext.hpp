#pragma once

#include "IdaIPC.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include "cli_fmt.hpp"
#include <filesystem>
#include <stdexcept>

struct CLIContext {
    IdaIPC&                  client;
    ProjectInfo&             pinfo;
    TargetManager            targets;
    bool                     running     = true;
    uint64_t                 bin_base;

    CLIContext(IdaIPC& client, ProjectInfo& pinfo);
};

using Args = std::vector<std::string>;