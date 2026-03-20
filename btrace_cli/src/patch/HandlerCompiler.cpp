#include "HandlerCompiler.hpp"
#include "patch_utils.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

extern "C" {
#include <libelf.h>
#include <gelf.h>
}

static std::vector<uint8_t> read_file(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f)
        throw std::runtime_error("HandlerCompiler: cannot open " + path.string());
    return { std::istreambuf_iterator<char>(f), {} };
}

static std::unordered_map<std::string, uint64_t>
parse_symbols(const std::filesystem::path& elf_path) {
    if (elf_version(EV_CURRENT) == EV_NONE)
        throw std::runtime_error("HandlerCompiler: libelf init failed");

    int fd = open(elf_path.c_str(), O_RDONLY);
    if (fd < 0)
        throw std::runtime_error("HandlerCompiler: cannot open " + elf_path.string());

    Elf* e = elf_begin(fd, ELF_C_READ, nullptr);
    if (!e) {
        close(fd);
        throw std::runtime_error("HandlerCompiler: elf_begin failed: "
                                 + std::string(elf_errmsg(-1)));
    }

    std::unordered_map<std::string, uint64_t> result;

    Elf_Scn* scn = nullptr;
    while ((scn = elf_nextscn(e, scn)) != nullptr) {
        GElf_Shdr shdr;
        if (!gelf_getshdr(scn, &shdr))
            continue;
        if (shdr.sh_type != SHT_SYMTAB && shdr.sh_type != SHT_DYNSYM)
            continue;

        Elf_Data* data = elf_getdata(scn, nullptr);
        if (!data)
            continue;

        size_t n = shdr.sh_size / shdr.sh_entsize;
        for (size_t i = 0; i < n; ++i) {
            GElf_Sym sym;
            if (!gelf_getsym(data, static_cast<int>(i), &sym))
                continue;
            if (GELF_ST_TYPE(sym.st_info) != STT_FUNC)
                continue;
            const char* name = elf_strptr(e, shdr.sh_link, sym.st_name);
            if (name && name[0] != '\0')
                result[name] = static_cast<uint64_t>(sym.st_value);
        }
    }

    elf_end(e);
    close(fd);
    return result;
}

HandlerBin HandlerCompiler::build(const std::vector<Target>& targets,
                                   const ProjectInfo& pinfo)
{
    namespace fs = std::filesystem;

    const fs::path& workdir = pinfo.getProjectDir();

    int ret = std::system(("make -C " + workdir.string() + " 2>&1").c_str());
    if (ret != 0)
        throw std::runtime_error("HandlerCompiler: make failed (exit " + std::to_string(ret) + ")");

    auto elf_path = workdir / "build" / "handlers.elf";
    auto bin_path = workdir / "build" / "handlers.bin";

    auto symbols = parse_symbols(elf_path);
    auto bytes   = read_file(bin_path);

    std::unordered_map<std::string, uint64_t> offsets;
    for (const auto& t : targets) {
        std::string sym = "handler_" + sanitize(t.name());
        auto it = symbols.find(sym);
        if (it == symbols.end())
            throw std::runtime_error("HandlerCompiler: symbol not found in ELF: " + sym);
        offsets[sym] = it->second;
    }

    return HandlerBin(std::move(bytes), std::move(offsets));
}
