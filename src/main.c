/*  Voculus -- the eye of webcam.
    Copyright (C) 2012 siers

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
