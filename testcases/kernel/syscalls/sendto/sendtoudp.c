/*
 * Copyright(c) 2016 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * alone with this program.
 */

/*
 * Test Name: sendto02
 *
 * Description:
 * When udp protocol is selected in socket(2) and buffer is invalid,
 * sendto(2) should fail and set errno to EFAULT, but it sets errno
 * to ENOMEM.
 *
 * This is a regression test and has been fixed by kernel commit:
 * 6e51fe7572590d8d86e93b547fab6693d305fd0d (udp: fix -ENOMEM result
 * with invalid user space pointer in sendto() syscall)
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tst_test.h"

#ifndef IPPROTO_UDP
# define IPPROTO_UDP	132
#endif

#define RANDOM_SIZE 8000000
#define PACKET_SIZE  1300
#define SERVER_ADDRESS  "10.0.0.100"
static int buf[1300];
static int sockfd;
static struct sockaddr_in sa;

static void setup(void)
{
    char *server_addr = SERVER_ADDRESS; 

    if (strlen(misc_arg)) {
        server_addr = misc_arg;
    }
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	if (sockfd == -1) {
		if (errno == EPROTONOSUPPORT)
			tst_brk(TCONF, "udp protocol was not supported");
		else
			tst_brk(TBROK | TERRNO, "socket() failed with udp");
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(server_addr);
	sa.sin_port = htons(5555);

    memset(buf, 'a', sizeof(buf));
}

static uint64_t timespec_to_ns(struct timespec* ts)
{
    return ts->tv_nsec + (ts->tv_sec * 1000000000LL); 
}

static void cleanup(void)
{
	if (sockfd > 0)
		SAFE_CLOSE(sockfd);
}

static void verify_sendto(void)
{
    struct timespec before = { 0 }, after = { 0 };
    uint64_t start, end, i = 0;

    SYSCALL_PERF_SET_CPU();
    start = SYSCALL_PERF_GET_TICKS();
    while(i++ < loop_count || loop_count < 0) {
	    TEST(sendto(sockfd, buf, 1300, 0, (struct sockaddr *) &sa, sizeof(sa)));

        if (TST_RET == -1) {
            tst_res(TFAIL | TTERRNO, "read(2) failed");
            break;
        }
    }
    end = SYSCALL_PERF_GET_TICKS();
    SYSCALL_PERF_MEASURE(start, end);
    tst_res(TPASS, "sendto returned %ld, %lld", TST_RET, i);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_sendto,
};
