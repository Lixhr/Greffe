#include "arm64/Arm64Relocator.hpp"

extern "C" {
#include <gum/arch-arm64/gumarm64relocator.h>
#include <gum/arch-arm64/gumarm64writer.h>
}

#include "GumRelocatorImpl.hpp"

struct Arm64Traits {
    using Writer    = GumArm64Writer;
    using Relocator = GumArm64Relocator;
    static constexpr const char* relocator_name = "Arm64Relocator";

    static void writer_init(Writer* w, void* buf)                          { gum_arm64_writer_init(w, buf); }
    static void writer_flush(Writer* w)                                    { gum_arm64_writer_flush(w); }
    static void writer_clear(Writer* w)                                    { gum_arm64_writer_clear(w); }
    static void writer_put_bytes(Writer* w, const uint8_t* d, size_t n)   { gum_arm64_writer_put_bytes(w, d, static_cast<guint>(n)); }

    static void     relocator_init(Relocator* r, const uint8_t* in, Writer* w) { gum_arm64_relocator_init(r, in, w); }
    static guint    relocator_read_one(Relocator* r, const cs_insn** insn)      { return gum_arm64_relocator_read_one(r, insn); }
    static gboolean relocator_eoi(Relocator* r)                                 { return gum_arm64_relocator_eoi(r); }
    static gboolean relocator_write_one(Relocator* r)                           { return gum_arm64_relocator_write_one(r); }
    static void     relocator_clear(Relocator* r)                               { gum_arm64_relocator_clear(r); }
};

std::string_view Arm64Relocator::name() const { return "ARM64"; }

bool Arm64Relocator::is_branch(const std::vector<uint8_t>& bytes, uint64_t ea) const {
    return is_branch_impl<Arm64Traits>(bytes, ea);
}

RelocatedCode Arm64Relocator::relocate(const std::vector<uint8_t>& input_bytes,
                                       size_t   n_bytes,
                                       uint64_t original_ea,
                                       uint64_t trampoline_addr,
                                       const std::vector<uint8_t>& trailer) {
    return relocate_impl<Arm64Traits>(input_bytes, n_bytes, original_ea, trampoline_addr, trailer);
}