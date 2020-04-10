#include "scheduler.h"


#ifdef USE_PTHREAD
#   define lock(l)      do {                          \
        useconds_t delay = g_MIN_DELAY;               \
        while (pthread_mutex_trylock(q->lock) != 0) { \
            usleep(delay);                            \
            if (delay < g_MAX_DELAY)                  \
                delay *= 2;                           \
        }                                             \
    } while (0)
#   define trylock(l) do {              \
        pthread_mutex_trylock( (l) )    \
    } while (0)
#   define unlock(l)    pthread_mutex_unlock( (l) )
#   define lock_init(l) pthread_mutex_init(   (l), NULL)
#else
#   define lock(l)         (void)0
#   define unlock(l)       (void)0
#   define lock_init(l)    (void)0
#endif // USE_PTHREAD

const useconds_t g_MAX_DELAY = 1000, // cannot wait for more than  1 millisecond
                 g_MIN_DELAY = 10;   // cannot wait for less than 10 microseconds

task_t   g_EMPTY_TASK = {.code = EMPTY_CODE, .data = EMPTY_DATA};
taskq_t* g_QUEUE;

/*
 * Simple task queue function that gets the next task from the FIFO queue.
 * XXX FIXME Note: this function is *NOT* thread-safe!
 */
task_t* queue_pop(taskq_t* q) 
{
    /* Thread-safety done here if macro is set */
    lock(q->lock);

    // Return empty task sentinel if queue is empty or not allocated
    if (NULL == q || 0 == q->n_tasks) {
        DEBUG(fprintf(stderr, "QUEUE IS EMPTY!\n"));
        unlock(q->lock);
        return &g_EMPTY_TASK;
    }

    // remove the task from the queue; store it in task ptr
    task_t* t = q->tasks[q->head];
    --q->n_tasks; 
    q->tasks[q->head] = &g_EMPTY_TASK;
    q->head = (q->head + 1) % TASKQ_MAX_SZ;

    unlock(q->lock);

    DEBUG({
        // the following should be useless -- this is for debugging purposes
        if (q->head == q->tail) // queue is empty ! 
            q->n_tasks = 0; // should already be the case
    });

    return t;
}

/*
 * Simple task queue function that puts a new task at the tail of the FIFO queue
 * Note: this function is *NOT* thread-safe!
 */
int queue_push(taskq_t* q, task_t* task)
{

    /* thread-safety here, if macro is set */
    lock(q->lock);

    if ( true == QUEUE_IS_FULL(q) ) {
        DEBUG(
            fprintf(stderr, 
                "QUEUE IS FULL! Details:\ntail = %lu, head = %lu, (tail-head) mod %lu = %lu\n",
                q->tail, q->head, TASKQ_MAX_SZ, (q->tail-q->head)%TASKQ_MAX_SZ));
        unlock(q->lock);
        return -1; // error : task queue is full !
    }

    DEBUG({
        /* Debug code to check that the task slot is indeed empty. */
        if (! TASK_IS_EMPTY(q->tasks[q->tail]) ) {
            task_t *t = q->tasks[q->tail];
                fprintf(stderr, 
                    "The task at index %lu is not empty! Address is %p (code: %p, data: %p)\n",
                    q->tail, (void*)t, (void*)t->code, (void*)t->data);
        }
    });
    q->tasks[q->tail] = task;
    q->tail = (q->tail + 1) % TASKQ_MAX_SZ;

    ++q->n_tasks;

    unlock(q->lock);
    return 0; // everything went fine
}

task_t* task_create(code_t code, data_t data)
{
    task_t* t = malloc(sizeof(task_t));
    if (!t) {
        fprintf(stderr, "%s:%d:\t", __FILE__, __LINE__);
        perror("malloc");
        exit(errno);
    }
    *t = (task_t){ .code = code, .data = data };
    DEBUG(printf("Created task %p (%p, %p)\n", (void*)t, (void*)t->code, (void*)t->data));
    return t;
}

void task_destroy(task_t* t)
{
    memset(t,0,sizeof(task_t));
    if (t) free(t);
}

taskq_t* queue_create(void) 
{
    // allocate memory and sets it to all zeros
    taskq_t* q = calloc(1ul, sizeof(taskq_t));

    // error management: quit gracefully if allocation fails
    if (!q) {
        fprintf(stderr, "%s:%d:\t", __FILE__, __LINE__);
        perror("malloc");
        exit(errno);
    }
    DEBUG({
        /* Initializes task slot to the address of an empty task sentinel */
        for (size_t i = 0; i < TASKQ_MAX_SZ; ++i) 
            q->tasks[i] = &g_EMPTY_TASK;
    });

    lock_init(q->lock);
    q->pause = false;
    q->stop  = false;

    return q;
}

void queue_destroy(taskq_t* q)
{
    memset(q,0,sizeof(taskq_t));
    if (q != NULL) free(q);
}

void queue_stop(taskq_t* q)
{
    q->stop = true;
}

static inline bool queue_is_poppable(taskq_t* q) {
    bool state = 0;
    lock(q->l);
    state = q->n_tasks != 0 && q->pause == false;
    unlock(q->lock);
    return state;
}

void schedule(taskq_t* q) 
{
    useconds_t delay = g_MIN_DELAY; 
    while (! q->stop) {
        while ( false == queue_is_poppable(q) )
        {
            usleep(delay);
            if (delay < g_MAX_DELAY)
                delay *= 2;
        }

        delay = g_MIN_DELAY;

        task_t* t = queue_pop(q);
        if ( false == TASK_IS_EMPTY(t) ) {
            // check and set flags to reconfigure FPGA with new bitstream
            HW_TASK_INPUT_SIGNAL_CHECK(); 
            t->code(t->data);
            // check for end-of-task flag to be set by HW task
            HW_TASK_OUTPUT_SIGNAL_CHECK();
            task_destroy(t);
        }
    }
}

void runtime_create(void)
{

}
