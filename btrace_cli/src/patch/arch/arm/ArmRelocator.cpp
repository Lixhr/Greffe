#include "arm/ArmRelocator.hpp"

extern "C" {
#include <gum/arch-arm/gumarmrelocator.h>
#include <gum/arch-arm/gumarmwriter.h>
}

#include <stdexcept>

std::string_view ArmRelocator::name() const { return "ARM"; }

RelocatedCode ArmRelocator::relocate(const std::vector<uint8_t>& input_bytes,
                                     size_t   n_bytes,
                                     uint64_t original_ea,
                                     uint64_t trampoline_addr,
                                     const std::vector<uint8_t>& trailer)
{
    if (input_bytes.empty() || n_bytes == 0)
        throw std::runtime_error("ArmRelocator: empty input");

    std::vector<uint8_t> output(512, 0);

    GumArmWriter writer;
    gum_arm_writer_init(&writer, output.data());
    writer.pc = static_cast<GumAddress>(trampoline_addr);

    GumArmRelocator rl;
    gum_arm_relocator_init(&rl, input_bytes.data(), &writer);
    rl.input_pc = static_cast<GumAddress>(original_ea);

    size_t n_insns = 0;
    const cs_insn* insn = nullptr;
    while (gum_arm_relocator_read_one(&rl, &insn) != 0) {
        ++n_insns;
        size_t consumed = (size_t)(rl.input_cur - rl.input_start);
        if (consumed >= n_bytes || gum_arm_relocator_eoi(&rl))
            break;
    }

    if (n_insns == 0)
        throw std::runtime_error("ArmRelocator: failed to read any instruction");

    while (gum_arm_relocator_write_one(&rl)) {}

    size_t insns_size = reinterpret_cast<uint8_t*>(writer.code)
                      - reinterpret_cast<uint8_t*>(writer.base);

    if (!trailer.empty())
        gum_arm_writer_put_bytes(&writer, trailer.data(),
                                 static_cast<guint>(trailer.size()));

    gum_arm_writer_flush(&writer);

    size_t written = reinterpret_cast<uint8_t*>(writer.code)
                   - reinterpret_cast<uint8_t*>(writer.base);

    gum_arm_relocator_clear(&rl);
    gum_arm_writer_clear(&writer);

    bool ends_with_branch = static_cast<bool>(rl.eob);
    output.resize(written);
    return { std::move(output), insns_size, n_insns, ends_with_branch };
}
