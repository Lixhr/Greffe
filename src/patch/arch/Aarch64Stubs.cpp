#include "Aarch64Stubs.hpp"
#include "ContextEntry.hpp"

#include <glib.h>
#include <stdexcept>

std::string Arm64Stubs::name() const { return "Aarch64"; }

std::vector<uint8_t> Arm64Stubs::nop_bytes() const {
    return {0x1F, 0x20, 0x03, 0xD5};
}

static std::vector<uint8_t> arm64_collect(GumArm64Writer& w,
                                           std::vector<uint8_t>& buf,
                                           const char* ctx) {
    gum_arm64_writer_flush(&w);
    size_t written = reinterpret_cast<uint8_t*>(w.code)
                   - reinterpret_cast<uint8_t*>(w.base);
    gum_arm64_writer_clear(&w);
    if (written == 0)
        throw std::runtime_error(std::string(ctx) + ": writer produced no bytes");
    buf.resize(written);
    return std::move(buf);
}

static void init_writer(GumArm64Writer& w, void* buf, ea_t pc, bool big_endian) {
    gum_arm64_writer_init(&w, buf);
    w.pc         = static_cast<GumAddress>(pc);
    w.data_endian = big_endian ? G_BIG_ENDIAN : G_LITTLE_ENDIAN;
}

void Arm64Stubs::save_ctx(GumArm64Writer *w) {
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X1,  ARM64_REG_X2);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X3,  ARM64_REG_X4);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X5,  ARM64_REG_X6);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X7,  ARM64_REG_X8);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X9,  ARM64_REG_X10);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X11, ARM64_REG_X12);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X13, ARM64_REG_X14);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X15, ARM64_REG_X16);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X17, ARM64_REG_X18);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X19, ARM64_REG_X20);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X21, ARM64_REG_X22);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X23, ARM64_REG_X24);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X25, ARM64_REG_X26);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X27, ARM64_REG_X28);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_FP,  ARM64_REG_LR);
    gum_arm64_writer_put_mov_reg_nzcv(w, ARM64_REG_X1);
    gum_arm64_writer_put_push_reg_reg(w, ARM64_REG_X1,  ARM64_REG_XZR); // NZCV + pad
}

void Arm64Stubs::restore_ctx(GumArm64Writer *w) {
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X1, ARM64_REG_XZR);   // NZCV
    gum_arm64_writer_put_mov_nzcv_reg(w, ARM64_REG_X1);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_FP,  ARM64_REG_LR);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X27, ARM64_REG_X28);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X25, ARM64_REG_X26);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X23, ARM64_REG_X24);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X21, ARM64_REG_X22);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X19, ARM64_REG_X20);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X17, ARM64_REG_X18);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X15, ARM64_REG_X16);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X13, ARM64_REG_X14);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X11, ARM64_REG_X12);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X9,  ARM64_REG_X10);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X7,  ARM64_REG_X8);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X5,  ARM64_REG_X6);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X3,  ARM64_REG_X4);
    gum_arm64_writer_put_pop_reg_reg(w, ARM64_REG_X1,  ARM64_REG_X2);
}

std::vector<uint8_t> Arm64Stubs::branch(ea_t from, ea_t to) {
    std::vector<uint8_t> buf(32, 0);
    GumArm64Writer w;
    init_writer(w, buf.data(), from, is_big_endian());
    gum_arm64_writer_put_branch_address(&w, static_cast<GumAddress>(to));
    return arm64_collect(w, buf, "Arm64Stubs::branch");
}

std::vector<uint8_t> Arm64Stubs::call(ea_t from, ea_t to) {
    std::vector<uint8_t> buf(32, 0);
    GumArm64Writer w;
    init_writer(w, buf.data(), from, is_big_endian());
    if (gum_arm64_writer_can_branch_directly_between(&w, from, to))
        gum_arm64_writer_put_bl_imm(&w, static_cast<GumAddress>(to));
    else {
        gum_arm64_writer_put_ldr_reg_address(&w, ARM64_REG_X16, static_cast<GumAddress>(to));
        gum_arm64_writer_put_blr_reg(&w, ARM64_REG_X16);
    }
    return arm64_collect(w, buf, "Arm64Stubs::call");
}

