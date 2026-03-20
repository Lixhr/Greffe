#include "arch/Arm64Stubs.hpp"

extern "C" {
#include <gum/arch-arm64/gumarm64writer.h>
}

#include <stdexcept>

std::string_view Arm64Stubs::name() const { return "ARM64"; }

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

static void push_pair(GumArm64Writer& w, arm64_reg a, arm64_reg b, const char* ctx) {
    if (!gum_arm64_writer_put_push_reg_reg(&w, a, b))
        throw std::runtime_error(std::string(ctx) + ": put_push_reg_reg failed");
}

static void pop_pair(GumArm64Writer& w, arm64_reg a, arm64_reg b, const char* ctx) {
    if (!gum_arm64_writer_put_pop_reg_reg(&w, a, b))
        throw std::runtime_error(std::string(ctx) + ": put_pop_reg_reg failed");
}

std::vector<uint8_t> Arm64Stubs::save_ctx(uint64_t at) {
    std::vector<uint8_t> buf(256, 0);
    GumArm64Writer w;
    gum_arm64_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    push_pair(w, ARM64_REG_X0,  ARM64_REG_X1,  "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X2,  ARM64_REG_X3,  "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X4,  ARM64_REG_X5,  "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X6,  ARM64_REG_X7,  "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X8,  ARM64_REG_X9,  "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X10, ARM64_REG_X11, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X12, ARM64_REG_X13, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X14, ARM64_REG_X15, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X16, ARM64_REG_X17, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X18, ARM64_REG_X19, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X20, ARM64_REG_X21, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X22, ARM64_REG_X23, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X24, ARM64_REG_X25, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X26, ARM64_REG_X27, "Arm64Stubs::save_ctx");
    push_pair(w, ARM64_REG_X28, ARM64_REG_LR,  "Arm64Stubs::save_ctx");

    return arm64_collect(w, buf, "Arm64Stubs::save_ctx");
}

std::vector<uint8_t> Arm64Stubs::restore_ctx(uint64_t at) {
    std::vector<uint8_t> buf(256, 0);
    GumArm64Writer w;
    gum_arm64_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(at);

    pop_pair(w, ARM64_REG_X28, ARM64_REG_LR,  "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X26, ARM64_REG_X27, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X24, ARM64_REG_X25, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X22, ARM64_REG_X23, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X20, ARM64_REG_X21, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X18, ARM64_REG_X19, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X16, ARM64_REG_X17, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X14, ARM64_REG_X15, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X12, ARM64_REG_X13, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X10, ARM64_REG_X11, "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X8,  ARM64_REG_X9,  "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X6,  ARM64_REG_X7,  "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X4,  ARM64_REG_X5,  "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X2,  ARM64_REG_X3,  "Arm64Stubs::restore_ctx");
    pop_pair(w, ARM64_REG_X0,  ARM64_REG_X1,  "Arm64Stubs::restore_ctx");

    return arm64_collect(w, buf, "Arm64Stubs::restore_ctx");
}

std::vector<uint8_t> Arm64Stubs::branch(uint64_t from, uint64_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumArm64Writer w;
    gum_arm64_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);
    gum_arm64_writer_put_b_imm(&w, static_cast<GumAddress>(to));
    return arm64_collect(w, buf, "Arm64Stubs::branch");
}

std::vector<uint8_t> Arm64Stubs::call(uint64_t from, uint64_t to) {
    std::vector<uint8_t> buf(16, 0);
    GumArm64Writer w;
    gum_arm64_writer_init(&w, buf.data());
    w.pc = static_cast<GumAddress>(from);
    gum_arm64_writer_put_bl_imm(&w, static_cast<GumAddress>(to));
    return arm64_collect(w, buf, "Arm64Stubs::call");
}
