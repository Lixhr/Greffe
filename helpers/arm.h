#ifndef ARM_HELPER_H
# define ARM_HELPER_H

#include <stdint.h>

typedef struct {
    uint32_t cpsr;
    uint32_t r1, r2, r3, r4, r5, r6, r7;
    uint32_t r8, r9, r10, r11, r12;
    uint32_t lr;
    uint32_t r0;
    uint32_t ret;
    uint32_t orig_sp; /* sp at the hooked instruction, before the trampoline/shared stub pushed this context */
} greffe_ctx_t;

extern greffe_ctx_t g_greffe_ctx;

void greffe_capture_ctx(uint32_t *sp)
{
    g_greffe_ctx.cpsr = sp[0];
    g_greffe_ctx.r1   = sp[1];
    g_greffe_ctx.r2   = sp[2];
    g_greffe_ctx.r3   = sp[3];
    g_greffe_ctx.r4   = sp[4];
    g_greffe_ctx.r5   = sp[5];
    g_greffe_ctx.r6   = sp[6];
    g_greffe_ctx.r7   = sp[7];
    g_greffe_ctx.r8   = sp[8];
    g_greffe_ctx.r9   = sp[9];
    g_greffe_ctx.r10  = sp[10];
    g_greffe_ctx.r11  = sp[11];
    g_greffe_ctx.r12  = sp[12];
    g_greffe_ctx.lr   = sp[13];
    g_greffe_ctx.r0   = sp[14];
    g_greffe_ctx.ret  = sp[15];
    g_greffe_ctx.orig_sp = (uint32_t)(uintptr_t)(sp + 16);
}


#endif