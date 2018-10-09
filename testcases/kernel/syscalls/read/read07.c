/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Fujitsu Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h> 
#include<time.h> 
#include "tst_test.h"

#define SIZE 512
#define RANDSIZE 8096
#define BUF_SIZE 8000000

static int fd;
static char buf[SIZE];

static void verify_read(void)
{
    uint64_t start, end, i = 0;
    uint64_t rand_arr[RANDSIZE];
    
    srand(time(0));
    for (i = 0; i < RANDSIZE; i++)
    {
        rand_arr[i] = rand() % 7999999;
    }

	SAFE_LSEEK(fd, 0, SEEK_SET);

    SYSCALL_PERF_SET_CPU();
    start = SYSCALL_PERF_GET_TICKS();
    while(i++ < 1000000000) {
        //TEST(read(fd, buf, SIZE));
        TEST(pread(fd, buf, SIZE, rand_arr[i % RANDSIZE]));

        if (TST_RET == -1) {
            tst_res(TFAIL | TTERRNO, "read(2) failed");
            break;
        }
    }
    end = SYSCALL_PERF_GET_TICKS();
    SYSCALL_PERF_MEASURE(start, end);
    tst_res(TPASS, "read(2) returned %ld, %lld", TST_RET, i);
}

static void setup(void)
{
	fd = SAFE_OPEN("/dev/sdd", O_RDWR, 0700);
	//SAFE_WRITE(1, fd, buf, SIZE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_read,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
