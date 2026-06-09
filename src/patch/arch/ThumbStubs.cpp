#include "ThumbStubs.hpp"
#include "ContextEntry.hpp"

extern "C" {
#include <gum/arch-arm/gumthumbwriter.h>
#include <gum/arch-arm/gumthumbrelocator.h>
#include <capstone/arm.h>
}

#include <stdexcept>

std::string ThumbStubs::name() const { return "Thumb"; }


static std::vector<uint8_t> thumb_collect(GumThumbWriter& w,
                                          std::vector<uint8_t>& buf,
                                          const char* ctx,
                                          bool big_endian = false) {
    size_t      pre_flush = reinterpret_cast<uint8_t*>(w.code)
                          - reinterpret_cast<uint8_t*>(w.base);
    GumAddress  pre_pc    = w.pc;
    gum_thumb_writer_flush(&w);
    size_t written = reinterpret_cast<uint8_t*>(w.code)
                   - reinterpret_cast<uint8_t*>(w.base);
    gum_thumb_writer_clear(&w);
    if (written == 0)
        throw std::runtime_error(std::string(ctx) + ": writer produced no bytes");
    buf.resize(written);

    if (big_endian)
        fix_be_pool(buf, pre_flush + ((pre_pc & 2) ? 2 : 0));

    return std::move(buf);
}

void ThumbStubs::save_ctx(GumThumbWriter *w) {
    // R0 already saved
    if (!gum_thumb_writer_put_push_regs(w, 13,
            ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
            ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
            ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
            ARM_REG_R12, ARM_REG_LR)) {
        gum_thumb_writer_clear(w);
        throw std::runtime_error("ThumbStubs::save_ctx: put_push_regs failed");
    }
    gum_thumb_writer_put_mov_reg_cpsr(w, ARM_REG_R1);
    if (!gum_thumb_writer_put_push_regs(w, 1, ARM_REG_R1)) {
        gum_thumb_writer_clear(w);
        throw std::runtime_error("ThumbStubs::save_ctx: cpsr push failed");
    }
}

void ThumbStubs::restore_ctx(GumThumbWriter *w) {
    if (!gum_thumb_writer_put_pop_regs(w, 1, ARM_REG_R0)) {
        gum_thumb_writer_clear(w);
        throw std::runtime_error("ThumbStubs::restore_ctx: cpsr pop failed");
    }
    if (!gum_thumb_writer_put_msr_reg_reg(w, ARM_SYSREG_APSR_NZCVQ, ARM_REG_R0)) {
        gum_thumb_writer_clear(w);
        throw std::runtime_error("ThumbStubs::restore_ctx: msr failed");
    }
    if (!gum_thumb_writer_put_pop_regs(w, 13,
            ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
            ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
            ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
            ARM_REG_R12, ARM_REG_LR)) {
        gum_thumb_writer_clear(w);
        throw std::runtime_error("ThumbStubs::restore_ctx: put_pop_regs failed");
    }

    // restore original R0
    if (!gum_thumb_writer_put_pop_regs(w, 1, ARM_REG_R0)) {
        gum_thumb_writer_clear(w);
        throw std::runtime_error("ThumbStubs::restore_ctx: r0 pop failed");
    }
}

static inline void write_branch(GumThumbWriter *w, ea_t from, ea_t to) {
    if (gum_thumb_writer_can_branch_directly_between(w, from, to))
        gum_thumb_writer_put_b_imm(w, static_cast<GumAddress>(to));
    else
        gum_thumb_writer_put_ldr_reg_address(w, ARM_REG_PC,
                                             static_cast<GumAddress>(to | 1)); // thumb bit set
}

std::vector<uint8_t> ThumbStubs::branch(ea_t from, ea_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);

    write_branch(&w, from, to);
    return thumb_collect(w, buf, "ThumbStubs::branch", is_big_endian());
}

std::vector<uint8_t> ThumbStubs::call(ea_t from, ea_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);
    gum_thumb_writer_put_bl_imm(&w, static_cast<GumAddress>(to));
    return thumb_collect(w, buf, "ThumbStubs::call", is_big_endian());
}

