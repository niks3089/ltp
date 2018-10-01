/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

char *TCID = "getpid03";

int main(int ac, char **av)
{
    uint64_t i, lc;
    uint64_t start, end;
	tst_parse_opts(ac, av, NULL, NULL);
	setup();

    SYSCALL_PERF_SET_CPU();
    lc = SYSCALL_GET_LOOP_COUNTER();
    start = SYSCALL_PERF_GET_TICKS();
	for (i = 0; i < lc; i++) {
		TEST(getpid());
    }
    end = SYSCALL_PERF_GET_TICKS();
    SYSCALL_PERF_MEASURE(start, end);
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

void cleanup(void)
{
}
