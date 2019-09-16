#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    uint64_t u64 = 0x12345;

    printf("u64=0x%02X\n", u64);
    printf("u64=0x%02X\n", u64 & -16ULL);
    printf("u64=0x%02X\n", u64 & 0xFFFFFFFFFFFFFFF0ULL);

    uint64_t u64_1 = 0xFFFFFFFFFFFFFFF0ULL;
    uint64_t u64_2 = -16ULL;
    assert(u64_1 == u64_2);

    return 0;
}
