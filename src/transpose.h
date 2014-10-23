#ifndef _HEADER_TRANSPOSE_H_
#define _HEADER_TRANSPOSE_H_

#include <pthread.h>

#include "main.h"
#include "pipe.h"
#include "sortsegments.h"


typedef struct {
  // Input
  sortsegments_t * in;
  
  // The single worker process we have.
  pthread_t       worker;

  // Output
  pipe_t          pipe;
} transpose_t;


transpose_t * transpose_open(sortsegments_t * pipe_in);
chunk_t * transpose_next_chunk(transpose_t * p);
void transpose_free_chunk(chunk_t * c);
void transpose_close(transpose_t * p);

#endif
