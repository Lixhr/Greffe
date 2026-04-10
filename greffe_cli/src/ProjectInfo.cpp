#include "ProjectInfo.hpp"
#include "MakefileTemplates.hpp"
#include "utils.hpp"
#include "colors.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "cli_fmt.hpp"

using namespace Color;

Segment::Segment(const json& json_seg)
    : start(json_get<uint64_t>(json_seg, "start"))
    , end  (json_get<uint64_t>(json_seg, "end"))
    , name (json_get<std::string>(json_seg, "name")) {

    if (end < start)
        throw std::runtime_error("Segment end < start");
}


void ProjectInfo::populateFromJson(const json& body) {
    bin_path   = json_get<std::string>(body, "bin_path");
    arch       = json_get<std::string>(body, "arch");
    endianness = json_get<std::string>(body, "endianness");
    bits       = json_get<int>        (body, "bits");
    bin_base   = json_get<uint64_t>   (body, "bin_base");
    

    for (const json& seg : json_get<json>(body, "segments"))
        segments.emplace_back(seg);
}

json ProjectInfo::fetchInfo(IdaIPC& client) {
    const json resp = client.send({ {"action", "info"} });

    if (!resp.value("ok", false))
        throw IdaIPCError(json_get<std::string>(resp, "body"));

    return json_get<json>(resp, "body");
}

void ProjectInfo::setupProjectDir() {
    project_dir = bin_path.parent_path() / "__greffe_workdir";

    if (!std::filesystem::exists(project_dir))
        std::filesystem::create_directory(project_dir);

    std::filesystem::path dest_mk = project_dir / "Makefile";
    if (!std::filesystem::exists(dest_mk)) {
        std::ofstream f(dest_mk);
        if (!f)
            throw std::runtime_error("cannot create " + dest_mk.string());
        f << MakefileTemplates::get(arch);
    }

    std::cout << GREY << ITALIC << "  Working directory : \n  " << project_dir.string() \
    << RST << "\n";
}


static uint64_t prompt_patch_base() {
    char* raw = readline("patch_base (hex): ");
    if (!raw)
        throw std::runtime_error("patch_base is required");
    std::string s = raw;
    free(raw);
    try {
        std::size_t pos = 0;
        uint64_t v = std::stoull(s, &pos, 0);
        if (pos != s.size()) throw std::invalid_argument("");
        return v;
    } catch (...) {
        throw std::runtime_error("invalid address: " + s);
    }
}

ProjectInfo::ProjectInfo(IdaIPC& client) {
    json info = fetchInfo(client);
    populateFromJson(info);
    setupProjectDir();
}

void ProjectInfo::initPatchBase(uint64_t bin_base) {
    uint64_t usr_base = prompt_patch_base();

    if (bits == 64 && usr_base & 0xF)
        throw std::invalid_argument("patch_base must be 0x10 aligned");

    if (bits == 32 && usr_base & 0x7)
        throw std::invalid_argument("patch_base must be 0x8 aligned");

    if (usr_base < bin_base) 
        throw std::invalid_argument("patch_base < bin_base");

    if (bits == 32 && usr_base >= UINT32_MAX)
        throw std::invalid_argument("patch_base > UINT32_MAX (32 bits arch)");

    patch_base = usr_base;
}
