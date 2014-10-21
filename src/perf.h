#ifndef _HEADER_PERF_H_
#define _HEADER_PERF_H_
#include <time.h>
#include <pthread.h>
#include <stdint.h>

#define PERF_MAX_ID 99

typedef struct {
  int id;
  int parent_id;
  char name [256];
  double last_update_time;
  double sum;
  double max;
  double min;
  int count;
} perf_entry_t;

typedef struct {
  pthread_mutex_t mutex;
  perf_entry_t * C [PERF_MAX_ID];
} perf_global_t;

extern perf_global_t report;


perf_entry_t * perf_create(char * name, int id, int parent_id);
void perf_update_start(perf_entry_t * p);
void perf_update_tick(perf_entry_t * p); // may be called more than once
void perf_output_progress(perf_entry_t * p); // output local progress
void perf_submit(perf_entry_t * p); // locking, call this to include perf in final report


void perf_global_init();
void perf_output_report(int id, uint8_t verbosity); // locking, output report with id as root.
void perf_global_free();

#endif
