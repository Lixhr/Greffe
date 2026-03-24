#pragma once

#include "IdaIPC.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include <filesystem>
#include <optional>

struct CLIContext {
    IdaIPC&                  client;
    ProjectInfo&             pinfo;
    TargetManager            targets;
    bool                     running     = true;

    uint64_t                 bin_base;
    std::optional<uint64_t>  patch_base;

    CLIContext(IdaIPC& client, ProjectInfo& pinfo)
        : client(client), pinfo(pinfo), targets(client), bin_base(pinfo.getBinBase()) {
        auto greffe = pinfo.getProjectDir() / ".greffe";
        if (std::filesystem::exists(greffe)) {
            auto cfg = targets.load(greffe);
            if (cfg.bin_base)   bin_base   = *cfg.bin_base;
            if (cfg.patch_base) patch_base = *cfg.patch_base;
        }
    }
};

using Args = std::vector<std::string>;