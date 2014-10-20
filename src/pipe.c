#include <string.h> /* memset */
#include "pipe.h"

void pipe_init(pipe_t *pipe)
{
    memset(pipe->buffer, 0, OUTPUT_BUFFER_SIZE*sizeof(void*));
    pipe->begin = 0;
    pipe->end = 0;
	pthread_cond_init(&pipe->cond_put, NULL);
	pthread_cond_init(&pipe->cond_took, NULL);
	pthread_mutex_init(&pipe->mutex, NULL);
}

void pipe_put(pipe_t *pipe, void *data)
{
    // Lock the buffer
    pthread_mutex_lock(&pipe->mutex);

    // Wait for buffer to have space for chunk.
    while ((pipe->end + 1) % OUTPUT_BUFFER_SIZE == pipe->begin) {
        pthread_cond_wait(&pipe->cond_took, &pipe->mutex);
    }

    // Add chunk into buffered chunk list.
    pipe->buffer[pipe->end] = data;
    pipe->end = (pipe->end + 1) % OUTPUT_BUFFER_SIZE;

    // Signal to optionally waiting reader that something was added to
    //   queue.
    pthread_cond_signal(&pipe->cond_put);	
    pthread_mutex_unlock(&pipe->mutex);
}

void *pipe_get(pipe_t *pipe)
{
	pthread_mutex_lock(&pipe->mutex);
	while (pipe->begin == pipe->end) {
		pthread_cond_wait(&pipe->cond_put, &pipe->mutex);
	}
	void *data = pipe->buffer[pipe->begin];
    if (data != NULL)
        pipe->begin = (pipe->begin + 1) % OUTPUT_BUFFER_SIZE;
    pthread_cond_signal(&pipe->cond_took);	
	pthread_mutex_unlock(&pipe->mutex);
    return data;
}

#ifdef UNIT_TEST
#include <stdio.h>
#include <stdlib.h>

void *_async_putter(void *arg) {
    pipe_t *pipe = arg;
    for (int i = 0; i < 4096; i++) {
        pipe_put(pipe, "AA");
    }
    pipe_put(pipe, NULL);
    return NULL;
}
void *_async_getter(void *arg) {
    pipe_t *pipe = arg;
    void *data;
    while ((data = pipe_get(pipe)) != NULL) {
        printf("%s\n", (char*)data);
    }
    return NULL;
}

int main()
{
    pthread_t putter;
    pthread_t getter;
    pipe_t pipe;
    pipe_init(&pipe);
    void *t = &pipe;
    pthread_create(&putter, NULL, _async_putter, t);
    pthread_create(&getter, NULL, _async_getter, t);
    pthread_join(putter, NULL);
    pthread_join(getter, NULL);
    return 0;
}
#endif
