#include "thumb/ThumbStubs.hpp"

extern "C" {
#include <gum/arch-arm/gumthumbwriter.h>
#include <capstone/arm.h>
}

#include <stdexcept>

std::string_view ThumbStubs::name() const { return "Thumb"; }

static std::vector<uint8_t> thumb_collect(GumThumbWriter& w,
                                          std::vector<uint8_t>& buf,
                                          const char* ctx) {
    gum_thumb_writer_flush(&w);
    size_t written = reinterpret_cast<uint8_t*>(w.code)
                   - reinterpret_cast<uint8_t*>(w.base);
    gum_thumb_writer_clear(&w);
    if (written == 0)
        throw std::runtime_error(std::string(ctx) + ": writer produced no bytes");
    buf.resize(written);
    return std::move(buf);
}

std::vector<uint8_t> ThumbStubs::save_ctx(uint64_t at) {
    std::vector<uint8_t> buf(128, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);
    if (!gum_thumb_writer_put_push_regs(&w, 14,
            ARM_REG_R0,  ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
            ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
            ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
            ARM_REG_R12, ARM_REG_LR)) {
        gum_thumb_writer_clear(&w);
        throw std::runtime_error("ThumbStubs::save_ctx: put_push_regs failed");
    }
    gum_thumb_writer_put_mov_reg_cpsr(&w, ARM_REG_R0);
    if (!gum_thumb_writer_put_push_regs(&w, 1, ARM_REG_R0)) {
        gum_thumb_writer_clear(&w);
        throw std::runtime_error("ThumbStubs::save_ctx: cpsr push failed");
    }
    return thumb_collect(w, buf, "ThumbStubs::save_ctx");
}

std::vector<uint8_t> ThumbStubs::restore_ctx(uint64_t at) {
    std::vector<uint8_t> buf(128, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);
    if (!gum_thumb_writer_put_pop_regs(&w, 1, ARM_REG_R0)) {
        gum_thumb_writer_clear(&w);
        throw std::runtime_error("ThumbStubs::restore_ctx: cpsr pop failed");
    }
    if (!gum_thumb_writer_put_msr_reg_reg(&w, ARM_SYSREG_APSR_NZCVQ, ARM_REG_R0)) {
        gum_thumb_writer_clear(&w);
        throw std::runtime_error("ThumbStubs::restore_ctx: msr failed");
    }
    if (!gum_thumb_writer_put_pop_regs(&w, 14,
            ARM_REG_R0,  ARM_REG_R0,  ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
            ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
            ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
            ARM_REG_R12, ARM_REG_LR)) {
        gum_thumb_writer_clear(&w);
        throw std::runtime_error("ThumbStubs::restore_ctx: put_pop_regs failed");
    }
    return thumb_collect(w, buf, "ThumbStubs::restore_ctx");
}

static inline void write_branch(GumThumbWriter *w, uint64_t from, uint64_t to) {
    if (gum_thumb_writer_can_branch_directly_between(w, from, to))
        gum_thumb_writer_put_b_imm(w, static_cast<GumAddress>(to));
    else
        gum_thumb_writer_put_ldr_reg_address(w, ARM_REG_PC,
                                             static_cast<GumAddress>(to));
}

std::vector<uint8_t> ThumbStubs::branch(uint64_t from, uint64_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);

    write_branch(&w, from, to);
    return thumb_collect(w, buf, "ThumbStubs::branch");
}

std::vector<uint8_t> ThumbStubs::call(uint64_t from, uint64_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);
    gum_thumb_writer_put_bl_imm(&w, static_cast<GumAddress>(to));
    return thumb_collect(w, buf, "ThumbStubs::call");
}

std::vector<uint8_t> ThumbStubs::trampoline_init(uint64_t at, uint64_t shstub_addr, uint32_t id) {
    std::vector<uint8_t> buf(128, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    // saves the original R0
    if (!gum_thumb_writer_put_push_regs(&w, 1, ARM_REG_R0)) {
        gum_thumb_writer_clear(&w);
        throw std::runtime_error("ThumbStubs::trampoline_init: push R0 failed");
    }

    // R0 knows the id 
    if (id < UINT8_MAX)
        gum_thumb_writer_put_mov_reg_u8(&w, ARM_REG_R0, (guint8)id);
    else
        gum_thumb_writer_put_ldr_reg_u32(&w, ARM_REG_R0, id);

    // jump to the shared stub
    write_branch(&w, w.pc, shstub_addr);
    
    return thumb_collect(w, buf, "ThumbStubs::trampoline_init");
}