std::vector<uint8_t> Arm64Stubs::trampoline_init(ea_t at,
                                                  ea_t shstub_addr,
                                                  uint8_t **ptr_array,
                                                  uint8_t **id_array) {
    std::vector<uint8_t> buf(64, 0);
    GumArm64Writer w;
    init_writer(w, buf.data(), at, is_big_endian());

    gum_arm64_writer_put_push_reg_reg(&w, ARM64_REG_X0, ARM64_REG_XZR);

    gum_arm64_writer_put_instruction(&w, 0x100000A0u);

    gum_arm64_writer_put_ldr_reg_address(&w, ARM64_REG_X16, static_cast<GumAddress>(shstub_addr));
    gum_arm64_writer_put_br_reg(&w, ARM64_REG_X16);

    std::vector<uint8_t> bytes = arm64_collect(w, buf, "Arm64Stubs::trampoline_init");

    size_t pool_offset = bytes.size();
    bytes.resize(bytes.size() + 2 * sizeof_ptr());
    *ptr_array = bytes.data() + pool_offset;
    *id_array  = bytes.data() + pool_offset + sizeof_ptr();
    return bytes;
}

std::vector<uint8_t> Arm64Stubs::build_shared_stub(ea_t at) {
    std::vector<uint8_t> buf(0x800, 0);
    GumArm64Writer w;
    init_writer(w, buf.data(), at, is_big_endian());

    save_ctx(&w);

    gum_arm64_writer_put_ldr_reg_reg(&w, ARM64_REG_X1, ARM64_REG_X0);                     // X1 = funcptr
    gum_arm64_writer_put_add_reg_reg_imm(&w, ARM64_REG_X2, ARM64_REG_X0, 16);             // X2 = ret addr
    gum_arm64_writer_put_str_reg_reg_offset(&w, ARM64_REG_X2, ARM64_REG_SP, 264);         // store ret addr
    gum_arm64_writer_put_ldr_reg_reg_offset(&w, ARM64_REG_X0, ARM64_REG_X0, 8);           // X0 = greffe_id
    gum_arm64_writer_put_blr_reg(&w, ARM64_REG_X1);

    restore_ctx(&w);

    // restore orig X0 and branch to ret addr
    gum_arm64_writer_put_ldr_reg_reg_offset(&w, ARM64_REG_X0,  ARM64_REG_SP, 0);
    gum_arm64_writer_put_ldr_reg_reg_offset(&w, ARM64_REG_X16, ARM64_REG_SP, 8);
    gum_arm64_writer_put_add_reg_reg_imm(&w, ARM64_REG_SP, ARM64_REG_SP, 16);
    gum_arm64_writer_put_br_reg(&w, ARM64_REG_X16);

    return arm64_collect(w, buf, "Arm64Stubs::build_shared_stub");
}

std::vector<uint8_t> Arm64Stubs::relocate_and_branch_back(
                        const std::vector<ContextEntry>& instrs,
                        ea_t                             dest_addr,
                        ea_t                             branch_to) {
    std::vector<uint8_t> buf(256, 0);
    GumArm64Writer w;
    init_writer(w, buf.data(), dest_addr, is_big_endian());

    for (const ContextEntry& e : instrs) {
        GumArm64Relocator r;
        gum_arm64_relocator_init(&r, e.raw.data(), &w);
        r.input_pc = static_cast<GumAddress>(e.ea);
        guint consumed = 0;
        do {
            consumed = gum_arm64_relocator_read_one(&r, nullptr);
        } while (consumed != 0 && consumed < e.raw.size());
        gum_arm64_relocator_write_all(&r);
        gum_arm64_relocator_clear(&r);
    }

    gum_arm64_writer_put_branch_address(&w, static_cast<GumAddress>(branch_to));

    return arm64_collect(w, buf, "Arm64Stubs::relocate_and_branch_back");
}
