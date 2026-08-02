#ifndef AARCH64_HELPER_H
# define AARCH64_HELPER_H

#include <stdint.h>

typedef struct {
    uint64_t nzcv;
    uint64_t fp, lr;
    uint64_t x27, x28;
    uint64_t x25, x26;
    uint64_t x23, x24;
    uint64_t x21, x22;
    uint64_t x19, x20;
    uint64_t x17, x18;
    uint64_t x15, x16;
    uint64_t x13, x14;
    uint64_t x11, x12;
    uint64_t x9, x10;
    uint64_t x7, x8;
    uint64_t x5, x6;
    uint64_t x3, x4;
    uint64_t x1, x2;
    uint64_t x0;
    uint64_t ret;
    uint64_t orig_sp; /* sp at the hooked instruction, before the trampoline/shared stub pushed this context */
} greffe_ctx_t;

extern greffe_ctx_t g_greffe_ctx;

void greffe_capture_ctx(uint64_t *sp)
{
    g_greffe_ctx.nzcv = sp[0];
    g_greffe_ctx.fp   = sp[2];
    g_greffe_ctx.lr   = sp[3];
    g_greffe_ctx.x27  = sp[4];
    g_greffe_ctx.x28  = sp[5];
    g_greffe_ctx.x25  = sp[6];
    g_greffe_ctx.x26  = sp[7];
    g_greffe_ctx.x23  = sp[8];
    g_greffe_ctx.x24  = sp[9];
    g_greffe_ctx.x21  = sp[10];
    g_greffe_ctx.x22  = sp[11];
    g_greffe_ctx.x19  = sp[12];
    g_greffe_ctx.x20  = sp[13];
    g_greffe_ctx.x17  = sp[14];
    g_greffe_ctx.x18  = sp[15];
    g_greffe_ctx.x15  = sp[16];
    g_greffe_ctx.x16  = sp[17];
    g_greffe_ctx.x13  = sp[18];
    g_greffe_ctx.x14  = sp[19];
    g_greffe_ctx.x11  = sp[20];
    g_greffe_ctx.x12  = sp[21];
    g_greffe_ctx.x9   = sp[22];
    g_greffe_ctx.x10  = sp[23];
    g_greffe_ctx.x7   = sp[24];
    g_greffe_ctx.x8   = sp[25];
    g_greffe_ctx.x5   = sp[26];
    g_greffe_ctx.x6   = sp[27];
    g_greffe_ctx.x3   = sp[28];
    g_greffe_ctx.x4   = sp[29];
    g_greffe_ctx.x1   = sp[30];
    g_greffe_ctx.x2   = sp[31];
    g_greffe_ctx.x0   = sp[32];
    g_greffe_ctx.ret  = sp[33];
    g_greffe_ctx.orig_sp = (uint64_t)(uintptr_t)(sp + 34);
}

#endif
