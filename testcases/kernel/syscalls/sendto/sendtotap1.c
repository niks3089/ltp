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
#include <fcntl.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "tst_test.h"

#ifndef IPPROTO_UDP
# define IPPROTO_UDP	132
#endif

#define RANDOM_SIZE 8000000
#define PACKET_SIZE  1300
static int sockfd;

#define ETHERTYPE_IP  0x0800
#define ETHERTYPE_ARP 0x0806
#define HLEN_ETHER  6
#define PLEN_IPV4  4

struct ether {
    uint8_t target[HLEN_ETHER];
    uint8_t source[HLEN_ETHER];
    uint16_t type;
};

struct ip {
    uint8_t version_ihl;
    uint8_t type;
    uint16_t length;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t proto;
    uint16_t checksum;
    uint8_t src_ip[PLEN_IPV4];
    uint8_t dst_ip[PLEN_IPV4];
};

struct ping {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seqnum;
    uint8_t data[0];
};

struct pingpkt {
    struct ether ether;
    struct ip ip;
    struct ping ping;
};

uint8_t ipaddr[4] = { 0x0a, 0x00, 0x00, 0x22 }; /* 10.0.0.2 */
uint8_t srcaddr[4] = { 0x0a, 0x00, 0x00, 0x12 }; /* 10.0.0.2 */
uint8_t macaddr[HLEN_ETHER] = {0x02, 0x01, 0x02, 0x03, 0x04, 0x08}; // Hardcode this based on tap100
uint8_t macaddr_brd[HLEN_ETHER] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static void setup(void)
{
    // Assume tap interface is created
    const char *ifname = "tap100";
    int err;
    struct ifreq ifr;

    sockfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
    if (sockfd == -1)
		tst_brk(TBROK | TERRNO, "socket() with tap failed");

    /*
     * Initialise ifr for TAP interface.
     */
    memset(&ifr, 0, sizeof(ifr));
    /*
     * TODO: IFF_NO_PI may silently truncate packets on read().
     */
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (strlen(ifname) > IFNAMSIZ) {
		tst_brk(TCONF, "socket() with tap failed");
    }
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    /*
     * Attach to the tap device; we have already verified that it exists, but
     * see below.
     */
    if (ioctl(sockfd, TUNSETIFF, (void *)&ifr) == -1) {
        err = errno;
        close(sockfd);
		tst_brk(TCONF, "socket() with tap failed");
    }
    /*
     * If we got back a different device than the one requested, e.g. because
     * the caller mistakenly passed in '%d' (yes, that's really in the Linux
     * API) then fail.
     */
    if (strncmp(ifr.ifr_name, ifname, IFNAMSIZ) != 0) {
		tst_brk(TCONF, "socket() with tap failed");
    }
}

static void cleanup(void)
{
	if (sockfd > 0)
		SAFE_CLOSE(sockfd);
}

static void verify_sendto(void)
{
    uint64_t start, end, i = 0;

    struct pingpkt p = { 0 };
    
    p.ip.version_ihl = 0x45;
    p.ip.type = 0x00;
    p.ip.proto = 0x01;
    memcpy(p.ip.src_ip, srcaddr, PLEN_IPV4);
    memcpy(p.ip.dst_ip, ipaddr, PLEN_IPV4);

    p.ping.type = 0x08;
    p.ping.code = 0x00;
    p.ip.checksum = 0;

    p.ether.type = htons(ETHERTYPE_IP);
    memcpy(p.ether.source, macaddr, HLEN_ETHER);
    memcpy(p.ether.target, macaddr_brd, HLEN_ETHER);

    SYSCALL_PERF_SET_CPU();
    start = SYSCALL_PERF_GET_TICKS();
    while(i++ < 10000000) {
        if (write(sockfd, (uint8_t *)&p, sizeof p) < 0) {
            printf("Could not send Ping packet\n");
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
