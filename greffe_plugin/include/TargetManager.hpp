#pragma once

#include <string>
#include <vector>
#include "PatchPlan.hpp"
#include "ida.hpp"

class ProjectInfo;
class GreffeCTX;

class TargetManager {
    public:
        TargetManager() = default;

        void                          add(ea_t ea, GreffeCTX& ctx);
        // void                          remove(size_t idx, GreffeCTX& ctx);

    private:
        std::pair<PatchPlan*, bool>   append_target(ea_t                    ea,
                                                    ea_t                    end_ea,
                                                    const std::string&      name,
                                                    const ProjectInfo&      pinfo,
                                                    std::vector<PatchPlan>& plans);
        std::pair<PatchPlan*, bool>   add_internal(ea_t ea, GreffeCTX& ctx);
};
