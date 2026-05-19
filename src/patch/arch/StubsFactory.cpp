#include "arch/StubsFactory.hpp"
#include "arch/ThumbStubs.hpp"
#include "arch/Arm32Stubs.hpp"
#include "arch/Aarch64Stubs.hpp"
#include <idp.hpp>
#include <map>
#include <segregs.hpp>
#include <stdexcept>

static std::string arm_detect_mode(ea_t ea) {
    int t_reg = str2reg("T");
    return (t_reg >= 0 && get_sreg(ea, t_reg) == 1) ? "thumb" : "arm32";
}

static const ArchDescriptor descriptors[] = {
    {
        32, "arm", "le",
        arm_detect_mode,
        [](const ArchKey& k) -> std::shared_ptr<IArchStubs> {
            if (k.mode == "thumb") return std::make_shared<ThumbStubs>(k.bits, k.endianness);
            return std::make_shared<Arm32Stubs>(k.bits, k.endianness);
        },
        "arm-none-eabi-gcc", "arm-none-eabi-objcopy", "",
        "*(.ARM.exidx*) *(.ARM.extab*)",
    },
    {
        32, "armb", "be",
        arm_detect_mode,
        [](const ArchKey& k) -> std::shared_ptr<IArchStubs> {
            if (k.mode == "thumb") return std::make_shared<ThumbStubs>(k.bits, k.endianness);
            return std::make_shared<Arm32Stubs>(k.bits, k.endianness);
        },
        "armeb-linux-gnueabihf-gcc", "armeb-linux-gnueabihf-objcopy", "",
        "*(.ARM.exidx*) *(.ARM.extab*)",
    },
    {
        64, "arm", "le",
        [](ea_t) { return std::string("aarch64"); },
        [](const ArchKey& k) -> std::shared_ptr<IArchStubs> {
            return std::make_shared<Arm64Stubs>(k.bits, k.endianness);
        },
        "aarch64-none-linux-gnu-gcc", "aarch64-none-linux-gnu-objcopy", "",
        "",
    },
    {
        64, "armb", "be",
        [](ea_t) { return std::string("aarch64"); },
        [](const ArchKey& k) -> std::shared_ptr<IArchStubs> {
            return std::make_shared<Arm64Stubs>(k.bits, k.endianness);
        },
        "aarch64_be-none-linux-gnu-gcc", "aarch64_be-none-linux-gnu-objcopy", "",
        "",
    },
};

const ArchDescriptor* StubsFactory::findDescriptor(int bits, const std::string& arch,
                                                    const std::string& endianness) {
    for (const auto& d : descriptors)
        if (d.bits == bits && d.arch == arch && d.endianness == endianness)
            return &d;
    return nullptr;
}

ArchKey StubsFactory::buildKey(int bits, const std::string& arch,
                                const std::string& endianness, ea_t ea) {
    const ArchDescriptor* d = findDescriptor(bits, arch, endianness);
    if (!d)
        throw std::runtime_error("StubsFactory: no descriptor for "
                                 + std::to_string(bits) + "/" + arch + "/" + endianness);
    return { bits, arch, d->detect_mode(ea), endianness };
}

std::shared_ptr<IArchStubs> StubsFactory::create(const ArchKey& key) {
    static std::map<ArchKey, std::shared_ptr<IArchStubs>> cache;

    auto it = cache.find(key);
    if (it != cache.end())
        return it->second;

    const ArchDescriptor* d = findDescriptor(key.bits, key.arch, key.endianness);
    if (!d)
        throw std::runtime_error("StubsFactory: unknown arch "
                                 + std::to_string(key.bits) + "/" + key.arch
                                 + "/" + key.mode + "/" + key.endianness);

    return cache.emplace(key, d->ctor(key)).first->second;
}
