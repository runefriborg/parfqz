#include <stdio.h>
#include <stdlib.h>

#include "compress.h"

compress_t *compress_open() {
	int r;
	compress_t *this = malloc(sizeof(compress_t));
	if (this == NULL) {
		fprintf(stderr, "Error allocating compress_t\n");
		exit(1);
	}

	this->zstream = malloc(sizeof(z_stream));
	if (this->zstream == NULL) {
		fprintf(stderr, "Error allocating z_stream\n");
		exit(1);
	}

	// Let zlib use the default allocator/freer.
	this->zstream->zalloc = Z_NULL;
	this->zstream->zfree  = Z_NULL;

	this->buffersize = 1024*1024; // 1MiB
	this->buffer = malloc(this->buffersize);
	if (this->buffer == NULL) {
		fprintf(stderr, "Error allocating output buffer\n");
		exit(1);
	}

	r = deflateInit(this->zstream, Z_DEFAULT_COMPRESSION);
	if (r != Z_OK) {
		fprintf(stderr, "Error initializing deflate\n");
		exit(1);
	}
		
	return this;
}

void __do_compress(compress_t *this, FILE *out,
		unsigned char *input, int input_size) {
	int r;
	unsigned int prev_total_out = 0, comp_count;

	this->zstream->next_in  = input;
	this->zstream->avail_in = input_size;

	this->zstream->next_out  = this->buffer;
	this->zstream->avail_out = this->buffersize;

	for (;;) {
		r = deflate(this->zstream, Z_FINISH);

		// Bytes compressed in last round.
		comp_count = this->zstream->total_out - prev_total_out;

		if (comp_count != 0) {
			if (fwrite(this->buffer, 1, comp_count, out) != comp_count 
					|| ferror(out)) {
				fprintf(stderr, "Error outputting compressed stream\n");
			}
		}

		if (r == Z_STREAM_END) {
			break;
		} else if (r == Z_OK) {
			this->zstream->next_out  = this->buffer;
			this->zstream->avail_out = this->buffersize;
			prev_total_out = this->zstream->total_out;
		}  else {
			fprintf(stderr, "Bad Z_STREAM state\n");
			exit(1);
		}
	}

}

void compress_next_chunk(compress_t *this, chunk_t *chunk, FILE *out) {
	__do_compress(this, out, (unsigned char*)chunk->read_base,
			chunk->read_len * chunk->read_count);

	__do_compress(this, out, (unsigned char*)chunk->read_qual,
			chunk->read_len * chunk->read_count);

	return;
}

void compress_close(compress_t *this) {
	int r;
	r = deflateEnd(this->zstream);

	switch (r) {
		case Z_OK:
			break;
		case Z_STREAM_ERROR:
			fprintf(stderr, "Inconsistent stream state: ");
			if (this->zstream->msg != NULL) {
				fprintf(stderr, "'%s'\n", this->zstream->msg);
			} else {
				fprintf(stderr, "zstream gave no error message\n");
			}
			break;
		case Z_DATA_ERROR:
			fprintf(stderr, "zstream was freed prematurely,"
					"before all data was processed\n");
			break;
		default:
			fprintf(stderr, "Unknown error freeing zstream\n");
			if (this->zstream->msg != NULL) {
				fprintf(stderr, "Error was: '%s'\n", this->zstream->msg);
			}
			break;
	}

	free(this->zstream);
	free(this->buffer);
	free(this);

	return;
}
