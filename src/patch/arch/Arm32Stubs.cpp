#include "Arm32Stubs.hpp"
#include "ContextEntry.hpp"

extern "C" {
#include <gum/arch-arm/gumarmwriter.h>
#include <gum/arch-arm/gumarmrelocator.h>
#include <capstone/arm.h>
}

#include <stdexcept>

std::string Arm32Stubs::name() const { return "Arm32"; }

std::vector<uint8_t> Arm32Stubs::nop_bytes() const {
    return {0x00, 0xF0, 0x20, 0xE3};
}


static std::vector<uint8_t> arm_collect(GumArmWriter& w,
                                        std::vector<uint8_t>& buf,
                                        const char* ctx,
                                        bool big_endian = false) {
    size_t pre_flush = reinterpret_cast<uint8_t*>(w.code)
                     - reinterpret_cast<uint8_t*>(w.base);
    gum_arm_writer_flush(&w);
    size_t written = reinterpret_cast<uint8_t*>(w.code)
                   - reinterpret_cast<uint8_t*>(w.base);
    gum_arm_writer_clear(&w);
    if (written == 0)
        throw std::runtime_error(std::string(ctx) + ": writer produced no bytes");
    buf.resize(written);

    if (big_endian)
        fix_be_pool(buf, pre_flush);
    return std::move(buf);
}

void Arm32Stubs::save_ctx(GumArmWriter *w) {
    // R0 already saved by trampoline_init
    gum_arm_writer_put_push_regs(w, 13,
            ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
            ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
            ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
            ARM_REG_R12, ARM_REG_LR);
    gum_arm_writer_put_mov_reg_cpsr(w, ARM_REG_R1);
    gum_arm_writer_put_push_regs(w, 1, ARM_REG_R1);
}

void Arm32Stubs::restore_ctx(GumArmWriter *w) {
    gum_arm_writer_put_pop_regs(w, 1, ARM_REG_R0);
    gum_arm_writer_put_mov_cpsr_reg(w, ARM_REG_R0);
    gum_arm_writer_put_pop_regs(w, 13,
            ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
            ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
            ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
            ARM_REG_R12, ARM_REG_LR);
    gum_arm_writer_put_pop_regs(w, 1, ARM_REG_R0);
}

static inline void write_branch(GumArmWriter *w, ea_t from, ea_t to) {
    if (gum_arm_writer_can_branch_directly_between(w, from, to))
        gum_arm_writer_put_b_imm(w, static_cast<GumAddress>(to));
    else
        gum_arm_writer_put_ldr_reg_address(w, ARM_REG_PC,
                                           static_cast<GumAddress>(to));
}

std::vector<uint8_t> Arm32Stubs::branch(ea_t from, ea_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);

    write_branch(&w, from, to);
    return arm_collect(w, buf, "Arm32Stubs::branch", is_big_endian());
}

std::vector<uint8_t> Arm32Stubs::call(ea_t from, ea_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);
    gum_arm_writer_put_bl_imm(&w, static_cast<GumAddress>(to));
    return arm_collect(w, buf, "Arm32Stubs::call", is_big_endian());
}

std::vector<uint8_t> Arm32Stubs::trampoline_init(ea_t at,
                                                  ea_t shstub_addr,
                                                  uint8_t **ptr_array) {
    std::vector<uint8_t> buf(128, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    // reserve slot for the return address
    gum_arm_writer_put_sub_reg_u32(&w, ARM_REG_SP, 0x4);

    // save original R0
    gum_arm_writer_put_push_regs(&w, 1, ARM_REG_R0);

    // In ARM mode PC = current_instr + 8, so at this instruction PC points
    // 8 bytes ahead — which is exactly where the literal pool lands after
    // the following 4-byte branch.
    gum_arm_writer_put_add_reg_reg_imm(&w, ARM_REG_R0, ARM_REG_PC, 0);
    write_branch(&w, w.pc, shstub_addr);

    std::vector<uint8_t> bytes = arm_collect(w, buf, "Arm32Stubs::trampoline_init", is_big_endian());
    *ptr_array = reinterpret_cast<uint8_t*>(bytes.data() + bytes.size());

    // reserve literal pool slot for handler address
    bytes.resize(bytes.size() + sizeof_ptr());
    return bytes;
}

std::vector<uint8_t> Arm32Stubs::relocate_and_branch_back(
                        const std::vector<ContextEntry>& instrs,
                        ea_t                             dest_addr,
                        ea_t                             branch_to) {
    std::vector<uint8_t> buf(256, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(dest_addr);

    for (const ContextEntry& e : instrs) {
        GumArmRelocator r;
        gum_arm_relocator_init(&r, e.raw.data(), &w);
        r.input_pc = static_cast<GumAddress>(e.ea);
        guint consumed = 0;
        do {
            consumed = gum_arm_relocator_read_one(&r, nullptr);
        } while (consumed != 0 && consumed < e.raw.size());
        gum_arm_relocator_write_all(&r);
        gum_arm_relocator_clear(&r);
    }

    ea_t br_from = dest_addr + (reinterpret_cast<uint8_t*>(w.code)
                                  - reinterpret_cast<uint8_t*>(w.base));
    write_branch(&w, br_from, branch_to);

    return arm_collect(w, buf, "Arm32Stubs::relocate_and_branch_back", is_big_endian());
}

std::vector<uint8_t> Arm32Stubs::build_shared_stub(ea_t at) {
    std::vector<uint8_t> buf(0x400, 0);
    GumArmWriter w;
    gum_arm_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    save_ctx(&w);

    // R0 = literal pool entry passed from trampoline: [0] handler ptr, [4] return addr
    gum_arm_writer_put_ldr_reg_reg(&w, ARM_REG_R1, ARM_REG_R0);
    gum_arm_writer_put_add_reg_reg_imm(&w, ARM_REG_R0, ARM_REG_R0, 0x4);

    // store return addr into the slot reserved at the bottom of the saved context
    gum_arm_writer_put_str_reg_reg_offset(&w, ARM_REG_R0, ARM_REG_SP, 0x3c);

    // call the handler
    gum_arm_writer_put_blx_reg(&w, ARM_REG_R1);

    restore_ctx(&w);

    // branch to the return addr (was stored at SP, which POP {PC} now loads)
    gum_arm_writer_put_pop_regs(&w, 1, ARM_REG_PC);

    return arm_collect(w, buf, "Arm32Stubs::build_shared_stub", is_big_endian());
}
