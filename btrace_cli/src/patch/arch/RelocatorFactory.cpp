#include "arch/RelocatorFactory.hpp"
#include "arch/ArmRelocator.hpp"
#include "arch/ThumbRelocator.hpp"
#include "arch/Arm64Relocator.hpp"

#include <stdexcept>

std::unique_ptr<IRelocator> RelocatorFactory::create(const Target& t,
                                                     const ProjectInfo& pi)
{
    if (pi.getBits() == 64)
        return std::make_unique<Arm64Relocator>();

    for (const auto& c : t.context()) {
        if (c.ea != t.ea())
            continue;
        if (c.mode == "thumb")
            return std::make_unique<ThumbRelocator>();
        return std::make_unique<ArmRelocator>();
    }

    throw std::runtime_error("RelocatorFactory: no context entry found for ea 0x"
                             + [&]{ std::string s; char buf[17];
                                    snprintf(buf, sizeof(buf), "%lx", t.ea());
                                    return std::string(buf); }());
}
