#include "arch/StubsFactory.hpp"
#include "arch/thumb/ThumbStubs.hpp"

#include <functional>
#include <stdexcept>
#include <string_view>
#include <tuple>


std::unique_ptr<IArchStubs> StubsFactory::create(const Target& t, const ProjectInfo& pi) {
    using Ctor = std::function<std::unique_ptr<IArchStubs>()>;

    static const std::tuple<int, std::string_view, Ctor> table[] = {
        { 32, "thumb", [] { return std::make_unique<ThumbStubs>(); } },
    };

    for (const auto& c : t.context()) {
        if (c.ea != t.ea())
            continue;

        for (const auto& [bits, mode, ctor] : table)
            if (pi.getBits() == bits && c.mode == mode)
                return ctor();

        throw std::runtime_error("StubsFactory: unknown arch "
                                 + std::to_string(pi.getBits()) + "/" + c.mode);
    }

    char buf[17];
    snprintf(buf, sizeof(buf), "%lx", t.ea());
    throw std::runtime_error("StubsFactory: no context entry found for ea 0x" + std::string(buf));
}