std::vector<uint8_t> ThumbStubs::trampoline_init(ea_t at,
                                                 ea_t shstub_addr,
                                                 uint8_t **ptr_array,
                                                 uint8_t **id_array) {
    std::vector<uint8_t> buf(128, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    // this first item on the stack will hold the 'return' addr
    if (!gum_thumb_writer_put_add_reg_imm(&w, ARM_REG_SP, -0x4))
        throw std::runtime_error("ThumbStubs::trampoline_init: push R0 failed");

    // saves the original R0
    if (!gum_thumb_writer_put_push_regs(&w, 1, ARM_REG_R0)) {
        throw std::runtime_error("ThumbStubs::trampoline_init: push R0 failed");
    }

    gum_thumb_writer_put_add_reg_reg_imm(&w, ARM_REG_R0, ARM_REG_PC, 4);
    write_branch(&w, w.pc, shstub_addr);

    gum_thumb_writer_put_nop(&w);

    std::vector<uint8_t> bytes = thumb_collect(w, buf, "ThumbStubs::trampoline_init", is_big_endian());

    size_t pool_offset = bytes.size();
    bytes.resize(bytes.size() + 2 * sizeof_ptr());
    *ptr_array = bytes.data() + pool_offset;
    *id_array  = bytes.data() + pool_offset + sizeof_ptr();
    return bytes;
}

std::vector<uint8_t> ThumbStubs::relocate_and_branch_back(
                        const std::vector<ContextEntry>& instrs,
                        ea_t                         dest_addr,
                        ea_t                         branch_to) {
    std::vector<uint8_t> buf(256, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(dest_addr);

    std::vector<uint8_t> combined;
    for (const auto& e : instrs)
        combined.insert(combined.end(), e.raw.begin(), e.raw.end());

    GumThumbRelocator r;
    gum_thumb_relocator_init(&r, combined.data(), &w);
    r.input_pc = static_cast<GumAddress>(instrs[0].ea);

    guint consumed = 0;
    do {
        consumed = gum_thumb_relocator_read_one(&r, nullptr);
    } while (consumed != 0 && consumed < combined.size());

    gum_thumb_relocator_write_all(&r);
    gum_thumb_relocator_clear(&r);

    ea_t br_from = dest_addr + (reinterpret_cast<uint8_t*>(w.code)
                                  - reinterpret_cast<uint8_t*>(w.base));
    write_branch(&w, br_from, branch_to);

    return thumb_collect(w, buf, "ThumbStubs::relocate_and_branch_back", is_big_endian());
}

bool ThumbStubs::at_reloc_boundary(const std::vector<ContextEntry>& collected) const {
    int it_remaining = 0;
    for (const auto& e : collected) {
        if (it_remaining > 0) {
            --it_remaining;
            continue;
        }
        if (e.raw.size() != 2)
            continue;
        uint16_t hw = (uint16_t(e.raw[1]) << 8) | e.raw[0];
        if ((hw >> 8) != 0xBF || (hw & 0x0F) == 0)
            continue;
        uint8_t mask = hw & 0x0F;
        if      (mask & 0x1) it_remaining = 4;
        else if (mask & 0x2) it_remaining = 3;
        else if (mask & 0x4) it_remaining = 2;
        else                 it_remaining = 1;
    }
    return it_remaining == 0;
}

std::vector<uint8_t> ThumbStubs::nop_bytes() const {
    return {0x00, 0xBF};
}

std::vector<uint8_t> ThumbStubs::build_shared_stub(ea_t at) {
    std::vector<uint8_t> buf(0x400, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    save_ctx(&w);

    gum_thumb_writer_put_ldr_reg_reg(&w, ARM_REG_R1, ARM_REG_R0);                    // R1 = funcptr
    gum_thumb_writer_put_ldr_reg_reg_offset(&w, ARM_REG_R2, ARM_REG_R0, 4);          // R2 = greffe_id

    gum_thumb_writer_put_add_reg_imm(&w, ARM_REG_R0, 0x8 | 1);                       // R0 = ret addr (pool+9)
    gum_thumb_writer_put_str_reg_reg_offset(&w, ARM_REG_R0, ARM_REG_SP, 0x3c);       // store ret addr

    gum_thumb_writer_put_mov_reg_reg(&w, ARM_REG_R0, ARM_REG_R2);                    // R0 = greffe_id
    gum_thumb_writer_put_blx_reg(&w, ARM_REG_R1);

    restore_ctx(&w);

    // branch to the 'ret'
    gum_thumb_writer_put_pop_regs(&w, 1, ARM_REG_PC);

    return thumb_collect(w, buf, "ThumbStubs::build_shared_stub", is_big_endian());
}
