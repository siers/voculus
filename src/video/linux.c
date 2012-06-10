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

struct video_buffer {
    void   *start;
    size_t length;
};

static struct video_buffer *buffers   = NULL;
static unsigned int n_buffers   = 0;

static int
_ioctl(int fd, int request, void * arg)
{
    int r;

    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);

    return r;
}

static int
read_frame(void)
{
    struct v4l2_buffer buf;

    CLEAR (buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == _ioctl (video.fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                return 0;

            case EIO:
                /* Could ignore EIO, see spec. */
                /* fall through */

            default:
                assert(0, "failure on ioctl.VIDIOC_DQBUF");
        }
    }

    assert(buf.index < n_buffers, "non-fatal assert");

    thread_lock(video.array);
    video.array.val = buffers[buf.index].start;
    thread_cond_broadcast(&video.array_new);

    assert_fatal(-1 != _ioctl (video.fd, VIDIOC_QBUF, &buf),
            "failure on ioctl.VIDIOC_QBUF");

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
        FD_SET (video.fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select (video.fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                continue;

            assert(0, "select == -1");
        }

        assert_fatal(0 != r, "select timeout");
        assert(read_frame() == 1, "error when reading frame");
    }
}

static void
start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;

        assert_fatal(-1 != _ioctl (video.fd, VIDIOC_QBUF, &buf),
            "failure on ioctl.VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    assert_fatal(-1 != _ioctl (video.fd, VIDIOC_STREAMON, &type),
        "failure on ioctl.VIDIOC_STREAMON");
}

static void
uninit_device(void)
{
    unsigned int i;

    for (i = 0; i < n_buffers; ++i)
        assert(-1 != munmap (buffers[i].start, buffers[i].length),
            "munmap");
}

static void
init_mmap(void)
{
    struct v4l2_requestbuffers req;

    CLEAR (req);

    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;

    if (-1 == _ioctl (video.fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            assert_fatal(0, "%s does not support "
                "memory mapping", video.dev_name);
        } else {
            assert_fatal(0, "failure on ioctl.VIDIOC_REQBUFS");
        }
    }

    assert_fatal(req.count >= 2, "Insufficient buffer memory on %s", video.dev_name);

    buffers = calloc (req.count, sizeof (*buffers));

    assert_fatal(buffers, "Out of memory");

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        assert_fatal(-1 != _ioctl (video.fd, VIDIOC_QUERYBUF, &buf),
            "failure on ioctl.VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
            mmap (NULL /* start anywhere */,
                    buf.length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    video.fd, buf.m.offset);

        assert_fatal(MAP_FAILED != buffers[n_buffers].start, "mmap failed");
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
    if (-1 == ioctl (video.fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (errno != EINVAL)
            log("failure on ioctl.VIDIOC_QUERYCTRL");
        else
            log("V4L2_CID_BRIGHTNESS is not supported");
    } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
            log("V4L2_CID_BRIGHTNESS is not supported");
    } else {
        memset (&control, 0, sizeof (control));
        control.id = V4L2_CID_BRIGHTNESS;
        control.value = queryctrl.default_value;
        log("Brightness set to %i.", queryctrl.default_value);
    }

    if (-1 == _ioctl (video.fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            assert_fatal(0, "%s is no V4L2 device\n",
                    video.dev_name);
        } else {
            assert_fatal(0, "VIDIOC_QUERYCAP");
        }
    }

    assert_fatal(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE,
        "%s is no video capture device", video.dev_name);

    /* Select video input, video standard and tune here. */

    CLEAR (cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == _ioctl (video.fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == _ioctl (video.fd, VIDIOC_S_CROP, &crop)) {
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
    fmt.fmt.pix.width       = video.width;
    fmt.fmt.pix.height      = video.height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    assert_fatal(-1 != _ioctl (video.fd, VIDIOC_S_FMT, &fmt),
        "failure on ioctl.VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    init_mmap ();
}

static void
close_device(void)
{
    assert_fatal(-1 != close (video.fd), "cannot close video device's fd");
    video.fd = -1;
}

static void
open_device(void)
{
    struct stat st;

    assert_fatal(-1 != stat (video.dev_name, &st), "Cannot identify '%s': %d, %s\n",
            video.dev_name, errno, strerror (errno));

    assert_fatal(S_ISCHR (st.st_mode), "%s is no device\n",
            video.dev_name);

    video.fd = open (video.dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    assert_fatal(-1 != video.fd, "Cannot open '%s': %d, %s\n",
                video.dev_name, errno, strerror(errno));
}

int
video_specific_init()
{
    video.dev_name = "/dev/video0";
    open_device ();
    init_device ();
    start_capturing ();
    capture();

    return 0;
}

void
video_specific_die()
{
    uninit_device ();
    close_device ();
}
