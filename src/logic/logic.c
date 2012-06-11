#include "everything.h"

#define DIV 10
static int div_w;
static int div_h;

struct logic logic;

static unsigned char*
small_frame()
{
    int w, h;
    char *src    = video_array_cpy();
    char *gray_f = malloc(div_w * div_h);

    for (h = 0; h < video.height; h += DIV) {
        for (w = video.width; w > 0; w -= DIV) {
            gray_f[(video.width-w)/DIV + (h/DIV) * div_w] =
                src[(w + h * video.width) * 2];
        }
    }

    free(src);

    return (unsigned char*) gray_f;
}

/* Adds a sample to logic.data.val array.
 * No checking in it. */
static int
sample_add(int sample)
{
    ((int*) logic.data.val)[logic.samples++] = sample;

    return logic.samples;
}

static char
sample_found(int sample)
{
    int i = logic.max;

    while (i--) {
        if (((int*) logic.data.val)[i] == sample)
            return 1;
    }

    return 0;
}

static char
frame_cmp(unsigned char* a, unsigned char* b)
{
    int i, j, offset, segment;

    thread_lock(logic.data);
    logic.samples = 0;

    for (j = 0; j < div_h; j++) {
        for (i = 0; i < div_w; i++) {
            offset = i + j * div_w;

            segment = i * 8 / div_w + j * 4 / div_h * 8;

            /* If this one's already found, jump to next one. */
            if (sample_found(segment))
                continue;

            /* some (actual) comparing */
            if (abs(a[offset] - b[offset]) > LOGIC_THRESHOLD) {
                if (logic.max == sample_add(segment)) {
                    log_d("Sample array full!");
                    j = div_h;
                    break;
                }
            }
        }
    }
    thread_unlock(logic.data);

    return logic.samples;
}

static void
loop()
{
    uint8_t *initial = small_frame();
    uint8_t *new;

    for (;;) {
        new = small_frame();

        if (frame_cmp(initial, new)) {
            thread_cond_broadcast(&logic.data_arrived);
        }

        free(new);
    }
}

void
logic_init()
{
    int time = 3;

    log("Initializing logic.");

    thread_mutex_init(logic.data);
    thread_cond_init(&logic.data_arrived);
    logic.max  = 100;
    logic.data.val = malloc(sizeof(int) * (logic.max+1));

    div_w = video.width  / DIV;
    div_h = video.height / DIV;
    log("Scanning area scaled to %ix%i.", div_w, div_h);

    /* Wait till camera's stable. */
    sleep(time);

    log("Logic has been started.");
    loop();
}
