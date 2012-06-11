#ifndef LOGIC_H
#define LOGIC_H

#define LOGIC_THRESHOLD 15

void logic_init();

struct logic {
    struct thread_mutex data;
    thread_cond_t data_arrived;
    int samples;
    int max;
} logic;

#endif /* LOGIC_H */
