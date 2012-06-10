#include "everything.h"

#define DIV 10
static int div_w;
static int div_h;

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

    return abs(a[offset] - b[offset]) > 15;
}

static void
print_frame(uint8_t* f)
{
    int w,h;
    char scale[] = " .,:;!#$";
    uint8_t pixel;

    for (h = 0; h < div_h; h+=2) {
        for (w = 0; w < div_w; w++) {
            pixel = f[w + h * div_w];
            putchar(scale[(int) ((1 - pixel/ 255.0) * sizeof scale)]);
        }
        putchar('\n');
    }
}

static void
loop()
{
    uint8_t *initial = small_frame();
    uint8_t *new;

    for (;;) {
        new = small_frame();

        //print_frame(new);
        assert(!frame_cmp(initial, new), "Spot on!");

        free(new);
    }
}

void
logic_init(void* arg)
{
    int time = 3;

    log("Initializing logic.");

    div_w = video.width  / DIV;
    div_h = video.height / DIV;
    log("Scanning area scaled to %ix%i.", div_w, div_h);

    /* Wait till camera's stable. */
    sleep(time);

    log("Logic has been started.");
    loop();
}
