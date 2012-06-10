#pragma once

void video_init();
int video_die();

int video_get_array_size();
void* video_array_cpy();

struct video {
    int width;
    int height;

    struct thread_mutex array;
    thread_cond_t array_new;

    char* dev_name;
    int fd;
} video;
