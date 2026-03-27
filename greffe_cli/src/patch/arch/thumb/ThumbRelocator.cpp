#include "thumb/ThumbRelocator.hpp"

extern "C" {
#include <gum/arch-arm/gumthumbrelocator.h>
#include <gum/arch-arm/gumthumbwriter.h>
}

#include "GumRelocatorImpl.hpp"

struct ThumbTraits {
    using Writer    = GumThumbWriter;
    using Relocator = GumThumbRelocator;
    static constexpr const char* relocator_name = "ThumbRelocator";

    static void writer_init(Writer* w, void* buf)                          { gum_thumb_writer_init(w, buf); }
    static void writer_flush(Writer* w)                                    { gum_thumb_writer_flush(w); }
    static void writer_clear(Writer* w)                                    { gum_thumb_writer_clear(w); }
    static void writer_put_bytes(Writer* w, const uint8_t* d, size_t n)   { gum_thumb_writer_put_bytes(w, d, static_cast<guint>(n)); }

    static void     relocator_init(Relocator* r, const uint8_t* in, Writer* w) { gum_thumb_relocator_init(r, in, w); }
    static guint    relocator_read_one(Relocator* r, const cs_insn** insn)      { return gum_thumb_relocator_read_one(r, insn); }
    static gboolean relocator_eoi(Relocator* r)                                 { return gum_thumb_relocator_eoi(r); }
    static gboolean relocator_write_one(Relocator* r)                           { return gum_thumb_relocator_write_one(r); }
    static void     relocator_clear(Relocator* r)                               { gum_thumb_relocator_clear(r); }
};

std::string_view ThumbRelocator::name() const { return "Thumb"; }

bool ThumbRelocator::is_branch(const std::vector<uint8_t>& bytes, uint64_t ea) const {
    return is_branch_impl<ThumbTraits>(bytes, ea);
}

RelocatedCode ThumbRelocator::relocate(const std::vector<uint8_t>& input_bytes,
                                       size_t   n_bytes,
                                       uint64_t original_ea,
                                       uint64_t trampoline_addr,
                                       const std::vector<uint8_t>& trailer) {
    return relocate_impl<ThumbTraits>(input_bytes, n_bytes, original_ea, trampoline_addr, trailer);
}