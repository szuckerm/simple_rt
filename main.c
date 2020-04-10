#include "scheduler.h"

int g_CNT = 0;

void* goodbye(void* arg)
{
    printf("good-bye, %s!\n", (char*)arg);
    --g_CNT;
    if (g_CNT <= 0) 
        queue_stop(g_QUEUE);
    return NULL;
}

void* hello(void* arg)
{
    printf("Hello, %s!\n", (char*)arg);
    queue_push(g_QUEUE, task_create(goodbye,arg));
    return NULL;
}

int main(int argc, char** argv)
{
    g_QUEUE = queue_create();

    g_CNT = argc-1;

    DEBUG(printf("argc = %d\n", argc));
    for (int i = 1; i < argc; ++i) {
        DEBUG(printf("i = %d\n", i));
        task_new(hello, argv[i]);
        //queue_push(g_QUEUE, task_create(hello, argv[i]));
    }


    DEBUG(printf("Queue size = %lu\n", g_QUEUE->n_tasks));
    schedule(g_QUEUE);

    queue_destroy(g_QUEUE);

    return EXIT_SUCCESS;
}
