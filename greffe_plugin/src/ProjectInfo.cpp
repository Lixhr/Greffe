#include "ProjectInfo.hpp"
#include "MakefileTemplates.hpp"
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "kernwin.hpp"
#include "utils.hpp"
#include <idp.hpp>
#include <segregs.hpp>

void ProjectInfo::setupProjectDir() {
    project_dir = bin_path.parent_path() / "__greffe_workdir";

    if (!std::filesystem::exists(project_dir))
        std::filesystem::create_directory(project_dir);

    std::filesystem::path dest_mk = project_dir / "Makefile";
    bool needs_write = !std::filesystem::exists(dest_mk)
                    || std::filesystem::file_size(dest_mk) == 0;
    if (needs_write) {
        std::string_view content = MakefileTemplates::get(arch);
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
}

void ProjectInfo::populateData() {
    char buf[QMAXPATH];
    if (getinf_buf(INF_INPUT_FILE_PATH, buf, sizeof(buf)) == -1)
        throw std::runtime_error("Failed to retrieve input file path");

    bin_path   = buf;
    arch       = inf_get_procname().c_str();
    endianness = inf_is_be()    ? "be" : "le";
    bits       = inf_is_64bit() ?  64  :  32;
    bin_base   = inf_get_baseaddr();
}

ProjectInfo::ProjectInfo() {
    populateData();
    setupProjectDir();
}

void ProjectInfo::add_region(ea_t start, ea_t end) {
    if (end <= start) {
        std::ostringstream ss;
        ss << "invalid patch region: 0x" << std::hex << start
           << " >= 0x" << end;
        throw std::runtime_error(ss.str());
    }
    _regions.push_back({ start, end });
}

std::string ProjectInfo::getModeAt(ea_t ea) const {
    char buf[32];
    get_idp_name(buf, sizeof(buf));
    std::string idp = buf;
    std::transform(idp.begin(), idp.end(), idp.begin(), ::tolower);

    if (idp == "arm") {
        int t_reg = str2reg("T");
        if (t_reg >= 0 && get_sreg(ea, t_reg) == 1)
            return "thumb";
        return "";
    }
    return "";
}
