#include "everything.h"

int main_thread_count;
thread_t threads[10];

int
thread_new(void (*start_routine)(void*), void* arg)
{
    return pthread_create(&threads[main_thread_count++], NULL,
            (void *(*)(void*)) start_routine, arg);
}

void
thread_mutex_init(struct thread_mutex l)
{
    pthread_mutex_init(&l.m, NULL);
}

void
thread_lock(struct thread_mutex l)
{
    assert_fatal(pthread_mutex_lock(&l.m) == 0, "unable to lock thread");
}

void
thread_unlock(struct thread_mutex l)
{
    assert_fatal(pthread_mutex_unlock(&l.m) == 0, "unable to unlock thread");
}

static void
join(thread_t thread)
{
    void *retval;

    assert_fatal(pthread_join(thread, &retval) == 0,
            "thread joined with errors");
    log_d("thread.join");
}

void
threads_join()
{
    while (main_thread_count--) {
        join(threads[main_thread_count]);
    }
}
