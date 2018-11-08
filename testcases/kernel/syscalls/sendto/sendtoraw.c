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
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>

#include "tst_test.h"

#ifndef IPPROTO_UDP
# define IPPROTO_UDP	132
#endif

#define RANDOM_SIZE 8000000
#define PACKET_SIZE  1300
static int sockfd;

#define ETHERTYPE_IP  0x0800
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

struct udp {
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
};

struct udppkt {
    struct ether ether;
    struct ip ip;
    struct udp udp;
    char   data[PACKET_SIZE];
};

struct interface_info {
    char    raw_name[10];
    uint8_t dest_addr[4];
    uint8_t src_addr[4];
    uint8_t macaddr[HLEN_ETHER];
};

struct interface_info if_info = { 0 };

int raw_init_id = 100;
uint8_t def_dest_addr[4] = { 0x64, 0x00, 0x00, 0x64 }; /* 100.0.0.10 */
uint8_t def_src_addr[4] = { 0x0a, 0x00, 0x00, 0x12 }; /* 10.0.0.18 */
uint8_t def_macaddr[HLEN_ETHER] = {0x02, 0x01, 0x02, 0x03, 0x04, 0x11}; 
int8_t macaddr_brd[HLEN_ETHER] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void initialise_interface(int offset)
{
    sprintf(if_info.raw_name, "veth%d", offset + 1);
    
    memcpy(if_info.dest_addr, def_dest_addr, PLEN_IPV4);
    if_info.dest_addr[0] += offset;

    memcpy(if_info.src_addr, def_src_addr, PLEN_IPV4);
    if_info.src_addr[0] += offset;

    memcpy(if_info.macaddr, def_macaddr, HLEN_ETHER);
    if_info.macaddr[5] += offset;
    printf("Running test for raw: %s\n", if_info.raw_name);
}

static void setup(void)
{
    int raw_offset = 0;

    if (strlen(misc_arg)) {
        raw_offset = strtoumax(misc_arg, NULL, 10);
        printf("Creating interface for offset %d\n", raw_offset + 1);
    }

    initialise_interface(raw_offset);

    // Assume raw interface is created
    const char *ifname = if_info.raw_name;

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1)
		tst_brk(TBROK | TERRNO, "socket() with raw failed");

    struct sockaddr_ll veth_sock;
    memset(&veth_sock, 0, sizeof (veth_sock));
    veth_sock.sll_family = PF_PACKET;
    veth_sock.sll_ifindex = if_nametoindex(ifname);
    veth_sock.sll_protocol = htons(ETH_P_ALL);

	if (bind(sockfd, (struct sockaddr*)&veth_sock,
		sizeof(veth_sock)) < 0) {
		tst_brk(TBROK | TERRNO, "bind() with raw failed");
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

    struct udppkt p = { 0 };
    
    p.ip.version_ihl = 0x45;
    p.ip.type = 0x00;
    p.ip.proto = 0x11;
    p.ip.length = htons(20 + 8 + PACKET_SIZE); 
    memcpy(p.ip.src_ip, if_info.src_addr, PLEN_IPV4);
    memcpy(p.ip.dst_ip, if_info.dest_addr, PLEN_IPV4);

    p.udp.source = htons(5555);
    p.udp.dest = htons(5555);
    p.udp.len = htons(8 + PACKET_SIZE); 
    p.udp.check = 0;

    p.ip.checksum = 0;

    p.ether.type = htons(ETHERTYPE_IP);
    memcpy(p.ether.source, if_info.macaddr, HLEN_ETHER);
    memcpy(p.ether.target, macaddr_brd, HLEN_ETHER);
    memset(p.data, 'a', PACKET_SIZE);

    SYSCALL_PERF_SET_CPU();
    start = SYSCALL_PERF_GET_TICKS();
    while(i++ < loop_count || loop_count < 0) {
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
