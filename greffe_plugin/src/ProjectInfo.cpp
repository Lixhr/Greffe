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

void ProjectInfo::order_insert(ea_t start, ea_t end) {
    auto pos = std::lower_bound(_regions.begin(), _regions.end(), start,
        [](const PatchRegion& p, ea_t val) { return p.base < val; });

    _regions.insert(pos, PatchRegion(start, end));\
    set_range_color(start, end, Color::PATCH_REGION);
}    

void ProjectInfo::interval_subtraction(std::vector<PatchRegion>::iterator it,
                                                                     ea_t start,
                                                                     ea_t end) {
    std::vector<std::pair<ea_t, ea_t>> to_insert;
    ea_t cursor = start;

    while (it != _regions.end() && it->base < end) {
        if (cursor < it->base)
            to_insert.emplace_back(cursor, it->base);
        if (it->end > cursor)
            cursor = it->end;
        ++it;
    }

    if (cursor < end)
        to_insert.emplace_back(cursor, end);

    for (auto& [s, e] : to_insert)
        order_insert(s, e);
}

void ProjectInfo::add_region(ea_t start, ea_t end) {
    auto it = std::lower_bound(_regions.begin(), _regions.end(), start,
        [](const PatchRegion& p, ea_t val) { return p.base < val; });

    if (it != _regions.begin() && std::prev(it)->end > start)
        --it;

    if (it != _regions.end() && it->base < end) {
        interval_subtraction(it, start, end);
    }
    else {
        order_insert(start, end);
    }
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
