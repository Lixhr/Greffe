#include "ProjectInfo.hpp"
#include "MakefileTemplates.hpp"
#include "colors.hpp"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "cli_fmt.hpp"
#include "kernwin.hpp"

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

    std::string display = project_dir.string();
    if (const char* home = getenv("HOME"); home && display.rfind(home, 0) == 0)
        display = "~" + display.substr(strlen(home));

    std::cout << GREY << ITALIC << "  Working directory : \n  " << display << RST << "\n\n";
}


// static uint64_t prompt_patch_base() {
//     char* raw = readline("patch_base (hex): ");
//     if (!raw)
//         throw std::runtime_error("patch_base is required");
//     std::string s = raw;
//     free(raw);
//     try {
//         std::size_t pos = 0;
//         uint64_t v = std::stoull(s, &pos, 0);
//         if (pos != s.size()) throw std::invalid_argument("");
//         return v;
//     } catch (...) {
//         throw std::runtime_error("invalid address: " + s);
//     }
// }


void ProjectInfo::populateData(void) {
    {
        char buf[QMAXPATH];
        if (getinf_buf(INF_INPUT_FILE_PATH, buf, sizeof(buf)) == -1)
            throw std::runtime_error("Failed to retrieve input file path");

        bin_path = buf;
    }

    arch       = inf_get_procname().c_str();
    endianness = inf_is_be()    ? "be" : "le";
    bits       = inf_is_64bit() ?  64  :  32;
    bin_base   = inf_get_baseaddr();
}


ProjectInfo::ProjectInfo() {
    populateData();
    setupProjectDir();
}

// void ProjectInfo::initPatchBase(uint64_t bin_base) {
//     uint64_t usr_base = prompt_patch_base();

//     if (bits == 64 && usr_base & 0xF)
//         throw std::invalid_argument("patch_base must be 0x10 aligned");

//     if (usr_base < bin_base) 
//         throw std::invalid_argument("patch_base < bin_base");

//     if (bits == 32 && usr_base >= UINT32_MAX)
//         throw std::invalid_argument("patch_base > UINT32_MAX (32 bits arch)");

//     patch_base = usr_base;
// }
