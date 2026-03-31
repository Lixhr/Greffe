#include "arch/StubsFactory.hpp"
#include "arch/thumb/ThumbStubs.hpp"

#include <functional>
#include <map>
#include <stdexcept>
#include <string_view>
#include <tuple>


std::shared_ptr<IArchStubs> StubsFactory::create(const Target& t, const ProjectInfo& pi) {
    using Key  = std::pair<int, std::string_view>;
    using Inst = std::shared_ptr<IArchStubs>;

    static std::map<Key, Inst> cache;

    static const std::tuple<int, std::string_view, std::function<Inst()>> table[] = {
        { 32, "thumb", [] { return std::make_shared<ThumbStubs>(); } },
    };

    for (const auto& c : t.context()) {
        if (c.ea != t.ea())
            continue;

        for (const auto& [bits, mode, ctor] : table) {
            if (pi.getBits() == bits && c.mode == mode) {
                Key key{bits, mode};
                auto it = cache.find(key);
                if (it == cache.end())
                    it = cache.emplace(key, ctor()).first;
                return it->second;
            }
        }

        throw std::runtime_error("StubsFactory: unknown arch "
                                 + std::to_string(pi.getBits()) + "/" + c.mode);
    }

    char buf[17];
    snprintf(buf, sizeof(buf), "%lx", t.ea());
    throw std::runtime_error("StubsFactory: no context entry found for ea 0x" + std::string(buf));
}
