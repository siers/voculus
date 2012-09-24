#pragma once

#include "thread.h"

void video_init();
void video_specific_init();
void video_specific_die();
int video_die();

int video_get_array_size();
void* video_array_cpy();

struct video {
    int width;
    int height;

    struct thread_mutex array;
    pthread_cond_t array_new;

    char* dev_name;
    int fd;
} video;
