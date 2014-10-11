#define _GNU_SOURCE

#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

inline uint8_t comp(uint64_t a) {
	uint64_t b;
	uint8_t r,r2,r3,r4;

	// Check if we are 8 bit repeating
	b = a >> 8;
	b ^= a;
	r  = ((0x00000000000000FF & b) != 0) +
	     ((0x000000000000FF00 & b) != 0) +
		 ((0x0000000000FF0000 & b) != 0) +
		 ((0x00000000FF000000 & b) != 0) +
		 ((0x000000FF00000000 & b) != 0) +
		 ((0x0000FF0000000000 & b) != 0) +
		 ((0x00FF000000000000 & b) != 0);

	// Allow for 2 neighbour variations CCFGCCCC, CCFFFFCC or CCFCCCCC, but not CCFCCFCC
	r = (r > 2);
	
	// Check if we are 16 bit repeating ABABABAB
	b = a >> 16;
	b ^= a;
	r2 = ((0x000000000000FFFF & b) != 0) +
	     ((0x00000000FFFF0000 & b) != 0) +
	     ((0x0000FFFF00000000 & b) != 0);

	// Check if we are 24 bit repeating AABAABAA
	b = a >> 24;
	b ^= a;
	r3 = ((0x0000000000FFFFFF & b) != 0);
	
	b = a << 24;
	b ^= a;
	r3 += ((0xFFFFFF0000000000 & b) != 0);

	// Check if we are 32 bit repeating AAABAAAB
	b = a >> 32;
	b ^= a;

	r4 = (0x00000000FFFFFFFF & b) != 0;

	return !(!r || !r2 || !r3 || !r4);
}

int filesize(char *input) {
	struct stat s;
	int r = stat(input, &s);
	if (r == -1) {
		fprintf(stderr, "Unable to stat\n");
		exit(1);
	}
	return(s.st_size);
}

int main() {
	char *input = "testfile";
	unsigned int i, inputsize = filesize(input);

	unsigned int int_count = inputsize;
	int_count -= (int_count % sizeof(uint64_t));
	int_count /= sizeof(uint64_t);

	int ifd = open(input, O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr, "Error opening");
		exit(1);
	}

	uint64_t *mem;
	mem = mmap(NULL, inputsize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, ifd, 0);
	if (mem == MAP_FAILED) {
		fprintf(stderr, "Error mmapping");
		exit(1);
	}

	for(i = 0; i < int_count; i++) {
		printf("%hhX\n", comp(mem[i]));
		//comp(mem[i]);
	}


	//printf("%hhX\n", comp());

	return 0;
}
