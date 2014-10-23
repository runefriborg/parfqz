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
#include "splitstream.h"
#include "pipe.h"

void *xrealloc(void *ptr, size_t s) {
  void *m = realloc(ptr, s);
  
  if (m == NULL) {
    fprintf(stderr, "Unable to reallocate memory: %s\n", strerror(errno));
    exit(1);
  }
  
  return m;
}

void *xmalloc(size_t s) {
  void *m = malloc(s);
  
  if (m == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n", strerror(errno));
    exit(1);
  }

  return m;
}

static int get_next_line(splitstream_t *t, char **start, int *len)
{
  static char *s = NULL;
  static size_t s_size = 2000;
  if (s == NULL)
    s = calloc(s_size, 1);
  ssize_t read = getline(&s, &s_size, t->fd);
  if (read == -1)
    return 1;
  *start = s;
  *len = read - 1;
  return 0;
}

chunk_t *_read_chunk(splitstream_t *t, int *done) {
  if (*done)
    return NULL;
  chunk_t *c = calloc(1, sizeof(chunk_t));
  c->read_len = 0;
  int i;
  int state = 0;                     // Which line in an input we are at.
  int state_offset[4] = {0};         // Offset in alloc'd memory.
  // Allocate 2MiB for the variable input lines.
  // Lines 2 and 4 have static length, and are allocated in the switch below.
  int id_alloc = 2*1024*1024;        // Alloc size.
  int plus_alloc = 2*1024*1024;      // --||--
  int line_length = 0;
  *done = 0;
  
  c->read_id_content = xmalloc(id_alloc);
  c->read_plus_content = xmalloc(plus_alloc);
  
  for (i = 0; i < COMPRESSION_CHUNK_SIZE*4; i++) {
    char *line;
    int is_done = get_next_line(t, &line, &line_length);
    if (is_done) {
      *done = 1;
      break;
    }
    
    // Case 0-3 corresponds to line 1-4.
    switch(state) {
    case 0:
      if(id_alloc <= state_offset[0] + line_length + 1) {
	c->read_id_content = xrealloc(c->read_id_content, 2*id_alloc);
	id_alloc *= 2;
      }
      
      memcpy(&c->read_id_content[state_offset[0]], line, line_length);
      c->read_id_content[state_offset[0] + line_length] = '\0';
      c->read_id_offset[i/4] = state_offset[0];
      state_offset[0] += line_length + 1;
      break;
    case 1:
      if (c->read_len == 0) {
	c->read_len = line_length;
	c->read_base = xmalloc(line_length * COMPRESSION_CHUNK_SIZE);
	c->read_qual = xmalloc(line_length * COMPRESSION_CHUNK_SIZE);
      }
      
      memcpy(&c->read_base[state_offset[1]], line, line_length);
      state_offset[1] += line_length;
      break;
    case 2:
      if(plus_alloc <= state_offset[2] + line_length + 1) {
	c->read_plus_content = xrealloc(c->read_plus_content, 2*plus_alloc);
	plus_alloc *= 2;
      }
      
      memcpy(&c->read_plus_content[state_offset[2]], line, line_length + 1);
      c->read_plus_offset[(i-2)/4] = state_offset[2];
      c->read_plus_content[state_offset[2] + line_length] = '\0';
      state_offset[2] += line_length + 1;
      break;
    case 3:
      memcpy(&c->read_qual[state_offset[3]], line, line_length);
      state_offset[3] += line_length;
      break;
    }
    
    // Advance past the newline.
    state = (state + 1) % 4;
  }
  
  if (i == 0) {
    splitstream_free_chunk(c);
    return NULL;
  }
  
  c->read_count = i/4;
  
  return c;
}

void *_chunk_async_worker(void *restrict arg) {
  splitstream_t *t = (splitstream_t *)arg;
  chunk_t *c = NULL;
  int done = 0;
  int order = 0;

  while (done == 0) {
    // Read into chunk type.
    c = _read_chunk(t, &done);
    c->order = order++;
    pipe_put(&t->pipe, c);
  }
  pipe_put(&t->pipe, NULL);
  
  return NULL;
}

// begin_chunk_parsing takes a filename and a struct and returns 1 if input
// was opened correctly and has filled the values needed for the struct.
// In return value is 0, the struct cannot be used.
int begin_chunk_parsing(char *restrict filename, splitstream_t *t) {
  if(strncmp(filename, "-", 2) == 0) {
    // Read from stdin.
    t->fd = stdin;
  } else {
    // Open the file for input.
    t->fd = fopen(filename, "r");
    if (t->fd == NULL) {
      return 0;
    }
  }
  pipe_init(&t->pipe);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&t->worker, &attr, _chunk_async_worker, t);
  
  return 1;
}

splitstream_t * splitstream_open(char * filename) {
  splitstream_t *stream = calloc(1, sizeof(splitstream_t));
  int ok = begin_chunk_parsing(filename, stream);
  if (ok == 0) {
    fprintf(stderr, "Opening '%s' failed\n", filename);
    exit(1);
  }
  return stream;
}

void splitstream_close(splitstream_t *t) {
  if (t->fd != NULL)
    {
      fclose(t->fd);
      t->fd = NULL;
    }
  free(t);
}

chunk_t * splitstream_next_chunk(splitstream_t *t) {
  return pipe_get(&t->pipe);
}

void splitstream_free_chunk(chunk_t * c) {
  free(c->read_base);
  free(c->read_qual);
  free(c->read_id_content);
  free(c->read_plus_content);
  free(c);
}
