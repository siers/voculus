#include "everything.h"

int main_thread_count;
thread_t threads[10];

int
thread_new(void (*start_routine)(void*), void* arg)
{
    return pthread_create(&threads[main_thread_count++], NULL,
            (void *(*)(void*)) start_routine, arg);
}

static void
join(thread_t thread)
{
    void *retval;

    assert_fatal(pthread_join(thread, &retval) == 0,
            "thread joined with errors")
}

void
threads_join()
{
    while (--main_thread_count) {
        join(threads[main_thread_count]);
    }
}
