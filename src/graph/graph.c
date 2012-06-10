#include "everything.h"

static void
process_data(const void* p)
{
    int i, j;
    char scale[] = " .,:;!#$";
    unsigned char luma;

    for (j = 0; j < video.height; j+=20) {
        for (i = video.width; i > 0; i-=10) {
            luma = ((unsigned char*) p)[(i + j * video.width) * 2];
            fputc(scale[(int) ((1 - luma / 255.0) * sizeof scale)], stderr);
        }
        fputc('\n', stderr);
    }
    fflush(stderr);
}

static void
loop()
{
    void* mem;

    for (;;) {
        if (video.array.val) {
            mem = video_array_cpy();
            process_data(mem);
            free(mem);
        }
    }
}

void
graph_init()
{
    log("Initializing graph.");
    //loop();
}
