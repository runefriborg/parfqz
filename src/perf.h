#ifndef _HEADER_PERF_H_
#define _HEADER_PERF_H_
#include <time.h>

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



perf_entry_t * perf_create(char * name, int id, int parent_id);
void perf_update_start(perf_entry_t * p);
void perf_update_tick(perf_entry_t * p); // may be called more than once
void perf_output_progress(perf_entry_t * p); // output local progress
//perf_submit(int id); // locking, call this to include perf in final report
//perf_output_report(int id, byte verbosity) // locking, output report with id as root.
//perf_free(perf_entry * p);


#endif
