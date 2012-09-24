#include "everything.h"
#include <sys/time.h>

int main_thread_count;
pthread_t threads[10];

int
thread_new(void (*start_routine)(void*))
{
    return pthread_create(&threads[main_thread_count++], NULL,
            (void *(*)(void*)) start_routine, NULL);
}

void
thread_cond_init(thread_cond_t *c)
{
    assert_fatal(pthread_cond_init(c, NULL) == 0, "unable to init cond");
}

/* Craves relative instead of absolute time like pthread_cond_timedwait(). */
void
thread_cond_wait(thread_cond_t *c, struct thread_mutex m, struct timespec *rel)
{
    int err;
    struct timeval t;

    if (rel) {
        assert(gettimeofday(&t, NULL) == 0, "gettimeofday failed");
        rel->tv_sec  = rel->tv_sec  + t.tv_sec;
        rel->tv_nsec = rel->tv_nsec + t.tv_usec * 1000;
        err = pthread_cond_timedwait(c, &m.m, rel);
        if (err == ETIMEDOUT) {
            err = 0;
        }
    } else {
        err = pthread_cond_wait(c, &m.m);
    }

    assert_fatal(err == 0, "unable to wait for cond");
}

void
thread_cond_broadcast(thread_cond_t *c)
{
    assert(pthread_cond_broadcast(c) == 0, "unable to braodcast cond");
}

void
thread_mutex_init(struct thread_mutex l)
{
    assert_fatal(pthread_mutex_init(&l.m, NULL) == 0, "unable to init mutex");
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
join(pthread_t thread)
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
