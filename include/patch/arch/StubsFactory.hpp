#pragma once

#include "IArchStubs.hpp"
#include <memory>
#include <string>

struct ArchKey {
    int         bits;
    std::string arch;
    std::string mode;
    std::string endianness;

    bool operator<(const ArchKey& o) const {
        if (bits != o.bits)       return bits < o.bits;
        if (arch != o.arch)       return arch < o.arch;
        if (mode != o.mode)       return mode < o.mode;
        return endianness < o.endianness;
    }
};

class StubsFactory {
    public:
        static std::shared_ptr<IArchStubs> create(const ArchKey& key);
};
