#ifndef _PERMUTE_H_
#define _PERMUTE_H_

#include <stdint.h>

/*
 * Assumes each block has exactly `elem_size` bytes of valid data.
 * Assumes 0 <= nblocks <= UINT16_MAX
 * The result is malloc'ed and must be freed by the caller.
 */
uint16_t *make_permutation_table(char **blocks, int nblocks, int elem_size); 

/* 
 * Assumes that the blocks are less than 2^30 bytes long and \0 terminated.
 * Assumes 0 <= nblocks <= UINT16_MAX
 * The result is malloc'ed and must be freed by the caller.
 */
uint16_t *make_permutation_table_varlen(char **blocks, int nblocks);

#endif
