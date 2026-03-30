#include "arm/ArmStubs.hpp"

extern "C" {
#include <gum/arch-arm/gumarmwriter.h>
}

#include <stdexcept>

std::string_view ArmStubs::name() const { return "ARM"; }

static std::vector<uint8_t> arm_collect(GumArmWriter& w,
                                        std::vector<uint8_t>& buf,
                                        const char* ctx) {
    gum_arm_writer_flush(&w);
    size_t written = reinterpret_cast<uint8_t*>(w.code)
                   - reinterpret_cast<uint8_t*>(w.base);
    gum_arm_writer_clear(&w);
    if (written == 0)
        throw std::runtime_error(std::string(ctx) + ": writer produced no bytes");
    buf.resize(written);
    return std::move(buf);
}

std::vector<uint8_t> ArmStubs::save_ctx(uint64_t at) {
    std::vector<uint8_t> buf(128, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);
    gum_arm_writer_put_push_regs(&w, 14,
        ARM_REG_R0,  ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
        ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
        ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
        ARM_REG_R12, ARM_REG_LR);
    gum_arm_writer_put_mov_reg_cpsr(&w, ARM_REG_R0);
    gum_arm_writer_put_push_regs(&w, 1, ARM_REG_R0);
    return arm_collect(w, buf, "ArmStubs::save_ctx");
}

std::vector<uint8_t> ArmStubs::restore_ctx(uint64_t at) {
    std::vector<uint8_t> buf(128, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);
    gum_arm_writer_put_pop_regs(&w, 1, ARM_REG_R0);
    gum_arm_writer_put_mov_cpsr_reg(&w, ARM_REG_R0);
    gum_arm_writer_put_pop_regs(&w, 14,
        ARM_REG_R0,  ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
        ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
        ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
        ARM_REG_R12, ARM_REG_LR);
    return arm_collect(w, buf, "ArmStubs::restore_ctx");
}

std::vector<uint8_t> ArmStubs::branch(uint64_t from, uint64_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);
    if (gum_arm_writer_can_branch_directly_between(&w, from, to))
        gum_arm_writer_put_b_imm(&w, static_cast<GumAddress>(to));
    else
        gum_arm_writer_put_ldr_reg_address(&w, ARM_REG_PC,
                                           static_cast<GumAddress>(to));
    return arm_collect(w, buf, "ArmStubs::branch");
}

std::vector<uint8_t> ArmStubs::call(uint64_t from, uint64_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);
    gum_arm_writer_put_bl_imm(&w, static_cast<GumAddress>(to));
    return arm_collect(w, buf, "ArmStubs::call");
}
