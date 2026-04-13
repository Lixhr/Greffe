#pragma once

#include "PatchRegionSet.hpp"
#include <filesystem>
#include <vector>
#include "ida.hpp"

struct ProjectInfo {
    public:
        ProjectInfo();

        void                              populateData();

        int                               getBits()          const { return bits; }
        uint64_t                          getBinBase()       const { return bin_base; }
        const std::filesystem::path&      getProjectDir()    const { return project_dir; }
        const std::filesystem::path&      getBinPath()       const { return bin_path; }
        const std::string&                getArch()          const { return arch; }
        const std::string&                getEndianness()    const { return endianness; }
        std::string                       getModeAt(ea_t ea) const;

        const PatchRegionSet&             getRegionsSet() const { return _regions; }
        PatchRegionSet&                   getRegionsSet()       { return _regions; }

    private:
        void    setupProjectDir();

        std::filesystem::path    bin_path;
        std::filesystem::path    project_dir;
        std::string              arch;
        std::string              endianness;
        int                      bits     = 0;
        uint64_t                 bin_base = 0;
        PatchRegionSet           _regions;
};