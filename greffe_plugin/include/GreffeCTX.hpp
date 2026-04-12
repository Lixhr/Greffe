#pragma once

#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include "patch/PatchLayout.hpp"

struct GreffeCTX {
    ProjectInfo   pinfo;
    TargetManager targets;
    PatchLayout   layout;

    GreffeCTX();
};

extern std::unique_ptr<GreffeCTX> g_ctx;
