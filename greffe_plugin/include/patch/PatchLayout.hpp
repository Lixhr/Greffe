#pragma once

#include <cstdint>
#include <vector>
#include "PatchPlan.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include "patch/SharedStub.hpp"
#include "patch/PatchBranch.hpp"
#include "PatchRegionSet.hpp"
#include "HandlerBin.hpp"

class TargetManager;

class PatchLayout {
    public:
        PatchLayout(ProjectInfo& pinfo, TargetManager& targets);

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

        ProjectInfo&              _pinfo;

        // PatchLayoutEntries
        std::vector<PatchPlan>&   _patch_plans;
        std::vector<SharedStub>   _shstubs;
        std::vector<PatchBranch>  _branches;

        PatchRegionSet&           _regions;
};
