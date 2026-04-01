#pragma once

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include "PatchPlan.hpp"
#include "IdaIPC.hpp"

class ProjectInfo;

struct SavedProject {
    std::optional<uint64_t> bin_base;
    std::optional<uint64_t> patch_base;
};

class TargetManager {
    public:
        explicit                     TargetManager(IdaIPC& ipc);
      
        const PatchPlan&             add(const std::string& target, CLIContext& cxt);
        bool                         add_direct(const json& entry, CLIContext& cxt);
        void                         remove(const std::string& target);
        const std::vector<PatchPlan>& plans() const;

        void                         save(const std::filesystem::path& path,
                                         uint64_t                     bin_base,
                                         std::optional<uint64_t>      patch_base) const;
        SavedProject                 load(const std::filesystem::path& path, const ProjectInfo& pinfo);

    private:
        json                             fetch_entry(const std::string& target);
        static std::vector<ContextEntry> parse_context(const json& entry);
        static void                      validate_context_modes(const std::string& target,
                                                                uint64_t ea,
                                                                const std::vector<ContextEntry>& context);

        std::pair<PatchPlan*, bool>      add_internal(const json& entry,
                                                      CLIContext& cxt);

        std::pair<PatchPlan*, bool>      append_target(const json& entry,
                                                       std::vector<ContextEntry> context,
                                                       const ProjectInfo& pinfo);

        void                             set_trampoline_addr(PatchPlan* plan, uint32_t plan_index,
                                                             uint64_t patch_base);
        IdaIPC&                _ipc;
        std::vector<PatchPlan> _plans;
        mutable std::mutex     _mutex;
        uint32_t               _current_id = 0;
};
