#pragma once

#include <pthread.h>

#define OUTPUT_BUFFER_SIZE 1024

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond_put, cond_took;

    void *buffer[OUTPUT_BUFFER_SIZE];
    int begin, end;
} pipe_t;

void pipe_init(pipe_t *pipe);
void pipe_put(pipe_t *pipe, void *data);
void *pipe_get(pipe_t *pipe);
