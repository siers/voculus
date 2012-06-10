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

static char
frame_cmp(unsigned char* a, unsigned char* b)
{
    int offset = div_w / 2 + (div_h / 2) * div_w;
    char result;
    int *buf;

    result = abs(a[offset] - b[offset]) > 15;

    if (result) {
        thread_lock(logic.data);
        memset(logic.data.val, 0, logic.samples);
        buf = logic.data.val;
        buf[0] = 0xde;
        buf[1] = 0xad;
        buf[2] = 0xbe;
        buf[3] = 0xef;
        thread_unlock(logic.data);
    }

    return result;
}

static void
loop()
{
    uint8_t *initial = small_frame();
    uint8_t *new;

    for (;;) {
        new = small_frame();

        if (frame_cmp(initial, new)) {
            log("Tada!");
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
    logic.samples  = 100;
    logic.data.val = malloc(sizeof(int) * (logic.samples+1));

    div_w = video.width  / DIV;
    div_h = video.height / DIV;
    log("Scanning area scaled to %ix%i.", div_w, div_h);

    /* Wait till camera's stable. */
    sleep(time);

    log("Logic has been started.");
    loop();
}
