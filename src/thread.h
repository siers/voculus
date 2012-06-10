#pragma once

#include "pthread.h"

typedef pthread_t thread_t;
typedef pthread_cond_t thread_cond_t;

struct thread_mutex {
    pthread_mutex_t m;
    void* val;
} thread_mutex;

void thread_cond_init(thread_cond_t*);
void thread_cond_wait(thread_cond_t*, struct thread_mutex);
void thread_cond_broadcast(thread_cond_t *c);

void thread_mutex_init(struct thread_mutex);
void thread_lock  (struct thread_mutex);
void thread_unlock(struct thread_mutex);

int  thread_new(void (*start_routine)(void*));
void threads_join();
