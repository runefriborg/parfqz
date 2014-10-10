#include <stdio.h>

// Input buffer of 20MiB.
static int INPUT_BUFFER_SIZE = 20971520;

typedef struct {
	FILE *fd;

	// The single worker process we have.
	pthread_t       worker;

	// Controls access to buffer.
	pthread_mutex_t mutex;

	// For waiting when buffer full/empty.
	pthread_cond_t  condc, condp;

	// Stores what is read from input.
	char            *input;

	// Size of allocated input.
	int             input_size;

	// This is where data read from fd begins. There is data from here until
	// input_length.
	int             input_pointer;

	// Designates where input no longer contains valid data, in case of short
	// read.
	int             input_length;

} splitstream_t;


struct chunk {
	char chunk_id [COMPRESSION_CHUNK_SIZE];
	char *chunk_id_content;

	char *chunk_base;

	char chunk_plus [COMPRESSION_CHUNK_SIZE];
	char *chunk_plus_content;

	char *chunk_qual;

	int read_len;

	// How many chunks there are in this chunk.
	// COMPRESSION_CHUNK_SIZE or less.
	int chunks;
};

typedef struct chunk chunk_t;

int begin_chunk_parsing(char *restrict filename, splitstream_t *t);
