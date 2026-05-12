// XTEA-based checksum: produces nested loops, PC-relative loads,
// shifted-register addressing, and inter-iteration data dependencies.

#include <stdint.h>

#define XTEA_ROUNDS 32
#define XTEA_DELTA  0x9E3779B9UL

static const uint32_t KEY[4] = {
    0xDEADBEEFUL,
    0xCAFEBABEUL,
    0x13371337UL,
    0xFEEDFACEUL,
};

__attribute__((noinline))
static uint32_t xtea_round(uint32_t val, uint32_t peer, uint32_t sum, int use_high)
{
    uint32_t mix = ((peer << 4) ^ (peer >> 5)) + peer;
    uint32_t idx = use_high ? ((sum >> 11) & 3) : (sum & 3);
    return val + (mix ^ (sum + KEY[idx]));
}

__attribute__((noinline))
static void xtea_block(uint32_t v[2])
{
    uint32_t v0  = v[0];
    uint32_t v1  = v[1];
    uint32_t sum = 0;
    int i;

    for (i = 0; i < XTEA_ROUNDS; i++) {
        v0   = xtea_round(v0, v1, sum, 0);
        sum += XTEA_DELTA;
        v1   = xtea_round(v1, v0, sum, 1);
    }

    v[0] = v0;
    v[1] = v1;
}

__attribute__((noinline))
uint32_t checksum(const uint8_t *data, int len)
{   
    uint32_t acc = 0;
    int      off = 0;

    while (off < len) {
        uint32_t v[2] = {0, 0};
        uint8_t *dst  = (uint8_t *)v;
        int      i;

        for (i = 0; i < 8 && off < len; i++, off++)
            dst[i] = data[off];

        xtea_block(v);
        acc ^= v[0] ^ v[1];
    }

    return acc;
}

__attribute__((noinline))
uint32_t checksum_check(const uint8_t *data, int len, uint32_t expected)
{
    return checksum(data, len) == expected ? 1 : 0;
}