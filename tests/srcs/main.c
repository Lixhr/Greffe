#include <stdio.h>
#include <endian.h>

__asm__(
    ".section .rwx, \"awx\", %progbits\n"
    ".global rwx_page\n"
    "rwx_page:\n"
    ".space 0x3000\n"
    ".previous\n"
);

extern char rwx_page[0x3000];

unsigned int checksum_check(const unsigned char *data, int len, unsigned int expected);

static const unsigned char test_data[] = "greffe-crc";

int main(void)
{
#if BYTE_ORDER == BIG_ENDIAN
    const unsigned int expected = 0xd4ce1060UL;
#else
    const unsigned int expected = 0xa8a07e51UL;
#endif
    int ok = checksum_check(test_data, sizeof(test_data) - 1, expected);
    printf("CRC %s\n", ok ? "OK" : "KO");
    return !ok;
}
