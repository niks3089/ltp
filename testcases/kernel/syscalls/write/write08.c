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
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h> 
#include<time.h> 
#include "tst_test.h"

static int fd;

#define RANDSIZE 8096
#define BUF_SIZE 8000000

static void verify_write(void)
{
	int i, badcount = 0;
    uint64_t start, end;
    uint64_t rand_arr[RANDSIZE];
	char buf[5]= {'1', '2', '3', '4', '5'};

    srand(time(0));
    for (i = 0; i < RANDSIZE; i++)
    {
        rand_arr[i] = rand() % 7999999;
    }

	SAFE_LSEEK(fd, 0, SEEK_SET);

    SYSCALL_PERF_SET_CPU();
    start = SYSCALL_PERF_GET_TICKS();
    //while (1) {
        for (i = 0; i < BUF_SIZE; i++) {
            pwrite(fd, &buf[i % 5], 1, rand_arr[i % RANDSIZE]);
            if (TST_RET == -1) {
                tst_res(TFAIL | TTERRNO, "write failed");
                printf("Wrote: %d bytes\n", i);
                break;
            }
        }
    //}
    end = SYSCALL_PERF_GET_TICKS();
    SYSCALL_PERF_MEASURE(start, end);

	if (badcount != 0)
		tst_res(TFAIL, "write() failed to return proper count");
	else
		tst_res(TPASS, "write() passed");
}

static void setup(void)
{
	fd = SAFE_OPEN("/dev/sdd", O_RDWR | O_DIRECT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_write,
	.needs_tmpdir = 1,
};
