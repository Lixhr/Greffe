#pragma once

#include "ProjectInfo.hpp"
#include "patch/PatchLayout.hpp"

struct GreffeCTX {
    ProjectInfo   pinfo;
    PatchLayout   layout;

    GreffeCTX();
};

extern std::unique_ptr<GreffeCTX> g_ctx;
