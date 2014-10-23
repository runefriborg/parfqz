#ifndef _HEADER_SORTSEGMENTS_H_
#define _HEADER_SORTSEGMENTS_H_

#include <pthread.h>

#include "main.h"
#include "pipe.h"
#include "splitchunk.h"


typedef struct {
  // Input
  splitchunk_t * in;
  
  // The single worker process we have.
  pthread_t       worker;

  // Output
  pipe_t          pipe;
} sortsegments_t;


sortsegments_t * sortsegments_open(splitchunk_t * pipe_in);
chunk_t * sortsegments_next_chunk(sortsegments_t * p);
void sortsegments_free_chunk(chunk_t * c);
void sortsegments_close(sortsegments_t * p);

#endif
