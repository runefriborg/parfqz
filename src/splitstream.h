#include <stdio.h>
#include <pthread.h>

#include "main.h"
#include "pipe.h"

typedef struct {
	int chunk_id [COMPRESSION_CHUNK_SIZE];
	char *chunk_id_content;

	char *chunk_base;

	int chunk_plus [COMPRESSION_CHUNK_SIZE];
	char *chunk_plus_content;

	char *chunk_qual;

	int read_len;

	// How many chunks there are in this chunk.
	// COMPRESSION_CHUNK_SIZE or less.
	int chunks;
} chunk_t;

typedef struct {
	FILE *fd;

	// The single worker process we have.
	pthread_t       worker;

	// Our buffer
	pipe_t          pipe;
} splitstream_t;

int begin_chunk_parsing(char *restrict filename, splitstream_t *t);

// TODO
splitstream_t * splitstream_open(char * filename);
chunk_t * splitstream_next_chunk(splitstream_t * p);
void splitstream_free_chunk(chunk_t * c);
void splitstream_close(splitstream_t * p);
