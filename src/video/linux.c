#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#include "../everything.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

struct buffer {
    void   *start;
    size_t length;
};

static char *dev_name           = NULL;
static io_method io             = IO_METHOD_MMAP;
static int fd                   = -1;
struct buffer *buffers          = NULL;
static unsigned int n_buffers   = 0;

static void
errno_exit(const char * s)
{
    fprintf (stderr, "%s error %d, %s\n",
            s, errno, strerror (errno));

    exit (EXIT_FAILURE);
}

static int
_ioctl(int fd, int request, void * arg)
{
    int r;

    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);

    return r;
}

static void
process_image(const void * p)
{
    int i, j;
    char scale[] = "12345678";
    unsigned char luma;

    for (j = 0; j < 480; j ++) {
        for (i = 640; i; i --) {
            luma = ((unsigned char*) p)[(i + j * 640)*2];
            fputc(scale[(int) ((1 - luma / 255.0) * sizeof scale)], stdout);
        }
        fputc('\n',stdout);
    }
    fputc ('\n', stdout);
    fputc ('\n', stdout);
    fflush (stdout);
}

static int
read_frame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io) {
        case IO_METHOD_READ:
            if (-1 == read (fd, buffers[0].start, buffers[0].length)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errno_exit ("read");
                }
            }

            process_image (buffers[0].start);

            break;

        case IO_METHOD_MMAP:
            CLEAR (buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == _ioctl (fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errno_exit ("VIDIOC_DQBUF");
                }
            }

            assert(buf.index < n_buffers, "non-fatal assert");

            process_image (buffers[buf.index].start);

            if (-1 == _ioctl (fd, VIDIOC_QBUF, &buf))
                errno_exit ("VIDIOC_QBUF");

            break;

        case IO_METHOD_USERPTR:
            CLEAR (buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == _ioctl (fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        errno_exit ("VIDIOC_DQBUF");
                }
            }

            for (i = 0; i < n_buffers; ++i)
                if (buf.m.userptr == (unsigned long) buffers[i].start
                        && buf.length == buffers[i].length)
                    break;

            assert(i < n_buffers, "non-fatal assert");

            process_image ((void *) buf.m.userptr);

            if (-1 == _ioctl (fd, VIDIOC_QBUF, &buf))
                errno_exit ("VIDIOC_QBUF");

            break;
    }

    return 1;
}

static void
capture()
{
    for (;;) {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO (&fds);
        FD_SET (fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select (fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                continue;

            assert(0, "select == -1");
        }

        if (0 == r) {
            fprintf (stderr, "select timeout\n");
            exit (EXIT_FAILURE);
        }

        if (read_frame ())
            break;
    }
}

static void
stop_capturing(void)
{
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            assert(-1 != _ioctl (fd, VIDIOC_STREAMOFF, &type),
                    "failure on ioctl.VIDIOC_STREAMOFF")

            break;
    }
}

static void
start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR (buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = i;

                if (-1 == _ioctl (fd, VIDIOC_QBUF, &buf))
                    errno_exit ("VIDIOC_QBUF");
            }

            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (-1 == _ioctl (fd, VIDIOC_STREAMON, &type))
                errno_exit ("VIDIOC_STREAMON");

            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR (buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_USERPTR;
                buf.index       = i;
                buf.m.userptr   = (unsigned long) buffers[i].start;
                buf.length      = buffers[i].length;

                if (-1 == _ioctl (fd, VIDIOC_QBUF, &buf))
                    errno_exit ("VIDIOC_QBUF");
            }

            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (-1 == _ioctl (fd, VIDIOC_STREAMON, &type))
                errno_exit ("VIDIOC_STREAMON");

            break;
    }
}

static void
uninit_device(void)
{
    unsigned int i;


    switch (io) {
        case IO_METHOD_READ:
            free (buffers[0].start);
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i)
                if (-1 == munmap (buffers[i].start, buffers[i].length))
                    errno_exit ("munmap");
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i)
                free (buffers[i].start);
            break;
    }

    free (buffers);
}

static void
init_read(unsigned int buffer_size)
{
    buffers = calloc (1, sizeof (*buffers));

    if (!buffers) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc (buffer_size);

    if (!buffers[0].start) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }
}

