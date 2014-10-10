/* Pthread implementation of monte carlo pi. 
 * 
 */ 
#include <pthread.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <sys/time.h>

typedef struct {
    int job_id; 
    double w_start; 
    double w_step; 
    int w; 
    double h_start; 
    double h_step; 
    int h; 
} mb_job; 

typedef struct {
    int job_id; 
    int *res; 
} mb_result; 


mb_job     *jobs; 
int         jobs_left; 
mb_result **results; 
int         n_workers;
 
pthread_mutex_t job_mutex = PTHREAD_MUTEX_INITIALIZER; 


/* Creates an array of jobs as well as the results array. */ 
void create_jobs(int jobcount, int w, int h)
{
    double xmin = -1.6744096758873175; 
    double xmax = -1.6744096714940624; 
    double ymin = 0.00004716419197284976; 
    double ymax = 0.000047167062611931696; 

    double w_step = (xmax-xmin)/w; 
    double h_step = (ymax-ymin)/h; 
    double w_start = xmin; 
    double h_start = ymin; 

    int job_h = h/jobcount; 

    printf("job_h %d h %d jobcount %d\n", job_h, h, jobcount); 
    // TODO: assert
    jobs = malloc(sizeof(mb_job) * (jobcount+1)); 
    results = malloc(sizeof(mb_result*) * (jobcount+1)); 

    for (jobs_left = 0; jobs_left < jobcount; jobs_left++)
    {
        mb_job* j = &jobs[jobs_left]; 

        j->job_id  = jobs_left; 
        j->w_start = w_start; 
        j->w_step  = w_step; 
        j->w       = w; 
        j->h_start = h_start + h_step * (j->job_id * job_h); 
        j->h_step  = h_step; 
        j->h       = job_h; 
    }

    // Insert extra job if we have left-over lines to cover. 
    if (job_h * jobcount < h) 
    {
        int last_h = h - job_h * jobcount; 
        mb_job* j = &jobs[jobs_left]; 
        j->job_id    = jobs_left; 
        j->w_start = w_start; 
        j->w_step  = w_step; 
        j->w       = w; 
        j->h_start = h_start + h_step * (jobcount * job_h); 
        j->h_step  = h_step; 
        j->h       = last_h; 
        jobs_left++; 
    }
}


/* Fetches a job, or returns NULL if none left */ 
mb_job *fetch_job() 
{
    int job_no; 
    pthread_mutex_lock(&job_mutex); 
    job_no = --jobs_left; 
    pthread_mutex_unlock(&job_mutex);
    if (job_no < 0) 
        return NULL; 
    return &jobs[job_no]; 
}

/* Allocate a result data structure, and add it to the global results array */
mb_result * allocate_result(int n_cells, int job_id)
{
    mb_result *r = malloc(sizeof(mb_result)); 
    r->res = malloc(sizeof(int) * n_cells); 
    r->job_id = job_id; 
    results[job_id] = r; 
    return r; 
}

/* Worker threads */
void* worker(void *arg)
{
    mb_job *job; 
    int maxit = 5000; 

    for (job = fetch_job(); job; job = fetch_job())
    {
        //printf("Running job %d\n", job->job_id); 
        mb_result *res = allocate_result(job->h * job->w, job->job_id); 
        //compute(res->res, job->h_step, job->w_step, job->h_start, job->w_start, job->h, job->w, maxit); 

    }
    return NULL; 
}


int main(int argc, char* argv[])
{
    int i; 

    if (argc < 5) 
    {
        printf("Usage: %s <nworkers> <njobs>\n", argv[0]); 
        exit(-1); 
    }
    n_workers = atoi(argv[1]); 
    int n_jobs    = atoi(argv[2]); 
    int width     = atoi(argv[3]); 
    int height    = atoi(argv[4]); 
    pthread_t    *workers = malloc(sizeof(pthread_t) * n_workers); 

    struct timeval tv_start, tv_stop; 
    double t_start, t_stop; 

    printf("PThreads mandelbrot.\n"); 
    printf(" Number of jobs : %d\n", n_jobs); 
    printf(" Width          : %d\n", width); 
    printf(" Height         : %d\n", height); 
    printf(" Workers        : %d\n", n_workers); 

    gettimeofday(&tv_start, NULL); 
    create_jobs(n_jobs, width, height); 

    for (i = 0; i < n_workers; i++) 
        pthread_create(&workers[i], NULL, worker, NULL); 
    
    for (i = 0; i < n_workers; i++) 
        pthread_join(workers[i], NULL); 
    gettimeofday(&tv_stop, NULL); 

    t_start = tv_start.tv_sec + tv_start.tv_usec / 1000000.0; 
    t_stop  = tv_stop.tv_sec  + tv_stop.tv_usec / 1000000.0; 
    double dt = t_stop - t_start; 

    printf(" *** Time spent: %f seconds\n", dt); 
    printf(" NB: includes job creation time and thread start/stop time\n"); 
    printf(" @(%d, %d, %d, %d, %f, 'pthreads')\n", n_jobs, width, height, n_workers, dt); 
    return 0; 
}
