#pragma once

// #include "IdaIPC.hpp"
#include <filesystem>
#include <optional>
#include <vector>
#include "ida.hpp"

struct ProjectInfo {
    public:
        ProjectInfo();

        void                         populateData(void);
        void                         setPatchBase(uint64_t base) { patch_base = base; };
        void                         initPatchBase(uint64_t bin_base);

        int                          getBits()       const { return bits; }
        uint64_t                     getBinBase()    const { return bin_base; }
        uint64_t                     getPatchBase()  const { return patch_base; }
        const std::filesystem::path& getProjectDir() const { return project_dir; }
        const std::filesystem::path& getBinPath()    const { return bin_path; }
        const std::string&           getArch()       const { return arch; }
        const std::string&           getEndianness() const { return endianness; }

    private:
        void setupProjectDir();

        std::filesystem::path   bin_path;
        std::filesystem::path   project_dir;
        std::string             arch;
        std::string             endianness;
        int                     bits;
        uint64_t                bin_base;
        uint64_t                patch_base = -1ULL;
};
