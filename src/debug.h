#ifndef _HEADER_DEBUG_H_
#define _HEADER_DEBUG_H_

#include <stdio.h>
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
} debug_t;


debug_t * debug_open(sortsegments_t * pipe_in);
chunk_t * debug_next_chunk(debug_t * p);
void debug_free_chunk(chunk_t * c);
void debug_close(debug_t * p);

#endif
