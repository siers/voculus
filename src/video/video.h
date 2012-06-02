#ifndef VIDEO_H
#define VIDEO_H

int video_init();
int video_die();

struct video {
    int width = 640;
    int height = 480;
    int fd;
} video;

#endif /* VIDEO_H */
