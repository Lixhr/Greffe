#include "Arm32Stubs.hpp"
#include "ContextEntry.hpp"

extern "C" {
#include <gum/arch-arm/gumarmwriter.h>
#include <gum/arch-arm/gumarmrelocator.h>
#include <capstone/arm.h>
}

#include <stdexcept>

std::string Arm32Stubs::name() const { return "Thumb"; }


// static std::vector<uint8_t> thumb_collect(GumArmWriter& w,
//                                           std::vector<uint8_t>& buf,
//                                           const char* ctx) {

//     (void)w;
//     (void)buf;
//     (void)ctx;
//     // gum_arm_writer_flush(&w);
//     // size_t written = reinterpret_cast<uint8_t*>(w.code)
//     //                - reinterpret_cast<uint8_t*>(w.base);
//     // gum_arm_writer_clear(&w);
//     // if (written == 0)
//     //     throw std::runtime_error(std::string(ctx) + ": writer produced no bytes");
//     // buf.resize(written);
//     // return std::move(buf);
//     return buf;
// }

void Arm32Stubs::save_ctx(GumArmWriter *w) {
    (void)w;

    // R0 already saved
    // gum_arm_writer_put_push_regs(w, 13,
    //         ARM_REG_R1,  ARM_REG_R2,  ARM_REG_R3,
    //         ARM_REG_R4,  ARM_REG_R5,  ARM_REG_R6,  ARM_REG_R7,
    //         ARM_REG_R8,  ARM_REG_R9,  ARM_REG_R10, ARM_REG_R11,
    //         ARM_REG_R12, ARM_REG_LR);

    // gum_arm_writer_put_mov_reg_cpsr(w, ARM_REG_R1);
    // gum_arm_writer_put_push_regs(w, 1, ARM_REG_R1);
}

void Arm32Stubs::restore_ctx(GumArmWriter *w) {
    (void)w;
}

static inline void write_branch(GumArmWriter *w, ea_t from, ea_t to) {
    (void)w;
    (void)from;
    (void)to;
}

std::vector<uint8_t> Arm32Stubs::branch(ea_t from, ea_t to) {
    std::vector<uint8_t> buf(16, 0);
    // GumArmWriter w;
    // gum_arm_writer_init(&w, buf.data());
    // w.pc = static_cast<GumAddress>(from);

    // write_branch(&w, from, to);
    // return thumb_collect(w, buf, "Arm32Stubs::branch");
    (void)from;
    (void)to;
    return buf;
}

std::vector<uint8_t> Arm32Stubs::call(ea_t from, ea_t to) {
    (void)from;
    (void)to;
    std::vector<uint8_t> buf(16, 0);
    // GumArmWriter w;
    // gum_arm_writer_init(&w, buf.data());
    // w.pc = static_cast<GumAddress>(from);
    // gum_arm_writer_put_bl_imm(&w, static_cast<GumAddress>(to));
    // return thumb_collect(w, buf, "Arm32Stubs::call");
    return buf;

}

std::vector<uint8_t> Arm32Stubs::trampoline_init(ea_t at, 
                                                 ea_t shstub_addr, 
                                                 uint8_t  **ptr_array) {
    (void) at;
    (void) shstub_addr;
    (void) ptr_array;
    std::vector<uint8_t> buf(128, 0);
    // GumArmWriter w;
    // gum_arm_writer_init(&w, buf.data());
    // w.pc = static_cast<GumAddress>(at);

    // // this first item on the stack will hold the 'return' addr 
    // gum_arm_writer_put_add_reg_imm(&w, ARM_REG_SP, -0x4);

    // // saves the original R0
    // gum_arm_writer_put_push_regs(&w, 1, ARM_REG_R0);

    // // get the literal pool address
    // gum_arm_writer_put_add_reg_reg_imm(&w, ARM_REG_R0, ARM_REG_PC, 4);
    // write_branch(&w, w.pc, shstub_addr);
    

    // // align 
    // gum_arm_writer_put_nop(&w);

    // std::vector<uint8_t> bytes = thumb_collect(w, buf, "Arm32Stubs::trampoline_init");
    // *ptr_array = reinterpret_cast<uint8_t *>(bytes.data() + bytes.size());

    // // reserve fake literal pool for handler address
    // bytes.resize(bytes.size() + sizeof_ptr());
    return buf;
}

std::vector<uint8_t> Arm32Stubs::relocate_and_branch_back(
                        const std::vector<ContextEntry>& instrs,
                        ea_t                         dest_addr,
                        ea_t                         branch_to) {
    std::vector<uint8_t> buf(256, 0);
    (void)instrs;
    (void)dest_addr;
    (void)branch_to;
    // GumArmWriter w;
    // gum_arm_writer_init(&w, buf.data());
    // w.pc = static_cast<GumAddress>(dest_addr);

    // for (const ContextEntry& e : instrs) {
    //     GumArmRelocator r;
    //     gum_arm_relocator_init(&r, e.raw.data(), &w);
    //     r.input_pc = static_cast<GumAddress>(e.ea);
    //     gum_arm_relocator_read_one(&r, nullptr);
    //     gum_arm_relocator_write_one(&r);
    //     gum_arm_relocator_clear(&r);
    // }

    // ea_t br_from = dest_addr + (reinterpret_cast<uint8_t*>(w.code)
    //                               - reinterpret_cast<uint8_t*>(w.base));
    // write_branch(&w, br_from, branch_to);

    // return thumb_collect(w, buf, "Arm32Stubs::relocate_and_branch_back");
    return buf;
}

std::vector<uint8_t> Arm32Stubs::build_shared_stub(ea_t at) {
    (void)at;
    std::vector<uint8_t> buf(0x400, 0);
    // GumArmWriter w;
    // gum_arm_writer_init(&w, buf.data());
    // w.pc = static_cast<GumAddress>(at);

    // save_ctx(&w);


    // // get the funcptr
    // gum_arm_writer_put_ldr_reg_reg(&w, ARM_REG_R1, ARM_REG_R0);

    // // get the 'ret' address, ensure the thumb bit is set
    // gum_arm_writer_put_add_reg_reg_imm(&w, ARM_REG_R0, ARM_REG_R0, 0x4 | 1);


    // // space is reserved at the bottom of our stack
    // // it stores the 'ret' to be available on POP PC
    // gum_arm_writer_put_str_reg_reg_offset(&w, ARM_REG_R0, ARM_REG_SP, 0x3c);

    // // call the handler                                                                                                                                                       
    // gum_arm_writer_put_blx_reg(&w, ARM_REG_R1);   

    // restore_ctx(&w);

    // // branch to the 'ret'
    // gum_arm_writer_put_pop_regs(&w, 1, ARM_REG_PC);

    // return thumb_collect(w, buf, "Arm32Stubs::build_shared_stub");

    return buf;
}
