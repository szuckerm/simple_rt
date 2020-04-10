#ifndef VIRTUAL_CPU_H_GUARD
#define VIRTUAL_CPU_H_GUARD

#include "scheduler.h"

typedef struct virtual_cpu_s {
    pthread_t thread;
    size_t    id;
    taskq_t*  queue;
} vcpu_t;

void vcpu_create(vcpu_t** vcpu, const size_t id);
void vcpu_destroy(vcpu_t* vcpu);
void vcpu_map(vcpu_t* vcpu, size_t pcpu_id);


#ifndef //VIRTUAL_CPU_H_GUARD
