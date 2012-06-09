/*  Voculus -- the eye of webcam.
    Copyright (C) 2011 siers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "everything.h"

int loglvl = (uint16_t) -1; /* everything */

int
main(void)
{
    assert_fatal(!thread_new(video_init, NULL), "failed to init video");
    assert_fatal(!thread_new(graph_init, NULL), "failed to init graph");
    assert_fatal(!thread_new(logic_init, NULL), "failed to init logic");

    threads_join();

    return 0;
}
