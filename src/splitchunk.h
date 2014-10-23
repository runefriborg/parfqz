#ifndef _HEADER_SPLITCHUNK_H_
#define _HEADER_SPLITCHUNK_H_

#include <stdio.h>
#include <pthread.h>

#include "main.h"
#include "pipe.h"
#include "splitstream.h"


typedef struct {
  // Input
  splitstream_t * in;
  
  // The single worker process we have.
  pthread_t       worker;

  // Output
  pipe_t          pipe;
} splitchunk_t;


splitchunk_t * splitchunk_open(splitstream_t * pipe_in);
chunk_t * splitchunk_next_chunk(splitchunk_t * p);
void splitchunk_free_chunk(chunk_t * c);
void splitchunk_close(splitchunk_t * p);

#endif
