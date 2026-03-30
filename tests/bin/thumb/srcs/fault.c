/*
 * fault.c - MemManage handler (Cortex-M "segfault")
 *
 * Fires when the MPU rejects an access.  Prints fault details to UART0
 * then halts.
 */

#define UART0_DR  (*(volatile unsigned int *)0x4000C000U)
#define UART0_FR  (*(volatile unsigned int *)0x4000C018U)
#define UART0_FR_TXFF (1u << 5)

/* Configurable Fault Status Register (byte 0 = MemManage status) */
#define SCB_CFSR  (*(volatile unsigned int *)0xE000ED28U)
/* HardFault Status Register */
#define SCB_HFSR  (*(volatile unsigned int *)0xE000ED2CU)
/* MemManage Fault Address Register */
#define SCB_MMFAR (*(volatile unsigned int *)0xE000ED34U)

/* CFSR MemManage bits */
#define MMFSR_IACCVIOL  (1u << 0)  /* instruction fetch from disallowed region */
#define MMFSR_DACCVIOL  (1u << 1)  /* data access to disallowed region         */
#define MMFSR_MMARVALID (1u << 7)  /* MMFAR contains the faulting address      */

static void putc_(char c)
{
    while (UART0_FR & UART0_FR_TXFF)
        ;
    UART0_DR = (unsigned int)c;
}

static void puts_(const char *s)
{
    while (*s) putc_(*s++);
}

static void puthex(unsigned int v)
{
    int i;
    puts_("0x");
    for (i = 28; i >= 0; i -= 4) {
        unsigned int n = (v >> i) & 0xFu;
        putc_(n < 10 ? '0' + n : 'a' + n - 10);
    }
}

void HardFault_Handler(void)
{
    puts_("[HardFault] HFSR=");
    puthex(SCB_HFSR);
    puts_("\r\n");
    while (1);
}

void MemManage_Handler(void)
{
    unsigned int cfsr = SCB_CFSR;

    puts_("[MemManage] CFSR=");
    puthex(cfsr);
    if (cfsr & MMFSR_MMARVALID) {
        puts_(" faulting_addr=");
        puthex(SCB_MMFAR);
    }
    if (cfsr & MMFSR_IACCVIOL) puts_(" [instruction access violation]");
    if (cfsr & MMFSR_DACCVIOL) puts_(" [data access violation]");
    puts_("\r\n");

    while (1)
        ;
}
