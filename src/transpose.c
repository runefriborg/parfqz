#include <stdlib.h>

#include <pthread.h>

#include "main.h"
#include "transpose.h"
#include "pipe.h"
#include "permute.h"

static char *_do_transpose_stuff(char **A, uint16_t *P, int N, int elem_size)
{
  /* Reorder A to match the order given in P */
  char **Ao = malloc(N*sizeof(char*));
  for (int i = 0; i < N; i++)
    Ao[i] = A[(P[i] + i) % N];
  /* Write result */
  char *r = malloc(N*elem_size+1);
  r[N*elem_size] = '\0';
  for (int col = 0; col < elem_size; col++)
  {
    for (int e = 0; e < N; e++)
      r[col*N + e] = Ao[e][col];
  }
  free(Ao);
  return r;
}

void *_transpose_async_worker(void *restrict arg) {
  transpose_t *t = (transpose_t *)arg;
  chunk_t *c = NULL;

  for (c = sortsegments_next_chunk(t->in); c != NULL; c = sortsegments_next_chunk(t->in)) {
    c->base_len_10_transposed = _do_transpose_stuff(
        c->base_len_10,
        c->base_len_10_permute,
        c->base_len_10_count,
        10);
    c->qual_len_10_transposed = _do_transpose_stuff(
        c->qual_len_10,
        c->qual_len_10_permute,
        c->qual_len_10_count,
        10);
    pipe_put(&(t->pipe), c);
  }
  pipe_put(&(t->pipe), NULL);
  return NULL;
}

transpose_t * transpose_open(sortsegments_t * pipe_in) {
  transpose_t *t = calloc(1, sizeof(transpose_t));
  
  // Register input line
  t->in = pipe_in;

  // Init output line
  pipe_init(&t->pipe);
  
  // Start worker
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&t->worker, &attr, _transpose_async_worker, t);

  return t;
}

chunk_t * transpose_next_chunk(transpose_t *t) {
  return pipe_get(&t->pipe);
}

void transpose_free_chunk(chunk_t * c) {
  free(c->base_len_10_transposed);
  free(c->qual_len_10_transposed);
  c->base_len_10_transposed = NULL;
  c->qual_len_10_transposed = NULL;
  sortsegments_free_chunk(c);
}

void transpose_close(transpose_t *t) {
  free(t);
}
