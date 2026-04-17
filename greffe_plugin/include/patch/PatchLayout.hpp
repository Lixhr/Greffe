#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "PatchPlan.hpp"
#include "ProjectInfo.hpp"
#include "patch/SharedStub.hpp"
#include "patch/PatchBranch.hpp"
#include "PatchRegionSet.hpp"
#include "HandlerBin.hpp"

class PatchLayout {
    public:
        PatchLayout(ProjectInfo& pinfo);

        void                             create_patch_entry(PatchPlan *plan);
        void                             place_handler_bin();
        void                             add_entry(std::unique_ptr<PatchLayoutEntry> entry);

        const std::vector<SharedStub>&   shstubs()          const;
        const std::vector<PatchPlan>&    patch_plans()      const;
        const std::vector<PatchBranch>&  branches()         const;
        const HandlerBin&                handler()          const;

        bool                             overlaps_any(ea_t s, ea_t e) const;

    private:
        const SharedStub*  get_shstub(PatchPlan *plan);
        const SharedStub*  create_shstub(PatchPlan *plan);
        void               insert_branch(PatchBranch branch);

        ProjectInfo&              _pinfo;

        // PatchLayoutEntries
        std::vector<PatchPlan>                          _patch_plans;
        std::vector<SharedStub>                         _shstubs;
        std::vector<PatchBranch>                        _branches;
        HandlerBin                                      _handlerbin;
        PatchRegionSet&                                 _regions;
        std::vector<std::unique_ptr<PatchLayoutEntry>>  _entries;
};
