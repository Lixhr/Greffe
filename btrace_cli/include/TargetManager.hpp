#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "Target.hpp"
#include "IdaIPC.hpp"

class TargetManager {
    public:
        explicit TargetManager(IdaIPC& ipc);

        const Target& add(const std::string& target);
        void remove(const std::string& target);
        const std::vector<Target>& targets() const;

        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);

    private:
        IdaIPC&             _ipc;
        std::vector<Target> _targets;
};
