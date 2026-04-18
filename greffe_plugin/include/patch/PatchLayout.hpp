#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "PatchPlan.hpp"
#include "patch/SharedStub.hpp"
#include "patch/PatchBranch.hpp"
#include "HandlerBin.hpp"

class ProjectInfo;
class PatchRegionSet;


class PatchLayout {
    public:
        PatchLayout(ProjectInfo& pinfo);

        PatchLayoutEntry*                queue_entry(unique_ple_t entry);
        void                             create_patch_entry(PatchPlan *plan);
        void                             place_handler_bin();


        const std::vector<SharedStub*>         shstubs()          const;
        const std::vector<PatchPlan*>          patch_plans()      const;
        const std::vector<PatchBranch*>        branches()         const;
        const std::vector<HandlerBin*>         handlers()         const;

        template <typename F>
        PatchLayoutEntry* entry_find_if(F&& fn) const {
            for (const auto& e : _entries)
                if (fn(*e))
                    return e.get();
            for (const auto& e : _queue)
                if (fn(*e))
                    return e.get();
            return nullptr;
        }

        template <typename F>
        void foreach_queue(F&& fn) const {
            for (const auto& e : _queue)
                fn(*e);
        }

        void                             commit();
        void                             flush();
        void                             sort_queue_by_type();
        bool                             overlaps_any(ea_t s, ea_t e) const;

    private:
        const SharedStub*  get_shstub(PatchPlan *plan);
        const SharedStub*  create_shstub(PatchPlan *plan);

        bool overlaps_vec(const std::vector<unique_ple_t>& vec, ea_t s, ea_t e) const;

        ProjectInfo&              _pinfo;
        PatchRegionSet&           _regions;
        std::vector<unique_ple_t> _entries;
        std::vector<unique_ple_t> _queue;
};
