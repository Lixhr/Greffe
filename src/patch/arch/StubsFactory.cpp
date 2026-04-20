#include "arch/StubsFactory.hpp"
#include "arch/ThumbStubs.hpp"
#include "arch/Arm32Stubs.hpp"
#include <functional>
#include <map>
#include <stdexcept>

std::shared_ptr<IArchStubs> StubsFactory::create(const ArchKey& key) {
    using Inst = std::shared_ptr<IArchStubs>;

    static std::map<ArchKey, Inst> cache;

    static const std::pair<ArchKey, std::function<Inst(const ArchKey&)>> table[] = {
        { { 32, "arm", "thumb", "le" }, [](const ArchKey& k) { return std::make_shared<ThumbStubs>(k.bits, k.endianness); } },
        { { 32, "arm", "thumb", "be" }, [](const ArchKey& k) { return std::make_shared<ThumbStubs>(k.bits, k.endianness); } },
        { { 32, "arm", "arm32",  "le" }, [](const ArchKey& k) { return std::make_shared<Arm32Stubs>(k.bits, k.endianness); } },
        { { 32, "arm", "arm32",  "be" }, [](const ArchKey& k) { return std::make_shared<Arm32Stubs>(k.bits, k.endianness); } },
    };

    for (const auto& [k, ctor] : table) {
        if (key.bits == k.bits && key.arch == k.arch
                && key.mode == k.mode && key.endianness == k.endianness) {
            auto it = cache.find(k);
            if (it == cache.end())
                it = cache.emplace(k, ctor(key)).first;
            return it->second;
        }
    }

    throw std::runtime_error("StubsFactory: unknown arch "
                             + std::to_string(key.bits) + "/" + key.arch
                             + "/" + key.mode + "/" + key.endianness);
}
