#include "arm64/Arm64Relocator.hpp"

extern "C" {
#include <gum/arch-arm64/gumarm64relocator.h>
#include <gum/arch-arm64/gumarm64writer.h>
}

#include <stdexcept>

std::string_view Arm64Relocator::name() const { return "ARM64"; }

RelocatedCode Arm64Relocator::relocate(const std::vector<uint8_t>& input_bytes,
                                       size_t   n_bytes,
                                       uint64_t original_ea,
                                       uint64_t trampoline_addr,
                                       const std::vector<uint8_t>& trailer)
{
    if (input_bytes.empty() || n_bytes == 0)
        throw std::runtime_error("Arm64Relocator: empty input");

    std::vector<uint8_t> output(512, 0);

    GumArm64Writer writer;
    gum_arm64_writer_init(&writer, output.data());
    writer.pc = static_cast<GumAddress>(trampoline_addr);

    GumArm64Relocator rl;
    gum_arm64_relocator_init(&rl, input_bytes.data(), &writer);
    rl.input_pc = static_cast<GumAddress>(original_ea);

    size_t n_insns = 0;
    const cs_insn* insn = nullptr;
    while (gum_arm64_relocator_read_one(&rl, &insn) != 0) {
        ++n_insns;
        size_t consumed = (size_t)(rl.input_cur - rl.input_start);
        if (consumed >= n_bytes || gum_arm64_relocator_eoi(&rl))
            break;
    }

    if (n_insns == 0)
        throw std::runtime_error("Arm64Relocator: failed to read any instruction");

    while (gum_arm64_relocator_write_one(&rl)) {}

    size_t insns_size = reinterpret_cast<uint8_t*>(writer.code)
                      - reinterpret_cast<uint8_t*>(writer.base);

    if (!trailer.empty())
        gum_arm64_writer_put_bytes(&writer, trailer.data(),
                                   static_cast<guint>(trailer.size()));

    gum_arm64_writer_flush(&writer);

    size_t written = reinterpret_cast<uint8_t*>(writer.code)
                   - reinterpret_cast<uint8_t*>(writer.base);

    gum_arm64_relocator_clear(&rl);
    gum_arm64_writer_clear(&writer);

    output.resize(written);
    return { std::move(output), insns_size, n_insns };
}
