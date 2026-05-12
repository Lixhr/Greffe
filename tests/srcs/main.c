#include <stdio.h>

__asm__(
    ".section .rwx, \"awx\", %progbits\n"
    ".global rwx_page\n"
    "rwx_page:\n"
    ".space 4096\n"
    ".previous\n"
);

extern char rwx_page[4096];

unsigned int checksum_check(const unsigned char *data, int len, unsigned int expected);

static const unsigned char test_data[] = "greffe-crc";

int main(void)
{
    const unsigned int expected = 0xa8a07e51UL;
    int ok = checksum_check(test_data, sizeof(test_data) - 1, expected);
    printf("CRC %s\n", ok ? "OK" : "KO");
    return !ok;
}
