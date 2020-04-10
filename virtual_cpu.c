#define _GNU_SOURCE
#include <sched.h>

#include "virtual_cpu.h"
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
    if (vcpu != NULL) {
        memset(vcpu, 0, sizeof(vcpu_t));
        free(vcpu);
    }
}

void vcpu_map(vcpu_t* vcpu, size_t pcpu_id)
{
    vcpu->thread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(pcpu_id, &cpuset);

    if (pthread_setaffinity_np(vcpu->thread, sizeof(cpu_set_t), &cpuset) != 0) 
        FATAL("pthread_setaffinity_np");

    DEBUG({
        if (CPU_ISSET(pcpu_id, &cpuset))
            printf("CPU -> %lu\n", pcpu_id);
    });
}
