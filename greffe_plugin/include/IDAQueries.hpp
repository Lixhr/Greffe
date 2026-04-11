#pragma once

#include "Target.hpp"
#include <filesystem>
#include <string>
#include <vector>

// Direct IDA SDK queries.  All functions must be called from the main IDA thread.
namespace IDAQueries {

struct SegmentInfo {
    uint64_t    start;
    uint64_t    end;
    std::string name;
};

struct ProjectData {
    std::filesystem::path    bin_path;
    std::string              arch;        // e.g. "ARM"
    std::string              endianness;  // "little" | "big"
    int                      bits;        // 32 | 64
    uint64_t                 bin_base;
    std::vector<SegmentInfo> segments;
};

struct ContextData {
    std::string               name;
    uint64_t                  ea;
    uint64_t                  end_ea;
    std::vector<ContextEntry> context;
};

ProjectData fetch_project_info();
ContextData fetch_context(uint64_t ea);

} // namespace IDAQueries
