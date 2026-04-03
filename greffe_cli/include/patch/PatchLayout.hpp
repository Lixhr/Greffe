#pragma once

#include <cstdint>
#include <vector>
#include "PatchPlan.hpp"
#include "ProjectInfo.hpp"
#include "TargetManager.hpp"
#include "SharedStub.hpp"

class PatchLayout {
    public:
        PatchLayout(const ProjectInfo& pinfo, TargetManager& targets);

        void                           create_patch_entry(PatchPlan *plan);
        
        const std::vector<SharedStub>& shstubs()                       const;
        const std::vector<PatchPlan> & patch_plans()                   const;
        uint64_t                       offset_to_addr(uint64_t offset) const;

    private:
        void                            set_trampoline_addr(PatchPlan* plan);
        const SharedStub                *get_shstub(PatchPlan *plan);
        const SharedStub                *create_shstub(PatchPlan *plan);
        

        const ProjectInfo&                _pinfo;
        const std::vector<PatchPlan>&     _patch_plans;
        uint64_t                          _patch_offset;
        std::vector<SharedStub>           _shstubs;

};