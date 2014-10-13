#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

static void _internal_simple_quick_sort(uint16_t *a, int n, const char **b, int bs)
{
    if (n < 2)
        return;
    uint16_t mid = a[n / 2];
    uint16_t *lo = a;
    uint16_t *hi = a + n - 1;
    while (lo <= hi) {
        if (strncmp(b[*lo], b[mid], bs) < 0) {
            lo += 1;
        }
        else if (strncmp(b[*hi], b[mid], bs) > 0) {
            hi -= 1;
        }
        else {
            uint16_t tmp = *lo;
            *lo = *hi;
            *hi = tmp;
            lo += 1;
            hi -= 1;
        }
    }
    _internal_simple_quick_sort(a,  hi - a + 1, b, bs);
    _internal_simple_quick_sort(lo, a + n - lo, b, bs);
}

/*
 * Assumes each block has exactly `elem_size` bytes of valid data.
 * Assumes 0 <= nblocks <= UINT16_MAX
 * The result is malloc'ed and must be freed by the caller.
 */
uint16_t *make_permutation_table(const char **blocks, int nblocks, int elem_size)
{
    assert(nblocks <= (1 << 16));
    int i;
    uint16_t *perm = malloc(nblocks*sizeof(uint16_t));

    for (i = 0; i < nblocks; i++)
        perm[i] = i;

    _internal_simple_quick_sort(perm, nblocks, blocks, elem_size);

    for (i = 0; i < nblocks; i++)
        perm[i] = (nblocks - i + perm[i]) % nblocks;

    return perm;
}

/* 
 * Assumes that the blocks are less than 2^30 bytes long and \0 terminated.
 * Assumes 0 <= nblocks <= UINT16_MAX
 * The result is malloc'ed and must be freed by the caller.
 */
uint16_t *make_permutation_table_varlen(const char **blocks, int nblocks)
{
    assert(nblocks <= (1 << 16));
    int i;
    uint16_t *perm = malloc(nblocks*sizeof(uint16_t));

    for (i = 0; i < nblocks; i++)
        perm[i] = i;

    _internal_simple_quick_sort(perm, nblocks, blocks, 1 << 30);

    for (i = 0; i < nblocks; i++)
        perm[i] = (nblocks - i + perm[i]) % nblocks;

    return perm;
}


#ifdef UNIT_TEST
#include <stdio.h>
int main()
{
    int N = (1 << 16);
    {
        const char *test_blocks[] = {"DD", "BB", "CC", "AA"};
        uint16_t *perm = make_permutation_table(test_blocks, 4, 2);
        int i = 0;
        for (i = 0; i < 4; i++)
            printf("%3s ", test_blocks[(perm[i] + i) % 4]);
        printf("\n");
    }
    {
        const char **test_blocks = malloc(N*sizeof(char*));
        int i;
        int nblocks = N;
        for (i = 0; i < nblocks; i++)
            test_blocks[i] = "BB";
        test_blocks[0] = "CC";
        test_blocks[nblocks-1] = "AA";
        uint16_t *perm = make_permutation_table(test_blocks, nblocks, 2);
        const char *last = test_blocks[perm[0] % nblocks];
        for (i = 1; i < nblocks; i++) {
            const char *current = test_blocks[(perm[i] + i) % nblocks];
            if (strncmp(last, current, 2) > 0)
            {
                printf("** ERROR **  not a proper sorting");
                return 1;
            }
            last = current;
        }
        printf("%s\n", test_blocks[(perm[0] + 0) % nblocks]);
        printf("%s\n", test_blocks[(perm[1] + 1) % nblocks]);
        printf("%s\n", test_blocks[(perm[nblocks-1] + (nblocks-1)) % nblocks]);
        free(perm);
        free(test_blocks);
    }
    return 0;
}
#endif
