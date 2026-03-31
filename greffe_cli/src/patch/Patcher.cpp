#include "Patcher.hpp"
#include <stdexcept>

std::vector<uint8_t> Patcher::collect_input(const Target& t) {
    std::vector<uint8_t> out;
    for (const auto& c : t.context()) {
        if (c.ea < t.ea()) continue;
        out.insert(out.end(), c.raw.begin(), c.raw.end());
    }
    return out;
}
