/*
 * mpu.c - MPU region setup for LM3S6965EVB (Cortex-M3)
 *
 * Allowed regions:
 *   0  Flash       0x00000000  256 KB   R/X
 *   1  SRAM        0x20000000   64 KB   R/W  XN
 *   2  Peripherals 0x40000000  512 MB   R/W  XN  device
 *
 * Everything else triggers MemManage_Handler.
 */

#define SCB_SHCSR (*(volatile unsigned int *)0xE000ED24U)
#define MPU_TYPE  (*(volatile unsigned int *)0xE000ED90U)
#define MPU_CTRL  (*(volatile unsigned int *)0xE000ED94U)
#define MPU_RNR   (*(volatile unsigned int *)0xE000ED98U)
#define MPU_RBAR  (*(volatile unsigned int *)0xE000ED9CU)
#define MPU_RASR  (*(volatile unsigned int *)0xE000EDA0U)

/* RASR field helpers */
#define RASR_ENABLE       (1u << 0)
#define RASR_SIZE(s)      ((unsigned int)(s) << 1)  /* size = 2^(s+1) bytes */
#define RASR_AP(ap)       ((unsigned int)(ap) << 24)
#define RASR_XN           (1u << 28)
#define RASR_B            (1u << 16)                /* bufferable (device) */

/* Access permission encodings */
#define AP_RO   0x6u   /* privileged + unprivileged read-only  */
#define AP_RW   0x3u   /* privileged + unprivileged full access */

static const struct {
    unsigned int base;
    unsigned int rasr;
} regions[] = {
    /* Flash 256 KB (2^18 → SIZE=17): read + execute */
    { 0x00000000u, RASR_AP(AP_RO) | RASR_SIZE(17) | RASR_ENABLE },

    /* SRAM 64 KB (2^16 → SIZE=15): read/write, no execute */
    { 0x20000000u, RASR_XN | RASR_AP(AP_RW) | RASR_SIZE(15) | RASR_ENABLE },

    /* Peripherals 512 MB (2^29 → SIZE=28): read/write, no execute, device */
    { 0x40000000u, RASR_XN | RASR_AP(AP_RW) | RASR_B | RASR_SIZE(28) | RASR_ENABLE },
};

void mpu_init(void)
{
    unsigned int i;

    /* Require MPU to be present */
    if ((MPU_TYPE & 0xFF00u) == 0)
        return;

    for (i = 0; i < sizeof(regions) / sizeof(regions[0]); i++) {
        MPU_RNR  = i;
        MPU_RBAR = regions[i].base;
        MPU_RASR = regions[i].rasr;
    }

    /* Enable MemManage fault — without this it escalates to HardFault */
    SCB_SHCSR |= (1u << 16);
    /* Enable MPU; no privileged default background region */
    MPU_CTRL = 1u;
}
