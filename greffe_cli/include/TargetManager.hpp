#pragma once

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include "Target.hpp"
#include "IdaIPC.hpp"

class ProjectInfo;

struct SavedProject {
    std::optional<uint64_t> bin_base;
    std::optional<uint64_t> patch_base;
};

class TargetManager {
    public:
        explicit TargetManager(IdaIPC& ipc);

        const Target&       add(const std::string& target, const ProjectInfo& pinfo);
        bool                add_direct(const json& entry, const ProjectInfo& pinfo);
        void                remove(const std::string& target);
        std::vector<Target> targets() const;

        void         save(const std::filesystem::path& path,
                          uint64_t                     bin_base,
                          std::optional<uint64_t>      patch_base) const;
        SavedProject load(const std::filesystem::path& path, const ProjectInfo& pinfo);

    private:
        json                             fetch_entry(const std::string& target);
        static std::vector<ContextEntry> parse_context(const json& entry);
        static void                      validate_context_modes(const std::string& target,
                                                                uint64_t ea,
                                                                const std::vector<ContextEntry>& context);

        std::pair<Target*, bool>         add_internal(const json& entry,
                                                      std::vector<ContextEntry> context,
                                                      const ProjectInfo& pinfo);

        std::pair<Target*, bool>         append_target(const json& entry,
                                                       std::vector<ContextEntry> context,
                                                       const ProjectInfo& pinfo);

        IdaIPC&              _ipc;
        std::vector<Target>  _targets;
        mutable std::mutex   _mutex;
};
