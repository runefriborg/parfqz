#ifndef _HEADER_CHUNK_H_
#define _HEADER_CHUNK_H_
#include <stdint.h>

typedef struct {
  // Chunk maintenaince variables
  int order; // Used to handle out-of-order execution
  int read_len;
  int read_count; // number of reads in chunk
  
  // splitstream data  
  int read_id_offset [COMPRESSION_CHUNK_SIZE];
  char *read_id_content;                         // read_id_content+read_id_offset[offset]
  char *read_base;                               // read_base+read_len*offset
  int read_plus_offset [COMPRESSION_CHUNK_SIZE];
  char *read_plus_content;                       // read_plus_content+read_plus_offset[offset]
  char *read_qual;                               // read_qual+read_len*offset
  
  // splitchunk data
  char ** base_len_10;
  int base_len_10_count;
  char ** qual_len_10;
  int qual_len_10_count;

  char ** base_len_20;
  int base_len_20_count;
  char ** qual_len_20;
  int qual_len_20_count;
  //char *base_len_20;
  //char *base_len_skew; // handle the possibly 0 to 9 bases in the end.
  //int base_len_skew_len;

  // sortsegments data
  uint16_t *base_len_10_permute;
  uint16_t *qual_len_10_permute;
  uint16_t *base_len_20_permute;
  uint16_t *qual_len_20_permute;

  // transpose data
  char *base_len_10_transposed;
  char *qual_len_10_transposed;
  char *base_len_20_transposed;
  char *qual_len_20_transposed;

} chunk_t;


#endif
