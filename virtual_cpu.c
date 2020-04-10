#define _GNU_SOURCE
#include <sched.h>
#include "scheduler.h"
#include <pthread.h>

#include "/home/lasher/local/include/utils.h"

typedef struct virtual_cpu_s {
    pthread_t thread;
    size_t    id;
    taskq_t*  queue;
} vcpu_t;

void vcpu_create(vcpu_t** vcpu, const size_t id)
{
    vcpu_t* tmp = calloc(1, sizeof(vcpu_t));
    if (!tmp) {
        fprintf(stderr, "%s:%d\t", __FILE__, __LINE__);
        perror("calloc");
        exit(errno);
    }

    tmp->id    = id;
    tmp->queue = queue_create();
    *vcpu      = tmp;
}

void vcpu_destroy(vcpu_t* vcpu)
{
    queue_destroy(vcpu->queue);
    memset(vcpu, 0, sizeof(vcpu_t));
    if (vcpu != NULL) 
        free(vcpu);
}

void vcpu_map(vcpu_t* vcpu, size_t pcpu_id)
{
    vcpu->thread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(pcpu_id, &cpuset);
    int err = pthread_setaffinity_np(vcpu->thread, sizeof(cpu_set_t), &cpuset);
    if (err) {
        fprintf(stderr, "%s:%d\t", __FILE__, __LINE__);
        perror("pthread_setaffinity_np");
        exit(errno);
    }
    DEBUG({
        if (CPU_ISSET(pcpu_id, &cpuset))
            printf("CPU -> %lu\n", pcpu_id);
    });
}
