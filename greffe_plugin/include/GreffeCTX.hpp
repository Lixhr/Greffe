#pragma once

#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include "PatchLayout.hpp"
#include <filesystem>
#include <stdexcept>

class PatchLayout;
class TargetManager;

struct GreffeCTX {
    ProjectInfo   pinfo;
    TargetManager targets;
    PatchLayout   layout;
    bool          running = true;

    public:
        GreffeCTX();
};

extern std::unique_ptr<GreffeCTX> g_ctx;

