#pragma once

#include "pthread.h"

typedef pthread_t thread_t;

struct thread_mutex {
    pthread_mutex_t m;
    void* val;
} thread_mutex;

void thread_mutex_init(struct thread_mutex l);
void thread_lock  (struct thread_mutex);
void thread_unlock(struct thread_mutex);

int  thread_new(void (*start_routine)(void*), void* arg);
void threads_join();
