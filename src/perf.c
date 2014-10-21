/* Perf collection
 * 
 */ 
#include <pthread.h>
#include <stdlib.h> 
#include <string.h> 
#include <errno.h>
#include <stdio.h> 
#include <sys/time.h>

#include "perf.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// INIT GLOBAL
perf_global_t report;

perf_entry_t * perf_create(char * name, int id, int parent_id) {
  perf_entry_t *p = (perf_entry_t *) malloc(sizeof(perf_entry_t));

  if (p == NULL) {
    fprintf(stderr, "Unable to allocate memory: %s\n", strerror(errno));
    exit(1);
  }
  
  strncpy(p->name, name, 256);
  p->id = id;
  p->parent_id = parent_id;
  p->count = 0;
  p->sum = 0.0;
  p->max = 0.0;
  p->min = 10000000000;
  
  return p;
}


void perf_update_start(perf_entry_t * p) {
  struct timeval tv;
  gettimeofday(&tv, NULL); 
  p->last_update_time = tv.tv_sec + tv.tv_usec / 1000000.0; 
}

void perf_update_tick(perf_entry_t * p) {
  struct timeval tv;
  double t,d;
  gettimeofday(&tv, NULL); 
  t = tv.tv_sec + tv.tv_usec / 1000000.0; 

  d = t - p->last_update_time;

  // Prepare for possible next tick
  p->last_update_time = t;

  // Update values
  p->max = MAX(p->max, d);
  p->min = MIN(p->min, d);
  p->sum = p->sum + d;
  p->count++;
}

void perf_output_progress(perf_entry_t * p) {
  if (p->count > 0) {
    printf(" *** %s : count %d, sum %f, avg %f, min %f, max %f\n", p->name, p->count, p->sum, p->sum/p->count, p->min, p->max); 
  }
}

void perf_global_init() {
  int i = 0;
  for (i = 0; i < PERF_MAX_ID; i++) {
    report.C[i] = NULL;
  }
  pthread_mutex_init(&(report.mutex), NULL);
}

void perf_global_free() {
  int i = 0;
  for (i = 0; i < PERF_MAX_ID; i++) {
    if (report.C[i] != NULL) {
      free(report.C[i]);
    }
  }
}

void perf_submit(perf_entry_t * p) {
  pthread_mutex_lock(&(report.mutex));
  report.C[p->id] = p;
  pthread_mutex_unlock(&(report.mutex));  
}


void perf_output_report(int id, uint8_t verbosity) {
  // output
  int i;
  perf_entry_t * n = report.C[id];
  
  if (n != NULL) {
    perf_output_progress(n);
    
    for (i = 0; i < PERF_MAX_ID; i++) {
      if (report.C[i] != NULL && report.C[i]->parent_id == n->id) {
	// child
	perf_output_report(i, verbosity);
      }
    }
  }
}

// Testing
#include <unistd.h>

int main(int argc, char* argv[])
{
  perf_entry_t * main = perf_create("Main", 0, -1);
  perf_entry_t * fisk = perf_create("Fisk", 1, 0);
  
  perf_update_start(main);
  
  perf_global_init();

  int i;

  for (i=0; i<10; i++) {
    perf_update_start(fisk);
    sleep(1);
    perf_submit(fisk);
    perf_update_tick(fisk);
    perf_output_progress(fisk);
  }
  perf_update_tick(main);
  
  perf_output_report(0, 0);  

  perf_global_free();

 
  return 0; 
}
