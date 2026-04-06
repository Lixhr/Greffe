#include "thumb/ThumbStubs.hpp"
#include "Target.hpp"

extern "C" {
#include <gum/arch-arm/gumthumbwriter.h>
#include <gum/arch-arm/gumthumbrelocator.h>
#include <capstone/arm.h>
}

#include <stdexcept>

std::string_view ThumbStubs::name() const { return "Thumb"; }

void ThumbStubs::write_ptr(uint8_t* dst, uint64_t addr) const {
    uint32_t v = static_cast<uint32_t>(addr);
    dst[0] = static_cast<uint8_t>(v);
    dst[1] = static_cast<uint8_t>(v >> 8);
    dst[2] = static_cast<uint8_t>(v >> 16);
    dst[3] = static_cast<uint8_t>(v >> 24);
}

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

std::vector<uint8_t> ThumbStubs::trampoline_init(uint64_t at, 
                                                 uint64_t shstub_addr, 
                                                 uint8_t  **ptr_array) {
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


    // get the literal pool address
    gum_thumb_writer_put_add_reg_reg_imm(&w, ARM_REG_R0, ARM_REG_PC, 4);
    write_branch(&w, w.pc, shstub_addr);
    

    // align 
    gum_thumb_writer_put_nop(&w);

    std::vector<uint8_t> bytes = thumb_collect(w, buf, "ThumbStubs::trampoline_init");
    *ptr_array = reinterpret_cast<uint8_t *>(bytes.data() + bytes.size());

    // reserve fake literal pool for handler address
    bytes.resize(bytes.size() + sizeof_ptr());
    return bytes;
}

std::vector<uint8_t> ThumbStubs::relocate_and_branch_back(
                        const std::vector<const ContextEntry*>& instrs,
                        uint64_t                                dest_addr,
                        uint64_t                                branch_to) {
    std::vector<uint8_t> buf(256, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(dest_addr);

    for (const ContextEntry* e : instrs) {
        GumThumbRelocator r;
        gum_thumb_relocator_init(&r, e->raw.data(), &w);
        r.input_pc = static_cast<GumAddress>(e->ea);
        gum_thumb_relocator_read_one(&r, nullptr);
        gum_thumb_relocator_write_one(&r);
        gum_thumb_relocator_clear(&r);
    }

    uint64_t br_from = dest_addr + (reinterpret_cast<uint8_t*>(w.code)
                                  - reinterpret_cast<uint8_t*>(w.base));
    write_branch(&w, br_from, branch_to);

    return thumb_collect(w, buf, "ThumbStubs::relocate_and_branch_back");
}

std::vector<uint8_t> ThumbStubs::build_shared_stub(uint64_t at) {
    std::vector<uint8_t> buf(0x400, 0);
    GumThumbWriter w;
    gum_thumb_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    save_ctx(&w);


    // get the funcptr
    gum_thumb_writer_put_ldr_reg_reg(&w, ARM_REG_R1, ARM_REG_R0);

    // get the 'ret' address, ensure the thumb bit is set
    gum_thumb_writer_put_add_reg_reg_imm(&w, ARM_REG_R0, ARM_REG_R0, 0x4 | 1);


    // space is reserved at the bottom of our stack
    // it stores the 'ret' to be available on POP PC
    // gum_thumb_writer_put_str_reg_reg(&w, ARM_REG_R0, ARM_REG_SP);
    gum_thumb_writer_put_str_reg_reg_offset(&w, ARM_REG_R0, ARM_REG_SP, 0x3c);

    // call the handler                                                                                                                                                       
    gum_thumb_writer_put_blx_reg(&w, ARM_REG_R1);   

    restore_ctx(&w);

    // branch to the 'ret'
    gum_thumb_writer_put_pop_regs(&w, 1, ARM_REG_PC);

    return thumb_collect(w, buf, "ThumbStubs::build_shared_stub");
}
