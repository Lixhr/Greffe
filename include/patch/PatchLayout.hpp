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
        HandlerBin *                     place_handler_bin();
        void                             free_handler_bin();


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
        void entries_delete_if(F&& fn) {
            auto erase = [&](std::vector<unique_ple_t>& vec) {
                vec.erase(std::remove_if(vec.begin(), vec.end(),
                    [&](const unique_ple_t& e) { return fn(*e); }),
                    vec.end());
            };
            erase(_entries);
        }

        template <typename F>
        void free_if(F&& fn) {
            std::vector<std::pair<ea_t, ea_t>> freed;
            for (const auto& e : _entries)
                if (fn(*e))
                    freed.emplace_back(e->ea(), e->end_ea());
            _entries.erase(std::remove_if(_entries.begin(), _entries.end(),
                [&](const unique_ple_t& e) { return fn(*e); }),
                _entries.end());

            for (auto& [start, end] : freed) {
                free_entry(start, end);
            }
        }

        template <typename F>
        void foreach_queue(F&& fn) const {
            for (const auto& e : _queue)
                fn(*e);
        }

        void                             commit();
        void                             rollback();
        void                             sort_queue_by_type();
        bool                             overlaps_any(ea_t s, ea_t e) const;

    private:
        const SharedStub*  get_shstub(PatchPlan *plan);
        const SharedStub*  create_shstub(PatchPlan *plan);

        bool overlaps_vec(const std::vector<unique_ple_t>& vec, ea_t s, ea_t e) const;
        void free_entry(ea_t start, ea_t end);

        ProjectInfo&              _pinfo;
        PatchRegionSet&           _regions;
        std::vector<unique_ple_t> _entries;
        std::vector<unique_ple_t> _queue;
};
