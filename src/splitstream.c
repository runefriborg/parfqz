

#include "main.h"

struct chunk {
  char * chunk_id [COMPRESSION_CHUNK_SIZE];
  char * chunk_id_content;
  char * chunk_base;
  char * chunk_plus [COMPRESSION_CHUNK_SIZE];
  char * chunk_plus_content;
  char * chunk_qual;
  int read_len;
};




struct chunk * splitstream(char * filename) {
  

  return;
}

 
