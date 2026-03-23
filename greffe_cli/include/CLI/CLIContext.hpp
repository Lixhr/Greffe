#pragma once

#include "IdaIPC.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include <filesystem>

struct CLIContext {
    IdaIPC&         client;
    ProjectInfo&    pinfo;
    TargetManager   targets;
    bool            running     = true;


    uint64_t        bin_base   = 0x00120000;
    uint64_t        patch_base = 0x0176A10;

    CLIContext(IdaIPC& client, ProjectInfo& pinfo)
        : client(client), pinfo(pinfo), targets(client) {
        auto greffe = pinfo.getProjectDir() / ".greffe";
        if (std::filesystem::exists(greffe))
            targets.load(greffe);
    }
};

using Args = std::vector<std::string>;