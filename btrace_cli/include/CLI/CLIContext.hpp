#pragma once

#include "IdaIPC.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include <filesystem>

struct CLIContext {
    IdaIPC&         client;
    ProjectInfo&    pinfo;
    TargetManager   targets;
    bool            running = true;

    CLIContext(IdaIPC& client, ProjectInfo& pinfo)
        : client(client), pinfo(pinfo), targets(client) {
        auto btrace = pinfo.getProjectDir() / ".btrace";
        if (std::filesystem::exists(btrace))
            targets.load(btrace);
    }
};

using Args = std::vector<std::string>;