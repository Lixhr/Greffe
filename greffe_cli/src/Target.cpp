#include "Target.hpp"
#include "colors.hpp"
#include <iomanip>

Target::Target(std::string name, uint64_t ea, uint64_t end_ea,
               std::vector<ContextEntry> context)
    : _name(std::move(name))
    , _ea(ea)
    , _end_ea(end_ea)
    , _context(std::move(context)) {}

const std::string&             Target::name()    const { return _name; }
uint64_t                       Target::ea()      const { return _ea; }
uint64_t                       Target::end_ea()  const { return _end_ea; }
const std::vector<ContextEntry>& Target::context() const { return _context; }

std::ostream& operator<<(std::ostream& os, const TargetView& v) {
    using namespace Color;
    const Target& t       = v.target;
    const int     padding = v.bits == 64 ? 16 : 8;

    os << BOLD << CYAN << t.name() << RST << '\n';

    for (const auto& c : t.context()) {
        bool is_target = (c.ea == t.ea());
        if (is_target)
            os << GREEN << "  > " << RST;
        else
            os << DIM   << "    " << RST;

        os << DIM << "0x" << std::hex << std::setw(padding) << std::setfill('0') << c.ea << RST
           << "  " << c.raw
           << "  " << DIM << c.mode << RST << '\n';
    }

    return os;
}
