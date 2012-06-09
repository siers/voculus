#include "everything.h"
#include "video/video.h"

#ifdef __linux
#include "video/linux.h"
#include "video/linux.c"
#endif /* __linux */

/* the sane defaults */
int video_width  = 640;
int video_height = 480;

void
video_init(void* arg)
{
    log("initializing video");
    video_specific_init();
    video_specific_die();
}

int
video_get_array_size()
{
    return video_width * video_height * 2;
}

char***
video_get_array()
{
    return video_get_array();
}

/* FOOTNOTE: The matrix is 2-bytes for each cell, so
 * it's size is width * height * 2.
 * 2 bytes -- Y' & UV, respectively. Y' = luma, UV = chrominance.
 */
