/* LM3S6965EVB (Cortex-M3) - UART0 base */
#define UART0_DR  (*(volatile unsigned int *)0x4000C000U)
#define UART0_FR  (*(volatile unsigned int *)0x4000C018U)
#define UART0_FR_TXFF (1u << 5)




static void uart_putc(char c)
{
    while (UART0_FR & UART0_FR_TXFF)
        ;
    UART0_DR = (unsigned int)c;
}

static void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

static void uart_puthex(unsigned char value)
{
    const char hex[] = "0123456789abcdef";
    uart_putc(hex[value >> 4]);
    uart_putc(hex[value & 0xf]);
}

void mpu_init(void);
unsigned int checksum_check(const unsigned char *data, int len, unsigned int expected);

static const unsigned char test_data[] = "greffe-crc";

int main(void)
{
    mpu_init();
    uart_puts("ARM Thumb test\r\n");

    const unsigned int expected = 0xa8a07e51UL;
    uart_puts(
            checksum_check(test_data, sizeof(test_data) - 1, expected)
            ? "CRC OK"
            : "CRC KO"
    );



    uart_puts("\r\n");


    while (1)
        ;
    return 0;
}
