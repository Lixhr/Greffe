#include "Target.hpp"
#include "colors.hpp"
#include <stdexcept>

#include <capstone/capstone.h>
#include <iomanip>
#include <sstream>
#include <vector>


Target::Target(std::string name, uint64_t ea, uint64_t end_ea,
               std::vector<ContextEntry> context,
               std::shared_ptr<IArchStubs> stubs)
    : _name(std::move(name))
    , _ea(ea)
    , _end_ea(end_ea)
    , _context(std::move(context))
    , _stubs(std::move(stubs)) {}

const std::string&               Target::name()    const { return _name; }
uint64_t                         Target::ea()      const { return _ea; }
uint64_t                         Target::end_ea()  const { return _end_ea; }
const std::vector<ContextEntry>& Target::context() const { return _context; }
uint64_t                         Target::trampoline_addr() const { return _trampoline_addr;}

void                             Target::setTrampolineAddr(uint64_t addr) { 
    _trampoline_addr = addr;
}

void                             Target::setTrampolineRetAddr(uint64_t addr) {
    _trampoline_ret_addr = addr;
}

void                             Target::setBranchInstr(std::vector<uint8_t> branch) {
    _branch_instr = std::move(branch);
}

IArchStubs&                      Target::stubs()   const {
    if (!_stubs) throw std::runtime_error("Target: stubs not set");
    return *_stubs;
}

void                             Target::setRelocdInstrs(
                                 std::vector<const ContextEntry *> instrs) {
    _relocd_instrs = std::move(instrs);
}


static bool open_cs(int /*bits*/, const std::string& /*mode*/, csh& handle) {
    cs_arch_register_arm();
    // cs_arch_register_arm64();

    cs_arch arch;
    cs_mode cs_m;

    // if (bits == 64) {
    //     arch = CS_ARCH_ARM64;
    //     cs_m = CS_MODE_ARM;
    // } else if (mode == "thumb") {
        // arch = CS_ARCH_ARM;
        // cs_m = CS_MODE_THUMB;
    // } else {
    //     arch = CS_ARCH_ARM;
    //     cs_m = CS_MODE_ARM;
    // }

    arch = CS_ARCH_ARM;
    cs_m = CS_MODE_THUMB;

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

        {
            std::ostringstream hex_ss;
            hex_ss << std::hex << std::setfill('0');
            for (uint8_t b : c.raw) hex_ss << std::setw(2) << static_cast<int>(b);
            os << "  " << GREY
               << std::left << std::setw(8) << std::setfill(' ') << hex_ss.str()
               << RST << std::right;
        }

        if (has_cs) {
            cs_insn* insn  = nullptr;
            size_t   count = cs_disasm(cs_handle, c.raw.data(), c.raw.size(), c.ea, 1, &insn);
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
