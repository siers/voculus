#pragma once

void video_init(void*);
int video_die();

struct video {
    int width;
    int height;

    struct thread_mutex array;

    char* dev_name;
    int fd;
} video;
