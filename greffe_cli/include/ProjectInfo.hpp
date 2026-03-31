#pragma once

#include "IdaIPC.hpp"
#include <filesystem>
#include <optional>
#include <vector>

class Segment {
public:
    Segment(const json& json_seg);

    uint64_t           getStart() const { return start; }
    uint64_t           getEnd()   const { return end; }
    const std::string& getName()  const { return name; }

private:
    const uint64_t    start;
    const uint64_t    end;
    const std::string name;
};


struct ProjectInfo {
public:
    ProjectInfo(IdaIPC& client);

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
    json fetchInfo(IdaIPC& client);
    void populateFromJson(const json& body);
    void setupProjectDir();

    std::filesystem::path   bin_path;
    std::filesystem::path   project_dir;
    std::string             arch;
    std::string             endianness;
    int                     bits;
    uint64_t                bin_base;
    uint64_t                patch_base = -1ULL;
    std::vector<Segment>    segments;
};
