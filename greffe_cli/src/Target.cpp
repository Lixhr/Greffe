#include "Target.hpp"
#include "colors.hpp"

#include <capstone/capstone.h>
#include <iomanip>
#include <vector>


Target::Target(std::string name, uint64_t ea, uint64_t end_ea,
               std::vector<ContextEntry> context)
    : _name(std::move(name))
    , _ea(ea)
    , _end_ea(end_ea)
    , _context(std::move(context)) {}

const std::string&              Target::name()    const { return _name; }
uint64_t                        Target::ea()      const { return _ea; }
uint64_t                        Target::end_ea()  const { return _end_ea; }
const std::vector<ContextEntry>& Target::context() const { return _context; }


static std::vector<uint8_t> hex_decode(const std::string& hex) {
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
        out.push_back(static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    return out;
}

static bool open_cs(int bits, const std::string& mode, csh& handle) {
    cs_arch_register_arm();
    cs_arch_register_arm64();

    cs_arch arch;
    cs_mode cs_m;

    if (bits == 64) {
        arch = CS_ARCH_ARM64;
        cs_m = CS_MODE_ARM;
    } else if (mode == "thumb") {
        arch = CS_ARCH_ARM;
        cs_m = CS_MODE_THUMB;
    } else {
        arch = CS_ARCH_ARM;
        cs_m = CS_MODE_ARM;
    }

    return cs_open(arch, cs_m, &handle) == CS_ERR_OK;
}


std::ostream& operator<<(std::ostream& os, const TargetView& v) {
    using namespace Color;

    const Target& t  = v.target;
    const int  addr_w = v.bits == 64 ? 16 : 8;

    os << BOLD << CYAN << t.name() << RST << '\n';

    std::string mode;
    for (const auto& c : t.context())
        if (c.ea == t.ea()) { mode = c.mode; break; }

    csh  cs_handle;
    bool has_cs = open_cs(v.bits, mode, cs_handle);

    for (const auto& c : t.context()) {
        bool is_target = (c.ea == t.ea());

        os << (is_target ? GREEN : DIM) << (is_target ? "  > " : "    ") << RST;

        os << BLUE << "0x"
           << std::hex << std::setw(addr_w) << std::setfill('0') << c.ea
           << RST << std::dec;

        os << "  " << GREY
           << std::left << std::setw(8) << std::setfill(' ') << c.raw
           << RST << std::right;

        if (has_cs) {
            auto     bytes = hex_decode(c.raw);
            cs_insn* insn  = nullptr;
            size_t   count = cs_disasm(cs_handle, bytes.data(), bytes.size(), c.ea, 1, &insn);
            if (count > 0) {
                os << "  " << CYAN << insn->mnemonic;
                if (insn->op_str[0] != '\0')
                    os << ' ' << insn->op_str;
                os << RST;
                cs_free(insn, count);
            }
        }

        os << '\n';
    }

    if (has_cs)
        cs_close(&cs_handle);

    return os;
}
