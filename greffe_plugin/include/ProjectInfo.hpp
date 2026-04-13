#pragma once

#include "patch/RegionCursor.hpp"
#include <filesystem>
#include <vector>
#include "ida.hpp"

struct ProjectInfo {
    public:
        ProjectInfo();

        void                              populateData();
        void                              add_region(ea_t start, ea_t end);

        bool                              has_regions()      const { return !_regions.empty(); }
        int                               getBits()          const { return bits; }
        uint64_t                          getBinBase()       const { return bin_base; }
        const std::filesystem::path&      getProjectDir()    const { return project_dir; }
        const std::filesystem::path&      getBinPath()       const { return bin_path; }
        const std::string&                getArch()          const { return arch; }
        const std::string&                getEndianness()    const { return endianness; }
        const std::vector<PatchRegion>&   getRegions() const { return _regions; }
              std::vector<PatchRegion>&   getRegions()       { return _regions; }
        std::string                       getModeAt(ea_t ea) const;

    private:
        void    setupProjectDir();
        void    order_insert(ea_t start, ea_t end);
        void    interval_subtraction(std::vector<PatchRegion>::iterator it, ea_t start, ea_t end);

        std::filesystem::path    bin_path;
        std::filesystem::path    project_dir;
        std::string              arch;
        std::string              endianness;
        int                      bits     = 0;
        uint64_t                 bin_base = 0;
        std::vector<PatchRegion> _regions;
};