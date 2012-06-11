/*  Voculus -- the eye of webcam.
    Copyright (C) 2012 siers */

#include "everything.h"
#include <locale.h>

int loglvl = (uint16_t) -1; /* everything */

int
main(void)
{
    setlocale(LC_CTYPE, "");

    assert_fatal(!thread_new(video_init), "failed to init video");
    assert_fatal(!thread_new(logic_init), "failed to init logic");
    assert_fatal(!thread_new(demonstr_init), "failed to init demonstr");

    threads_join();

    return 0;
}
