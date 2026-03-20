#include "arch/ThumbRelocator.hpp"

extern "C" {
#include <gum/arch-arm/gumthumbrelocator.h>
#include <gum/arch-arm/gumthumbwriter.h>
}

#include <stdexcept>

std::string_view ThumbRelocator::name() const { return "Thumb"; }

RelocatedCode ThumbRelocator::relocate(const std::vector<uint8_t>& input_bytes,
                                       size_t   n_bytes,
                                       uint64_t original_ea,
                                       uint64_t trampoline_addr)
{
    if (input_bytes.empty() || n_bytes == 0)
        throw std::runtime_error("ThumbRelocator: empty input");

    std::vector<uint8_t> output(512, 0);

    GumThumbWriter writer;
    gum_thumb_writer_init(&writer, output.data());
    writer.pc = static_cast<GumAddress>(trampoline_addr);

    GumThumbRelocator rl;
    gum_thumb_relocator_init(&rl, input_bytes.data(), &writer);
    rl.input_pc = static_cast<GumAddress>(original_ea);

    size_t n_insns = 0;
    const cs_insn* insn = nullptr;
    while (gum_thumb_relocator_read_one(&rl, &insn) != 0) {
        ++n_insns;
        size_t consumed = (size_t)(rl.input_cur - rl.input_start);
        if (consumed >= n_bytes || gum_thumb_relocator_eoi(&rl))
            break;
    }

    if (n_insns == 0)
        throw std::runtime_error("ThumbRelocator: failed to read any instruction");

    while (gum_thumb_relocator_write_one(&rl)) {}

    gum_thumb_writer_flush(&writer);

    size_t written = reinterpret_cast<uint8_t*>(writer.code)
                   - reinterpret_cast<uint8_t*>(writer.base);

    gum_thumb_relocator_clear(&rl);
    gum_thumb_writer_clear(&writer);

    output.resize(written);
    return { std::move(output), n_insns };
}
