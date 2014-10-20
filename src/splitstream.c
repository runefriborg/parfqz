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

inline int find_newline(char *s, int l) {
	int i = 0;
	while (i < l && s[i++] != '\n');
	return i;
}

chunk_t *_read_chunk(splitstream_t *t) {
	chunk_t *c = xmalloc(sizeof(chunk_t));
	int r
		, i
		, state = 0       // Which line in an input we are at.
		, state_offset[4] // Offset in alloc'd memory.
		, id_alloc        // Alloc size.
		, plus_alloc      // --||--
		, line_length = 0;

	state_offset[0] = state_offset[1] = state_offset[2] = state_offset[3] = 0;

	// Allocate 2MiB for the variable input lines.
	// Lines 2 and 4 have static length, and are allocated in the switch below.
	id_alloc = 2097152;
	plus_alloc = 2097152;

	c->chunk_id_content = xmalloc(id_alloc);
	c->chunk_plus_content = xmalloc(plus_alloc);


	for (i = 0; i < COMPRESSION_CHUNK_SIZE*4; i++) {
		line_length = 0;

		// Find a complete line in the string.
		while (1) {
			/*
			 * Invariants: the fastq format consists of four lines per entry.
			 * When we are called we know that the we are starting with a
			 * completely new enty, or with no entries left in the input
			 * stream.
			 *
			 * Either t->input_pointer == t->input_end and no data is currently
			 * in the buffer, or t->input_pointer < t->input_end, and this is
			 * the data we have to begin with.
			 */
			if (line_length + t->input_pointer >= t->input_length) {
				// Read new data.

				if (t->input_pointer < t->input_length) {
					// Move existing data into beginning of input.
					memcpy(t->input, &t->input[t->input_pointer], t->input_length - t->input_pointer);

					// Store the amount of data in the end of input.
					t->input_pointer = t->input_length - t->input_pointer;
				}

				// Read new data into the buffer.
				r = fread(&t->input[t->input_pointer], 1, t->input_size - t->input_pointer, t->fd);
				t->input_pointer = 0;
				t->input_length = r;

				if (r == 0) { // EOF or input error.
					// We skip the newline, so it isn't a problem here.
					if (line_length != 0) {
						// This means the file didn't end with a newline.
						// Hope that this is the case.
						break;
					} else {
						goto STOP;
					}
				}
			}
			
			if (t->input[line_length] == '\n') {
				break;
			}

			line_length++;
		}

		// Case 0-3 corresponds to line 1-4.
		switch(state) {
		case 0:
			if(id_alloc <= state_offset[0] + line_length) {
				c->chunk_id_content = xrealloc(c->chunk_id_content, 2*id_alloc);
				id_alloc *= 2;
			}

			memcpy(&c->chunk_id_content[state_offset[0]], &t->input[t->input_pointer], line_length);
			c->chunk_id[i/4] = line_length + 1; // line_length is 0 indexed.
			state_offset[0] += line_length;
			break;
		case 1:
			if (c->read_len == 0) {
				c->read_len = line_length;
				c->chunk_base = xmalloc(line_length * COMPRESSION_CHUNK_SIZE);
				c->chunk_qual = xmalloc(line_length * COMPRESSION_CHUNK_SIZE);
			}

			memcpy(&c->chunk_base[state_offset[2]], &t->input[t->input_pointer], line_length);
			state_offset[1] += line_length;
			break;
		case 2:
			if(plus_alloc <= state_offset[2] + line_length) {
				c->chunk_plus_content = xrealloc(c->chunk_plus_content, 2*plus_alloc);
				plus_alloc *= 2;
			}

			memcpy(&c->chunk_plus_content[state_offset[3]], &t->input[t->input_pointer], line_length);
			c->chunk_plus[(i-2)/4] = line_length + 1; // line_length is 0 indexed.
			state_offset[2] += line_length;
			break;
		case 3:
			memcpy(&c->chunk_qual[state_offset[2]], &t->input[t->input_pointer], line_length);
			state_offset[3] += line_length;
			break;
		}

		// Advance past the newline.
		t->input_pointer += line_length + 2;
		state = (state + 1) % 4;
	}

	// If we ran into EOF, shrink allocations.
	STOP: if (i != COMPRESSION_CHUNK_SIZE * 4) {
		c->chunk_id_content = xrealloc(c->chunk_id_content, state_offset[0]);
		c->chunk_plus_content = xrealloc(c->chunk_plus_content, state_offset[2]);
		c->chunk_qual = xrealloc(c->chunk_qual, state_offset[1]);
	}

	c->chunks = i/4;

	return c;
}

void *_chunk_async_worker(void *restrict arg) {
	splitstream_t *t = (splitstream_t *)arg;
	chunk_t *c = NULL;

	while (1 == 1) {
		// Read into chunk type.
		c = _read_chunk(t);
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

	t->input_length = t->input_pointer = 0;

	t->input_size = INPUT_BUFFER_SIZE;
	t->input      = xmalloc(INPUT_BUFFER_SIZE);

	pipe_init(&t->pipe);
	pthread_create(&t->worker, NULL, _chunk_async_worker, t);

	return 1;
}

splitstream_t * splitstream_open(char * filename)
{
	splitstream_t *stream = calloc(1, sizeof(splitstream_t));
	int ok = begin_chunk_parsing(filename, stream);
	if (ok == 0) {
		fprintf(stderr, "Opening '%s' failed\n", filename);
		exit(1);
	}
	return stream;
}
void splitstream_close(splitstream_t *t)
{
	(void)t;
}
chunk_t * splitstream_next_chunk(splitstream_t *t)
{
	return pipe_get(&t->pipe);
}
void splitstream_free_chunk(chunk_t * c)
{
	(void)c;
}
