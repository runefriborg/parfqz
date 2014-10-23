#ifndef _HEADER_SPLITSTREAM_H_
#define _HEADER_SPLITSTREAM_H_

#include <stdio.h>
#include <pthread.h>

#include "main.h"
#include "chunk.h"
#include "pipe.h"


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

#endif
