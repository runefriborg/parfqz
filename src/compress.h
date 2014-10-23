#include <zlib.h>

#include "main.h"
#include "chunk.h"

typedef struct compress {
	z_stream      *zstream;
	int            buffersize;
	unsigned char *buffer;
} compress_t;

compress_t *compress_open();
void compress_close(compress_t *this);
