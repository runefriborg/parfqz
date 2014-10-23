#include <stdlib.h>

#include <pthread.h>

#include "main.h"
#include "sortsegments.h"
#include "pipe.h"
#include "permute.h"


void *_sortsegments_async_worker(void *restrict arg) {
  sortsegments_t *t = (sortsegments_t *)arg;
  chunk_t *c = NULL;

  for (c = splitchunk_next_chunk(t->in); c != NULL; c = splitchunk_next_chunk(t->in)) {
    c->base_len_10_permute = make_permutation_table(c->base_len_10, c->base_len_10_count, 10);
    c->qual_len_10_permute = make_permutation_table(c->qual_len_10, c->qual_len_10_count, 10);
    c->base_len_20_permute = make_permutation_table(c->base_len_20, c->base_len_20_count, 20);
    c->qual_len_20_permute = make_permutation_table(c->qual_len_20, c->qual_len_20_count, 20);
    pipe_put(&(t->pipe), c);
  }
  pipe_put(&(t->pipe), NULL);
  return NULL;
}

sortsegments_t * sortsegments_open(splitchunk_t * pipe_in) {
  sortsegments_t *t = calloc(1, sizeof(sortsegments_t));
  
  // Register input line
  t->in = pipe_in;

  // Init output line
  pipe_init(&t->pipe);
  
  // Start worker
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&t->worker, &attr, _sortsegments_async_worker, t);

  return t;
}

chunk_t * sortsegments_next_chunk(sortsegments_t *t) {
  return pipe_get(&t->pipe);
}

void sortsegments_free_chunk(chunk_t * c) {
  free(c->base_len_10_permute);
  free(c->qual_len_10_permute);
  c->base_len_10_permute = NULL;
  c->qual_len_10_permute = NULL;
  free(c->base_len_20_permute);
  free(c->qual_len_20_permute);
  c->base_len_20_permute = NULL;
  c->qual_len_20_permute = NULL;
  splitchunk_free_chunk(c);
}

void sortsegments_close(sortsegments_t *t) {
  free(t);
}
