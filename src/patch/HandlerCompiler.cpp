#include "HandlerCompiler.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

extern "C" {
#include <libelf.h>
#include <gelf.h>
}

static std::vector<uint8_t> load_elf_image(const std::filesystem::path& elf_path) {
    if (elf_version(EV_CURRENT) == EV_NONE)
        throw std::runtime_error("HandlerCompiler: libelf init failed");

    int fd = open(elf_path.c_str(), O_RDONLY);
    if (fd < 0)
        throw std::runtime_error("HandlerCompiler: cannot open " + elf_path.string());

    Elf *e = elf_begin(fd, ELF_C_READ, nullptr);
    if (!e) {
        close(fd);
        throw std::runtime_error("HandlerCompiler: elf_begin failed: "
                                 + std::string(elf_errmsg(-1)));
    }

    size_t phnum = 0;
    if (elf_getphdrnum(e, &phnum) != 0) {
        elf_end(e);
        close(fd);
        throw std::runtime_error("HandlerCompiler: elf_getphdrnum failed");
    }

    std::vector<uint8_t> image;
    for (size_t i = 0; i < phnum; ++i) {
        GElf_Phdr phdr;
        if (!gelf_getphdr(e, static_cast<int>(i), &phdr) || phdr.p_type != PT_LOAD)
            continue;

        image.resize(static_cast<size_t>(phdr.p_memsz), 0);
        ssize_t n = pread(fd, image.data(),
                          static_cast<size_t>(phdr.p_filesz),
                          static_cast<off_t>(phdr.p_offset));
        if (n != static_cast<ssize_t>(phdr.p_filesz)) {
            elf_end(e);
            close(fd);
            throw std::runtime_error("HandlerCompiler: pread failed on PT_LOAD segment");
        }
        break;
    }

    elf_end(e);
    close(fd);
    return image;
}

#ifndef R_ARM_ABS32
#define R_ARM_ABS32 2
#endif

static std::vector<uint32_t>
collect_abs32_relocs(const std::filesystem::path& elf_path) {
    int fd = open(elf_path.c_str(), O_RDONLY);
    if (fd < 0) return {};

    Elf *e = elf_begin(fd, ELF_C_READ, nullptr);
    if (!e) { close(fd); return {}; }

    std::vector<uint32_t> offsets;
    Elf_Scn *scn = nullptr;
    while ((scn = elf_nextscn(e, scn)) != nullptr) {
        GElf_Shdr shdr;
        if (!gelf_getshdr(scn, &shdr)) continue;
        if (shdr.sh_type != SHT_REL || shdr.sh_entsize == 0) continue;

        Elf_Data *data = elf_getdata(scn, nullptr);
        if (!data) continue;

        size_t n = shdr.sh_size / shdr.sh_entsize;
        for (size_t i = 0; i < n; ++i) {
            GElf_Rel rel;
            if (!gelf_getrel(data, static_cast<int>(i), &rel)) continue;
            if (GELF_R_TYPE(rel.r_info) == R_ARM_ABS32)
                offsets.push_back(static_cast<uint32_t>(rel.r_offset));
        }
    }

    elf_end(e);
    close(fd);
    return offsets;
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

        if (shdr.sh_entsize == 0)
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

HandlerBin HandlerCompiler::build(const std::vector<PatchPlan *> plans,
                                  const ProjectInfo& pinfo) {
    namespace fs = std::filesystem;

    const fs::path& workdir = pinfo.getProjectDir();

    {
        std::ofstream mk(workdir / "greffe_active.mk");
        if (!mk)
            throw std::runtime_error("HandlerCompiler: cannot write greffe_active.mk");
        mk << "ACTIVE_GREFFE_SRCS :=";
        for (const auto& p : plans)
            mk << " handlers/" << p->name << ".c";
        mk << '\n';
    }

    int pipefd[2];
    if (pipe(pipefd) < 0)
        throw std::runtime_error("HandlerCompiler: pipe() failed");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("HandlerCompiler: fork() failed");
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execlp("make", "make", "re", "-C", workdir.c_str(), nullptr);
        _exit(1);
    }
    close(pipefd[1]);

    std::string build_output;
    {
        char buf[4096];
        ssize_t n;
        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0)
            build_output.append(buf, n);
        close(pipefd[0]);
    }

    int status;
    if (qwait(&status, pid, 0) != pid)
        throw std::runtime_error("HandlerCompiler: waitpid() failed");
    int ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    if (ret != 0) {
        msg("%s\n", build_output.c_str());
        throw std::runtime_error("HandlerCompiler: make failed (exit " + std::to_string(ret) + ")");
    }

    auto elf_path = workdir / "build" / "handlers.elf";

    auto symbols  = parse_symbols(elf_path);
    auto bytes    = load_elf_image(elf_path);
    auto rel_offs = collect_abs32_relocs(elf_path);

    std::unordered_map<std::string, uint64_t> offsets;
    for (const auto& p : plans) {
        std::string sym = "handler_" + p->name;
        auto it = symbols.find(sym);
        if (it == symbols.end())
            throw std::runtime_error("HandlerCompiler: symbol not found in ELF: " + sym);
        offsets[sym] = it->second;
    }

    return HandlerBin(std::move(bytes), std::move(offsets), std::move(rel_offs));
}
