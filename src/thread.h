#pragma once

#include "pthread.h"

typedef pthread_t thread_t;

int  thread_new(void (*start_routine)(void*), void* arg);
void threads_join();
