#pragma once

void video_init(void*);
int video_die();

struct video {
    int width;
    int height;
    int fd;
} video;
