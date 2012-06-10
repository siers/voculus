#include "everything.h"

#ifdef __linux
#include "video/linux.h"
#include "video/linux.c"
#endif /* __linux */

/* the sane defaults */
struct video video = {
    .width  = 640,
    .height = 480
};

void
video_init(void* arg)
{
    log("initializing video");
    thread_mutex_init(video.array);
    video.array.val = NULL;

    video_specific_init();
    video_specific_die();
}

int
video_get_array_size()
{
    return video.width * video.height * 2;
}

/* FOOTNOTE: The matrix is 2-bytes for each cell, so
 * it's size is width * height * 2.
 * 2 bytes -- Y' & UV, respectively. Y' = luma, UV = chrominance.
 */
