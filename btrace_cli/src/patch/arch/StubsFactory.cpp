#include "arch/StubsFactory.hpp"
#include "arch/ArmStubs.hpp"
#include "arch/ThumbStubs.hpp"
#include "arch/Arm64Stubs.hpp"

#include <stdexcept>

std::unique_ptr<IArchStubs> StubsFactory::create(const Target& t,
                                                  const ProjectInfo& pi)
{
    if (pi.getBits() == 64)
        return std::make_unique<Arm64Stubs>();

    for (const auto& c : t.context()) {
        if (c.ea != t.ea())
            continue;
        if (c.mode == "thumb")
            return std::make_unique<ThumbStubs>();
        return std::make_unique<ArmStubs>();
    }

    throw std::runtime_error("StubsFactory: no context entry found for ea 0x"
                             + [&]{ char buf[17];
                                    snprintf(buf, sizeof(buf), "%lx", t.ea());
                                    return std::string(buf); }());
}
