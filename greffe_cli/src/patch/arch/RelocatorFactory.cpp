#include "arch/RelocatorFactory.hpp"
#include "arch/arm/ArmRelocator.hpp"
#include "arch/thumb/ThumbRelocator.hpp"
#include "arch/arm64/Arm64Relocator.hpp"

#include <functional>
#include <stdexcept>
#include <string_view>
#include <tuple>

std::unique_ptr<IRelocator> RelocatorFactory::create(const Target& t,
                                                     const ProjectInfo& pi) {
    using Ctor = std::function<std::unique_ptr<IRelocator>()>;
    static const std::tuple<int, std::string_view, Ctor> table[] = {
        { 32, "arm",   [] { return std::make_unique<ArmRelocator>(); } },
        { 32, "thumb", [] { return std::make_unique<ThumbRelocator>(); } },
        { 64, "arm64", [] { return std::make_unique<Arm64Relocator>(); } },
    };

    for (const auto& c : t.context()) {
        if (c.ea != t.ea())
            continue;
        for (const auto& [bits, mode, ctor] : table)
            if (pi.getBits() == bits && c.mode == mode) return ctor();
        throw std::runtime_error("RelocatorFactory: unknown arch "
                                 + std::to_string(pi.getBits()) + "/" + c.mode);
    }

    throw std::runtime_error("RelocatorFactory: no context entry found for ea 0x"
                             + [&]{ char buf[17];
                                    snprintf(buf, sizeof(buf), "%lx", t.ea());
                                    return std::string(buf); }());
}
