#pragma once

#include <cstdint>
#include <vector>
#include "PatchPlan.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include "patch/HandlerBin.hpp"
#include "patch/SharedStub.hpp"
#include "patch/PatchBranch.hpp"
#include "patch/RegionCursor.hpp"

class TargetManager;

class PatchLayout {
    public:
        PatchLayout(const ProjectInfo& pinfo, TargetManager& targets);

        void                             create_patch_entry(PatchPlan *plan);
        void                             rebuild();
        void                             place_handler_bin(HandlerBin& bin);

        const std::vector<SharedStub>&   shstubs()     const;
        const std::vector<PatchPlan>&    patch_plans() const;
        std::vector<PatchPlan>&          patch_plans();
        const std::vector<PatchBranch>&  branches()    const;

    private:
        const SharedStub*  get_shstub(PatchPlan *plan);
        const SharedStub*  create_shstub(PatchPlan *plan);
        void               insert_branch(PatchBranch branch);

        const ProjectInfo&        _pinfo;
        std::vector<PatchPlan>&   _patch_plans;
        RegionCursor              _cursor;
        std::vector<SharedStub>   _shstubs;
        std::vector<PatchBranch>  _branches;
};
