#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include "main.h"
#include "debug.h"
#include "pipe.h"

#include <sys/stat.h>

void writefile(char * data, int len, char * filename) {
  FILE *fp;
  fp = fopen(filename, "w");
  if (fp == NULL) {
    fprintf(stderr, "Can't open output file %s!\n",
	    filename);
    exit(1);
  }
  fwrite(data, len, sizeof(char), fp);
  fclose(fp);
}

void _debug(debug_t *t, chunk_t *c) {
  mkdir("debug", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  // Write debug files
  char name[255];
  sprintf(name, "debug/%06d_read_base", c->order);
  writefile(c->read_base, c->read_len*c->read_count, name);

}

void *_debug_async_worker(void *restrict arg) {
  debug_t *t = (debug_t *)arg;
  chunk_t *c = NULL;

  for (c = sortsegments_next_chunk(t->in); c != NULL; c = sortsegments_next_chunk(t->in)) {
    _debug(t, c);
    pipe_put(&(t->pipe), c);
  }
  pipe_put(&(t->pipe), NULL);
  return NULL;
}


debug_t * debug_open(sortsegments_t * pipe_in) {
  debug_t *t = malloc(sizeof(debug_t));
  
  // Register input line
  t->in = pipe_in;

  // Init output line
  pipe_init(&t->pipe);
  
  // Start worker
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&t->worker, &attr, _debug_async_worker, t);

  return t;
}


chunk_t * debug_next_chunk(debug_t *t) {
  return pipe_get(&t->pipe);
}


void debug_free_chunk(chunk_t * c) {
  sortsegments_free_chunk(c);
}

void debug_close(debug_t *t) {
  free(t);
}

