#pragma once

int video_init();
int video_die();

struct video {
    int width;
    int height;
    int fd;
} video;
