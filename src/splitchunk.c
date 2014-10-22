#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "main.h"
#include "splitchunk.h"
#include "pipe.h"


void _split(splitchunk_t *t, chunk_t *c) {

}

void *_splitchunk_async_worker(void *restrict arg) {
  splitchunk_t *t = (splitchunk_t *)arg;
  chunk_t *c = NULL;

  for (c = splitstream_next_chunk(t->in); c != NULL; c = splitstream_next_chunk(t->in)) {
    _split(t, c);
    pipe_put(&(t->pipe), c);
  }
  pipe_put(&(t->pipe), NULL);
  return NULL;
}


splitchunk_t * splitchunk_open(splitstream_t * pipe_in) {
  splitchunk_t *t = malloc(sizeof(splitchunk_t));
  
  // Register input line
  t->in = pipe_in;

  // Init output line
  pipe_init(&t->pipe);
  
  // Start worker
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&t->worker, &attr, _splitchunk_async_worker, t);

  return t;
}


chunk_t * splitchunk_next_chunk(splitchunk_t *t) {
  return pipe_get(&t->pipe);
}


void splitchunk_free_chunk(chunk_t * c) {
  splitstream_free_chunk(c);
}

void splitchunk_close(splitchunk_t *t) {
  free(t);
}

