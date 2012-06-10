#include "everything.h"

#include <stdio.h>
#include <unistd.h>

static void
process_data(const void* p)
{
    int i, j;
    char scale[] = " .,:;!#$";
    unsigned char luma;

    for (j = 0; j < 480; j+=20) {
        for (i = 640; i > 0; i-=10) {
            luma = ((unsigned char*) p)[(i + j * 640)*2];
            putchar(scale[(int) ((1 - luma / 255.0) * sizeof scale)]);
        }
        putchar('\n');
    }
    fflush(stdout);
}

static void
loop()
{
    for (;;) {
        thread_lock(video.array);

        if (video.array.val)
            process_data(video.array.val);

        thread_unlock(video.array);

        usleep(66666);
    }
}

void
graph_init(void* arg)
{
    log("initializing graph");
    loop();
}
