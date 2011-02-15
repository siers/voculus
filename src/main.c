#include "everything.h"

int loglvl = (uint16_t) -1; /* everything */

int
main(void)
{
    assert_fatal(!video_init(), "failed to init video");
    assert_fatal(!graph_init(), "failed to init graph");
    assert_fatal(!logic_init(), "failed to init logic");

    return 0;
}
