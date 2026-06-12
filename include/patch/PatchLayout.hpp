#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
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

        const std::vector<SharedStub*>&  shstubs()     const { return _shstubs_idx; }
        const std::vector<PatchPlan*>&   patch_plans() const { return _plans_idx;   }
        const std::vector<PatchBranch*>& branches()    const { return _branches_idx; }
        const std::vector<HandlerBin*>&  handlers()    const { return _handlers_idx; }

        PatchLayoutEntry*                entry_at(ea_t ea) const;

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
            std::vector<std::pair<ea_t, ea_t>> to_revert;
            size_t w = 0;
            for (size_t r = 0; r < _entries.size(); ++r) {
                auto& e = _entries[r];
                if (fn(*e)) {
                    to_revert.emplace_back(e->ea(), e->end_ea());
                    remove_from_type_idx(e.get());
                } else {
                    if (w != r) _entries[w] = std::move(_entries[r]);
                    ++w;
                }
            }
            _entries.erase(_entries.begin() + w, _entries.end());
            revert_entries(to_revert);
        }

        template <typename F>
        void free_if(F&& fn) {
            std::vector<std::pair<ea_t, ea_t>> freed;
            size_t w = 0;
            for (size_t r = 0; r < _entries.size(); ++r) {
                auto& e = _entries[r];
                if (fn(*e)) {
                    freed.emplace_back(e->ea(), e->end_ea());
                    remove_from_type_idx(e.get());
                } else {
                    if (w != r) _entries[w] = std::move(_entries[r]);
                    ++w;
                }
            }
            _entries.erase(_entries.begin() + w, _entries.end());

            for (auto& [start, end] : freed)
                free_entry(start, end);
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
        void                             load_from_db(netnode &node);
        void                             save_db(netnode &node);
    private:
        const SharedStub*  get_shstub(PatchPlan *plan);
        const SharedStub*  create_shstub(PatchPlan *plan);

        void add_to_type_idx(PatchLayoutEntry* e);
        void remove_from_type_idx(PatchLayoutEntry* e);

        bool overlaps_vec(const std::vector<unique_ple_t>& vec, ea_t s, ea_t e) const;
        void free_entry(ea_t start, ea_t end);
        void revert_entries(const std::vector<std::pair<ea_t, ea_t>>& ranges);

        ProjectInfo&              _pinfo;
        PatchRegionSet&           _regions;
        std::vector<unique_ple_t> _entries;
        std::vector<unique_ple_t> _queue;

        std::vector<PatchBranch*>                    _branches_idx;
        std::vector<PatchPlan*>                      _plans_idx;
        std::vector<SharedStub*>                     _shstubs_idx;
        std::vector<HandlerBin*>                     _handlers_idx;
        std::unordered_map<std::string, SharedStub*> _shstub_by_name;

        // True once a handler bin has been placed and its addresses resolved.
        // Gates free_handler_bin() plan/entry scan on every add
        bool                                         _handler_resolved = false;
};