static void
init_mmap(void)
{
    struct v4l2_requestbuffers req;

    CLEAR (req);

    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;

    if (-1 == _ioctl (fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s does not support "
                    "memory mapping\n", dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf (stderr, "Insufficient buffer memory on %s\n",
                dev_name);
        exit (EXIT_FAILURE);
    }

    buffers = calloc (req.count, sizeof (*buffers));

    if (!buffers) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        if (-1 == _ioctl (fd, VIDIOC_QUERYBUF, &buf))
            errno_exit ("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
            mmap (NULL /* start anywhere */,
                    buf.length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit ("mmap");
    }
}

static void
init_userp(unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;
    unsigned int page_size;

    page_size = getpagesize ();
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    CLEAR (req);

    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_USERPTR;

    if (-1 == _ioctl (fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s does not support "
                    "user pointer i/o\n", dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_REQBUFS");
        }
    }

    buffers = calloc (4, sizeof (*buffers));

    if (!buffers) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = memalign (/* boundary */ page_size,
                buffer_size);

        if (!buffers[n_buffers].start) {
            fprintf (stderr, "Out of memory\n");
            exit (EXIT_FAILURE);
        }
    }
}

static void
init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;
    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;

    memset (&queryctrl, 0, sizeof (queryctrl));
    queryctrl.id = V4L2_CID_BRIGHTNESS;

    if (-1 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (errno != EINVAL) {
            perror ("VIDIOC_QUERYCTRL");
            exit (EXIT_FAILURE);
        } else {
            printf ("V4L2_CID_BRIGHTNESS is not supported\n");
        }
    } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        printf ("V4L2_CID_BRIGHTNESS is not supported\n");
    } else {
        memset (&control, 0, sizeof (control));
        control.id = V4L2_CID_BRIGHTNESS;
        control.value = queryctrl.default_value;
        printf ("V4L2_CID_BRIGHTNESS defaullt = %i\n",
                queryctrl.default_value);

        /*
           if (-1 == ioctl (fd, VIDIOC_S_CTRL, &control)) {
           perror ("VIDIOC_S_CTRL");
           exit (EXIT_FAILURE);
           }*/
    }



    if (-1 == _ioctl (fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s is no V4L2 device\n",
                    dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf (stderr, "%s is no video capture device\n",
                dev_name);
        exit (EXIT_FAILURE);
    }

    switch (io) {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                fprintf (stderr, "%s does not support read i/o\n",
                        dev_name);
                exit (EXIT_FAILURE);
            }

            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                fprintf (stderr, "%s does not support streaming i/o\n",
                        dev_name);
                exit (EXIT_FAILURE);
            }

            break;
    }


    /* Select video input, video standard and tune here. */


    CLEAR (cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == _ioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == _ioctl (fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    } else {
        /* Errors ignored. */
    }


    CLEAR (fmt);

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (-1 == _ioctl (fd, VIDIOC_S_FMT, &fmt))
        errno_exit ("VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    switch (io) {
        case IO_METHOD_READ:
            init_read (fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            init_mmap ();
            break;

        case IO_METHOD_USERPTR:
            init_userp (fmt.fmt.pix.sizeimage);
            break;
    }
}

static void
close_device(void)
{
    assert_fatal(-1 == close (fd), "cannot close video device's fd");
    fd = -1;
}

static void
open_device(void)
{
    struct stat st;

    assert_fatal(-1 == stat (dev_name, &st), "Cannot identify '%s': %d, %s\n",
            dev_name, errno, strerror (errno));

    assert_fatal(!S_ISCHR (st.st_mode), "%s is no device\n",
            dev_name);

    fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    assert_fatal(-1 == fd, "Cannot open '%s': %d, %s\n",
                dev_name, errno, strerror (errno));
}

int
video_specific_init()
{
    dev_name = "/dev/video0";
    open_device ();
    init_device ();
    start_capturing ();
    capture();
    stop_capturing ();
    uninit_device ();
    close_device ();

    return 0;
}

void
video_specific_die()
{
    stop_capturing ();
    uninit_device ();
    close_device ();
}
