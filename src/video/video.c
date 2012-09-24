#include "everything.h"

/* the sane defaults */
struct video video = {
    .width  = 640,
    .height = 480
};

void
video_init()
{
    log("Initializing video.");

    thread_mutex_init(video.array);
    video.array.val = NULL;
    thread_cond_init(&video.array_new);

    video_specific_init(); // Should loop.
    video_specific_die();
}

int
video_get_array_size()
{
    return video.width * video.height * 2;
}

void*
video_array_cpy()
{
    void* tmp;
    int size;

    size = video_get_array_size();
    tmp = malloc(size);

    thread_cond_wait(&video.array_new, video.array, NULL);
    memcpy(tmp, video.array.val, size);
    thread_unlock(video.array);

    return tmp;
}

/* FOOTNOTE: The matrix is 2-bytes for each cell, so
 * it's size is width * height * 2.
 * 2 bytes -- Y' & UV, respectively. Y' = luma, UV = chrominance.
 */
