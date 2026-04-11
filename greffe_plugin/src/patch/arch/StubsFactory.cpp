#include "arch/StubsFactory.hpp"
#include "arch/thumb/ThumbStubs.hpp"
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>


std::shared_ptr<IArchStubs> StubsFactory::create(int bits, std::string_view mode) {
    using Key  = std::pair<int, std::string>;
    using Inst = std::shared_ptr<IArchStubs>;

    static std::map<Key, Inst> cache;

    static const std::tuple<int, std::string_view, std::function<Inst()>> table[] = {
        { 32, "thumb", [] { return std::make_shared<ThumbStubs>(); } },
    };

    for (const auto& [b, m, ctor] : table) {
        if (bits == b && mode == m) {
            Key key{b, std::string(m)};
            auto it = cache.find(key);
            if (it == cache.end())
                it = cache.emplace(key, ctor()).first;
            return it->second;
        }
    }

    throw std::runtime_error("StubsFactory: unknown arch "
                             + std::to_string(bits) + "/" + std::string(mode));
}