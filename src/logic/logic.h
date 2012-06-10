#ifndef LOGIC_H
#define LOGIC_H

void logic_init();

struct logic {
    struct thread_mutex data;
    thread_cond_t data_arrived;
    int samples;
} logic;

#endif /* LOGIC_H */
