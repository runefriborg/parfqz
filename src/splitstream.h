#include <stdio.h>
#include <pthread.h>

#include "main.h"
#include "pipe.h"


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
  
} chunk_t;


typedef struct {
  FILE *fd;
  
  // The single worker process we have.
  pthread_t       worker;
  
  // Our buffer
  pipe_t          pipe;
} splitstream_t;


int begin_chunk_parsing(char *restrict filename, splitstream_t *t);

splitstream_t * splitstream_open(char * filename);
chunk_t * splitstream_next_chunk(splitstream_t * p);
void splitstream_free_chunk(chunk_t * c);
void splitstream_close(splitstream_t * p);
