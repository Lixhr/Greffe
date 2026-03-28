#include "arm/ArmRelocator.hpp"

extern "C" {
#include <gum/arch-arm/gumarmrelocator.h>
#include <gum/arch-arm/gumarmwriter.h>
}

#include "GumRelocatorImpl.hpp"

struct ArmTraits {
    using Writer    = GumArmWriter;
    using Relocator = GumArmRelocator;
    static constexpr const char* relocator_name = "ArmRelocator";

    static void writer_init(Writer* w, void* buf)                          { gum_arm_writer_init(w, buf); }
    static void writer_flush(Writer* w)                                    { gum_arm_writer_flush(w); }
    static void writer_clear(Writer* w)                                    { gum_arm_writer_clear(w); }
    static void writer_put_bytes(Writer* w, const uint8_t* d, size_t n)   { gum_arm_writer_put_bytes(w, d, static_cast<guint>(n)); }

    static void     relocator_init(Relocator* r, const uint8_t* in, Writer* w) { gum_arm_relocator_init(r, in, w); }
    static guint    relocator_read_one(Relocator* r, const cs_insn** insn)      { return gum_arm_relocator_read_one(r, insn); }
    static gboolean relocator_eoi(Relocator* r)                                 { return gum_arm_relocator_eoi(r); }
    static gboolean relocator_write_one(Relocator* r)                           { return gum_arm_relocator_write_one(r); }
    static void     relocator_clear(Relocator* r)                               { gum_arm_relocator_clear(r); }
};

std::string_view ArmRelocator::name() const { return "ARM"; }

bool ArmRelocator::is_branch(const std::vector<uint8_t>& bytes, uint64_t ea) const {
    return is_branch_impl<ArmTraits>(bytes, ea);
}

RelocatedCode ArmRelocator::relocate(const std::vector<uint8_t>& input_bytes,
                                     size_t   n_bytes,
                                     uint64_t original_ea,
                                     uint64_t trampoline_addr,
                                     const std::vector<uint8_t>& trailer) {
    return relocate_impl<ArmTraits>(input_bytes, n_bytes, original_ea, trampoline_addr, trailer);
}