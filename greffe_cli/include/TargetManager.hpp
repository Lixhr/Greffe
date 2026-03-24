#pragma once

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include "Target.hpp"
#include "IdaIPC.hpp"

struct SavedProject {
    std::optional<uint64_t> bin_base;
    std::optional<uint64_t> patch_base;
};

class TargetManager {
    public:
        explicit TargetManager(IdaIPC& ipc);

        const Target&       add(const std::string& target);
        bool                add_direct(const json& entry);
        void                remove(const std::string& target);
        std::vector<Target> targets() const;

        void         save(const std::filesystem::path& path,
                          uint64_t                     bin_base,
                          std::optional<uint64_t>      patch_base) const;
        SavedProject load(const std::filesystem::path& path);

    private:
        IdaIPC&              _ipc;
        std::vector<Target>  _targets;
        mutable std::mutex   _mutex;
};
