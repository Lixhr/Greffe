#pragma once

#include "IArchStubs.hpp"
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include "ida.hpp"

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

struct ArchDescriptor {
    int              bits;
    std::string_view arch;
    std::string_view endianness;

    std::function<std::string(ea_t)> detect_mode;

    std::function<std::shared_ptr<IArchStubs>(const ArchKey&)> ctor;

    std::string_view cc;
    std::string_view objcopy;
    std::string_view cflags_extra;
    std::string_view ld_discard;
};

class StubsFactory {
    public:
        static std::shared_ptr<IArchStubs> create(const ArchKey& key);

        static ArchKey buildKey(int bits, const std::string& arch,
                                const std::string& endianness, ea_t ea);

        static const ArchDescriptor* findDescriptor(int bits, const std::string& arch,
                                                    const std::string& endianness);
};
