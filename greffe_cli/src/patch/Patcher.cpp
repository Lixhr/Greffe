#include "Patcher.hpp"
#include <stdexcept>

static std::vector<uint8_t> hex_decode(const std::string& hex) {
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
        out.push_back(static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    return out;
}

std::vector<uint8_t> Patcher::collect_input(const Target& t) {
    std::vector<uint8_t> out;
    for (const auto& c : t.context()) {
        if (c.ea < t.ea()) continue;
        if (c.raw.size() % 2 != 0)
            throw std::runtime_error("malformed raw bytes for " + t.name());
        auto b = hex_decode(c.raw);
        out.insert(out.end(), b.begin(), b.end());
    }
    return out;
}
