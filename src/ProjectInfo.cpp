#include "ProjectInfo.hpp"
#include "MakefileTemplates.hpp"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "kernwin.hpp"
#include "utils.hpp"
#include <idp.hpp>

void ProjectInfo::setupProjectDir() {
    project_dir = bin_path.parent_path() / "__greffe_workdir";

    if (!std::filesystem::exists(project_dir))
        std::filesystem::create_directory(project_dir);

    std::filesystem::path dest_mk = project_dir / "Makefile";
    bool needs_write = !std::filesystem::exists(dest_mk)
                    || std::filesystem::file_size(dest_mk) == 0;
    if (needs_write) {
        std::string content = MakefileTemplates::get(bits, arch, endianness);
        std::ofstream f(dest_mk);
        if (!f)
            throw std::runtime_error("cannot create " + dest_mk.string());
        f << content;
    }

    std::string display = project_dir.string();

    qstring home;
    if (qgetenv("HOME", &home) && display.rfind(home.c_str(), 0) == 0)
        display = "~" + display.substr(strlen(home.c_str()));

    greffe_msg("Workdir: %s\n", display.c_str());
    workdir_popup(project_dir, display);
}

void ProjectInfo::populateData() {
    char buf[QMAXPATH];
    if (getinf_buf(INF_INPUT_FILE_PATH, buf, sizeof(buf)) == -1)
        throw std::runtime_error("Failed to retrieve input file path");

    bin_path   = buf;
    arch       = inf_get_procname().c_str();
    std::transform(arch.begin(), arch.end(), arch.begin(), ::tolower);
    endianness = inf_is_be()    ? "be" : "le";
    bits       = inf_is_64bit() ?  64  :  32;
    bin_base   = inf_get_baseaddr();
}

ProjectInfo::ProjectInfo() {
    populateData();
    setupProjectDir();
}

ArchKey ProjectInfo::getArchKeyAt(ea_t ea) const {
    return StubsFactory::buildKey(bits, arch, endianness, ea);
}
