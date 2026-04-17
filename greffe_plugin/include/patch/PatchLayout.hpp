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

        PatchLayoutEntry*                add_entry(unique_ple_t entry);
        void                             create_patch_entry(PatchPlan *plan);
        void                             place_handler_bin();


        const std::vector<SharedStub*>   shstubs()          const;
        const std::vector<PatchPlan*>    patch_plans()      const;
        const std::vector<PatchBranch*>  branches()         const;
        const std::vector<HandlerBin*>   handlers()         const;

        template <typename F>
        PatchLayoutEntry* entry_find_if(F&& fn) const {
            for (const auto& e : _entries)
                if (fn(*e))
                    return e.get();
            return nullptr;
        }

        
        bool                             overlaps_any(ea_t s, ea_t e) const;

    private:
        const SharedStub*  get_shstub(PatchPlan *plan);
        const SharedStub*  create_shstub(PatchPlan *plan);

        ProjectInfo&              _pinfo;

        // PatchLayoutEntries
        // std::vector<PatchPlan>          _patch_plans;
        // std::vector<SharedStub>         _shstubs;
        // std::vector<PatchBranch>        _branches;
        // HandlerBin                      _handlerbin;
        PatchRegionSet&                 _regions;
        std::vector<unique_ple_t>       _entries;
};
