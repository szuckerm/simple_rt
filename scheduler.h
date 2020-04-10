#ifndef SCHEDULER_H_GUARD
#define SCHEDULER_H_GUARD


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <utils.h>

#ifdef USE_PTHREAD
#   include <pthread.h>
#endif // USE_PTHREAD

#define TASKQ_MAX_SZ 100UL
#define EMPTY_CODE NULL
#define EMPTY_DATA NULL

#define TASK_IS_EMPTY(task) ((task) == &g_EMPTY_TASK)

#define HW_TASK_INPUT_SIGNAL_CHECK()
#define HW_TASK_OUTPUT_SIGNAL_CHECK()

#ifndef NDEBUG
#   define DEBUG(code) do { (code); } while(0)
#else
#   define DEBUG(code) 
#endif


typedef void* (*code_t)(void*); 
typedef void*   data_t;

typedef enum priority_e {
    HIGHEST = -20, 
    NORMAL  =   0, 
    LOWEST  =  20
} priority_t;

typedef struct task_s {
    code_t code;
    data_t data;
    /* priority_t priority; */
    /* task_t* next_to_run; */
} task_t;

typedef struct task_queue_s {
    task_t          *tasks[TASKQ_MAX_SZ];
    size_t           n_tasks,
                     head, 
                     tail;
    bool             pause;
    bool             stop;
    pthread_mutex_t *lock; // For now -- later: use house-made lock
} taskq_t;

#define QUEUE_IS_EMPTY(queue) ((queue)->head == (queue)->tail)
#define QUEUE_IS_FULL(queue)  \
    ( ((queue)->tail - (queue)->head) % TASKQ_MAX_SZ == TASKQ_MAX_SZ-1 )

extern task_t   g_EMPTY_TASK;
extern taskq_t* g_QUEUE;

task_t*  task_create(code_t code, data_t data);
void     task_destroy(task_t* t);
task_t*  queue_pop(taskq_t* q);
int      queue_push(taskq_t* q, task_t* task);
taskq_t* queue_create(void);
void     queue_destroy(taskq_t* q);
void     queue_stop(taskq_t* q);
void     schedule(taskq_t* q) ;
void     runtime_create(void);

static inline void task_new(code_t code, data_t data) {
    task_t* t = task_create(code, data);
    if (t == NULL) 
        FATAL("couldn't create new task");

    queue_push(g_QUEUE, t);
}


#endif // SCHEDULER_H_GUARD
