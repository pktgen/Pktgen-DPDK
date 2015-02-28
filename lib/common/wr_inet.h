/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef __WR_INET_H
#define __WR_INET_H

#define IPv4_VERSION    4
#define IPv6_VERSION	6

#ifdef RTE_MACHINE_CPUFLAG_SSE4_2
#include <nmmintrin.h>
#else
#include <rte_jhash.h>
#endif
#include <stdio.h>
#include <stdint.h>

#include <rte_ether.h>


/* Internet protocol header structure */
/* Basic IPv4 packet header
 *
 *                        IPv4 Header Format
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   | Ver   | hlen  |      TOS      |         Total length          |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |         Ident                 |flags|     fragment offset     |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |    TTL        |    Protocol   |       Header Checksum         |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    Source Address (32  Bits)                  |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                 Destination Address (32  Bits)                |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                             data                              |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct ipHdr_s {
    uint8_t        vl;			/* Version and header length */
    uint8_t        tos;         /* Type of Service */
    uint16_t       tlen;        /* total length */
    uint16_t       ident;       /* identification */
    uint16_t       ffrag;       /* Flags and Fragment offset */
    uint8_t        ttl;         /* Time to Live */
    uint8_t        proto;       /* Protocol */
    uint16_t       cksum;       /* Header checksum */
    uint32_t       src;         /* source IP address */
    uint32_t       dst;         /* destination IP address */
} __attribute__((__packed__)) ipHdr_t;

#define PG_ISFRAG(off)  ((off) & (PG_OFF_MF | PG_OFF_MASK))
#define PG_OFF_MASK     0x1fff
#define PG_OFF_MF       0x2000
#define PG_OFF_DF       0x4000

/* Basic IPv6 packet header
 *
 *                        IPv6 Header Format
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   | Ver   | Traffic Class |            Flow Label                 |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |         Payload Length        |  Next Header  |   Hop Limit   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   |                    Source Address (128 Bits)                  |
 *   |                                                               |
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   |                 Destination Address (128 Bits)                |
 *   |                                                               |
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                             data                              |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct ipv6Hdr_s {
	uint32_t		ver_tc_fl;
	uint16_t		payload_length;
	uint8_t			next_header;
	uint8_t			hop_limit;
	uint8_t			saddr[16];
	uint8_t			daddr[16];
} __attribute__ ((__packed__)) ipv6Hdr_t;

/******************************************************************************
 * ipHdr_t.proto values in the IP Header.
 *  1     ICMP        Internet Control Message            [RFC792]
 *  2     IGMP        Internet Group Management          [RFC1112]
 *  4     IP          IP in IP (encapsulation)           [RFC2003]
 *  6     TCP         Transmission Control                [RFC793]
 * 17     UDP         User Datagram                   [RFC768,JBP]
 * 41     IPv6        Ipv6                               [Deering]
 * 43     IPv6-Route  Routing Header for IPv6            [Deering]
 * 44     IPv6-Frag   Fragment Header for IPv6           [Deering]
 * 47     GRE         Generic Routing Encapsulation [RFC2784,2890]
 * 58     IPv6-ICMP   ICMP for IPv6                      [RFC1883]
 * 59     IPv6-NoNxt  No Next Header for IPv6            [RFC1883]
 * 60     IPv6-Opts   Destination Options for IPv6       [RFC1883]
 */
#define PG_IPPROTO_NONE         0
#define PG_IPPROTO_IP           IPPROTO_IP
#define PG_IPPROTO_ICMP         IPPROTO_ICMP
#define PG_IPPROTO_IGMP         IPPROTO_IGMP
#define PG_IPPROTO_IPV4         IPPROTO_IPV4
#define PG_IPPROTO_TCP          IPPROTO_TCP
#define PG_IPPROTO_UDP          IPPROTO_UDP
#define PG_IPPROTO_IPV6         IPPROTO_IPV6
#define PG_IPPROTO_IPV6_ROUTE   43
#define PG_IPPROTO_IPV6_FRAG    44
#define PG_IPPROTO_GRE          IPPROTO_GRE
#define PG_IPPROTO_ICMPV6       IPPROTO_ICMPV6
#define PG_IPPROTO_IPV6_ICMP    IPPROTO_ICMPV6
#define PG_IPPROTO_IPV6_NONXT   59
#define PG_IPPROTO_IPV6_OPTS    60
#define PG_IPPROTO_RAW          IPPROTO_RAW
#define PG_IPPROTO_MAX          256

#define IPv4(a,b,c,d)	((uint32_t)(((a) & 0xff) << 24) |	\
				(((b) & 0xff) << 16) |		\
				(((c) & 0xff) << 8)  |		\
				((d) & 0xff))

/* IP overlay header for the pseudo header */
typedef struct ipOverlay_s {
    uint32_t    node[2];
    uint8_t     pad0;			/* overlays ttl */
    uint8_t     proto;          /* Protocol type */
    uint16_t    len;            /* Protocol length, overlays cksum */
    uint32_t    src;            /* Source address */
    uint32_t    dst;            /* Destination address */
} __attribute__((__packed__)) ipOverlay_t;

typedef struct ipv6Overlay_s {
	uint8_t		saddr[16];
	uint8_t		daddr[16];
	uint32_t	tcp_length;
	uint16_t	zero;
	uint8_t		pad;
	uint8_t		next_header;
} __attribute__((__packed__)) ipv6Overlay_t;

typedef unsigned int	seq_t;  /* TCP Sequence type */

/* UDP Header */
typedef struct udpHdr_s {
    uint16_t       sport;		/* Source port value */
    uint16_t       dport;       /* Destination port value */
    uint16_t       len;         /* Length of datagram + header */
    uint16_t       cksum;       /* Checksum of data and header */
} __attribute__((__packed__)) udpHdr_t;

/* The UDP/IP Pseudo header */
typedef struct udpip_s {
    ipOverlay_t		ip;         /* IPv4 overlay header */
    udpHdr_t		udp;        /* UDP header for protocol */
} __attribute__((__packed__)) udpip_t;

/* The UDP/IPv6 Pseudo header */
typedef struct udpipv6_s {
    ipv6Overlay_t	ip;         /* IPv6 overlay header */
    udpHdr_t		udp;        /* UDP header for protocol */
} __attribute__((__packed__)) udpipv6_t;

/* Basic TCP packet header
 *
 *                        TCP Header Format
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |          Source Port          |       Destination Port        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                        Sequence Number                        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    Acknowledgment Number                      |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |  Data |           |U|A|P|R|S|F|                               |
 *   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
 *   |       |           |G|K|H|T|N|N|                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |           Checksum            |         Urgent Pointer        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    Options                    |    Padding    |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                             data                              |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct tcpHdr_s {
    uint16_t       sport;		/* Source port address */
    uint16_t       dport;       /* Destination port address */
    seq_t      	   seq;         /* Sequence number */
    seq_t      	   ack;         /* ACK number */
    uint8_t        offset;      /* Offset */
    uint8_t        flags;       /* Control flags */
    uint16_t       window;      /* Window value */
    uint16_t       cksum;       /* Checksum */
    uint16_t       urgent;      /* Urgent pointer or offset */
    uint8_t        opts[0];     /* Place holder for options */
} __attribute__((__packed__)) tcpHdr_t;

enum { URG_FLAG = 0x20, ACK_FLAG = 0x10, PSH_FLAG = 0x08, RST_FLAG = 0x04, SYN_FLAG = 0x02, FIN_FLAG = 0x01 };

/* The TCP/IPv4 Pseudo header */
typedef struct tcpip_s {
    ipOverlay_t		ip;         /* IPv4 overlay header */
    tcpHdr_t    	tcp;        /* TCP header for protocol */
} __attribute__((__packed__)) tcpip_t;

/* The TCP/IPv6 Pseudo header */
typedef struct tcpipv6_s {
    ipv6Overlay_t	ip;         /* IPv6 overlay header */
    tcpHdr_t    	tcp;        /* TCP header for protocol */
} __attribute__((__packed__)) tcpipv6_t;

/* ICMPv4 Packet structure */
typedef struct icmpv4Hdr_s {
    uint8_t        type;                	/* Message Type */
    uint8_t        code;                	/* Code value */
    uint16_t       cksum;               	/* checksum */

    union {
        struct {
            uint16_t    ident;          	/* Identifier */
            uint16_t    seq;            	/* Sequence Number */
            uint32_t    data;           	/* Data for sequence number */
        } echo;

        struct {
            uint32_t    gateway;        	/* Gateway Address */
            uint16_t    ip[10];         	/* IP information */
            uint16_t    transport[4];   	/* Transport information */
        } redirect;

        struct {
            uint32_t    nextHopMtu;     	/* Only if code NEED_FRAG or unused */
            uint16_t    ip[10];         	/* IP information */
            uint16_t    transport[4];		/* Transport information */
        } failing_pkt;

        struct {
            uint16_t    ident;             	/* Identifier */
            uint16_t    seq;               	/* Sequence Number */
            uint32_t    originate;         	/* originate time */
            uint32_t    receive;           	/* receive time */
            uint32_t    transmit;          	/* transmit time */
        } timestamp;

        struct {
            uint32_t    multicast;         	/* Multicast information */
            uint8_t     s_qrv;             	/* s_grv value */
            uint8_t     qqic;              	/* qqoc value */
            uint16_t    numberOfSources;   	/* number of Source routes */
            uint16_t    source_addr[1];    	/* source address */
        } igmp;

        struct {
            uint8_t     pointer;           	/* Parameter pointer */
            uint8_t     unused[3];         	/* Not used */
        } param;                           	/* Parameter Problem */

        struct {
            uint8_t     numAddrs;          	/* Number of address */
            uint8_t     addrEntrySize;     	/* Size of each entry */
            uint16_t    lifetime;          	/* lifetime value */
            uint32_t    advert[1];         	/* advertized values */
        } advertise;

        /* Address mask */
        struct {
            uint16_t    ident;             	/* Identifier */
            uint16_t    seq;               	/* Sequence Number */
            uint32_t    dmask;             	/* Mask data */
        } mask;
    } data;
} __attribute__((__packed__)) icmpv4Hdr_t;

#define ICMP4_TIMESTAMP_SIZE		12

/* ICMPv4 Message Types */
#define ICMP4_ECHO_REPLY            0
#define ICMP4_DEST_UNREACHABLE      3
#define ICMP4_SOURCE_QUENCH         4
#define ICMP4_REDIRECT              5
#define ICMP4_ECHO                  8
#define ICMP4_ROUTER_ADVERT         9
#define ICMP4_ROUTER_SOLICIT        10
#define ICMP4_TIME_EXCEEDED         11
#define ICMP4_PARAMETER_PROBLEM     12
#define ICMP4_TIMESTAMP             13
#define ICMP4_TIMESTAMP_REPLY       14
#define ICMP4_INFO_REQUEST          15
#define ICMP4_INFO_REPLY            16
#define ICMP4_MASK_REQUEST          17
#define ICMP4_MASK_REPLY            18


/* MPLS header
 *                        MPLS Header Format
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                 Label                 | EXP |S|     TTL       |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Label: 20-bit label value
 * EXP: Experimental (QoS and ECN)
 *   - 3-bit Traffic Class field for QoS priority
 *   - Explicit Congestion Notification
 * S: Bottom-of-Stack. If set, the current label is the last in the stack.
 * TTL: Time-to-Live
 */

/* Set/clear Bottom of Stack flag */
#define MPLS_SET_BOS(mpls_label) do { mpls_label |=  (1 << 8); } while (0);
#define MPLS_CLR_BOS(mpls_label) do { mpls_label &= ~(1 << 8); } while (0);

typedef struct mplsHdr_s {
	uint32_t label;				/**< MPLS label */
} __attribute__ ((__packed__)) mplsHdr_t;


/* Q-in-Q (802.1ad) header
 *                      Q-in-Q Header Format
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |              0x88A8           | PCP |D|         VID           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |              0x8100           | PCP |D|         VID           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * 0x88A8, 0x8100: EtherType associated with the tag. 0x88A8 means outer VLAN
 *   tag, 0x8100 means inner VLAN tag.
 * PCP: Priority code point. Class of Service indicator
 * D: Drop eligible indicator
 * VID: VLAN identifier
 */
typedef struct qinqHdr_s {
	uint16_t qinq_tci;		/**< Outer tag PCP, DEI, VID */
	uint16_t vlan_tpid;		/**< Must be ETHER_TYPE_VLAN (0x8100) */
	uint16_t vlan_tci;		/**< Inner tag PCP, DEI, VID */
	uint16_t eth_proto;		/**< EtherType of encapsulated frame */
} __attribute__ ((__packed__)) qinqHdr_t;


/* GRE header
 *
 *                      GRE Header Format
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |C| |K|S|    Reserved0    | Ver |         Protocol Type         |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |      Checksum (optional)      |     Reserved1 (optional)      |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                         Key (optional)                        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                  Sequence Number (optional)                   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * C: 1 if the Checksum and the Reserved1 fields are present and the Checksum
 *    field contains valid information
 * K: 1 if the Key field is present
 * S: the Sequence Number field is present
 * Reserved0: must be 0
 * Ver: Version Number, must be 0
 * Protocol Type: EtherType of encapsulated packet. When IPv4 is being carried
 *   as the GRE payload, the Protocol Type field MUST be set to 0x800.
 * Checksum: the IP (one's complement) checksum sum of all the 16 bit words in
 *   the GRE header and the payload packet
 * Reserved1: must be 0
 * Key: 32bit number determined by the encapsulator
 * Sequence Number: for packet ordering purposes
 */

/* GRE header */
typedef struct greHdr_s {
	uint8_t		reserved0_0 : 4;	/**< must be 0 */
	uint8_t		seq_present : 1;	/**< Sequence Number present */
	uint8_t		key_present : 1;	/**< Key present */
	uint8_t		unused      : 1;
	uint8_t		chk_present : 1;	/**< Checksum and Reserved1 present */
	uint8_t		version     : 3;	/**< Version Number */
	uint8_t		reserved0_1 : 5;	/**< must be 0 */
	uint16_t	eth_type;			/**< EtherType of encapsulated packet */
	uint32_t	extra_fields[3];	/**< Room for Checksum+Reserved1, Key and Sequence Number fields if present */
} __attribute__ ((__packed__)) greHdr_t;

/* the GRE/IPv4 header */
typedef struct greIp_s {
	ipHdr_t		ip;		/* Outer IPv4 header */
	greHdr_t	gre;	/* GRE header for protocol */
} __attribute__ ((__packed__)) greIp_t;

/* the GRE/Ethernet header */
typedef struct greEther_s {
	ipHdr_t				ip;				/* Outer IPv4 header */
	greHdr_t			gre;			/* GRE header */
	struct ether_hdr	ether;			/* Inner Ethernet header */
} __attribute__ ((__packed__)) greEther_t;


/* Common defines for Ethernet */
#define ETH_HW_TYPE					1		/* Ethernet hardware type */
#define ETH_HDR_SIZE				14      /* Ethernet MAC header length */
#define ETH_ADDR_SIZE				6       /* Ethernet MAC address length */
#define ETH_MTU						1500    /* Max MTU for Ethernet */
#define ETH_MAX_PKT					1518    /* Max Ethernet frame size */
#define ETH_MIN_PKT					60      /* Min frame size minus CRC */
#define IPV6_ADDR_LEN				16		/* IPv6 Address length */

#define ETH_VLAN_ENCAP_LEN     		4       /* 802.1Q VLAN encap. length */


/* Extra EtherTypes */
#define ETHER_TYPE_MPLS_UNICAST		0x8847
#define ETHER_TYPE_MPLS_MULTICAST	0x8848

#define ETHER_TYPE_Q_IN_Q			0x88A8
#define ETHER_TYPE_TRANSP_ETH_BR	0x6558	/* Transparent Ethernet Bridge */


/* RARP and ARP opcodes */
enum {	ARP_REQUEST = 1, ARP_REPLY = 2, RARP_REQUEST = 3, RARP_REPLY = 4, GRATUITOUS_ARP = 5 };

typedef union {
	uint16_t	_16[3];
	uint8_t		_8[6];
} mac_e;

typedef union {
	uint16_t	_16[2];
	uint32_t	_32;
} ip4_e;

/* ARP packet format */
typedef struct arpPkt_s {
    uint16_t       hrd;            	/**< ARP Hardware type */
    uint16_t       pro;            	/**< ARP Protocol type */
    uint8_t        hln;            	/**< Hardware length */
    uint8_t        pln;            	/**< Protocol length */
    uint16_t       op;             	/**< opcode */
    mac_e	       sha;            	/**< Sender hardware address */
    ip4_e     	   spa;            	/**< Sender IP address */
    mac_e          tha;            	/**< Target hardware address */
    ip4_e          tpa;           	/**< Target protocol address */
} __attribute__((__packed__)) arpPkt_t;

typedef struct pkt_hdr_s {
	struct ether_hdr    eth;		/**< Ethernet header */
	union {
		ipHdr_t			ipv4;		/**< IPv4 Header */
		ipv6Hdr_t		ipv6;		/**< IPv6 Header */
		tcpip_t			tip;		/**< TCP + IPv4 Headers */
		udpip_t			uip;		/**< UDP + IPv4 Headers */
		icmpv4Hdr_t		icmp;		/**< ICMP + IPv4 Headers */
		tcpipv6_t		tip6;		/**< TCP + IPv6 Headers */
		udpipv6_t		uip6;		/**< UDP + IPv6 Headers */
		uint64_t		pad[8];		/**< Length of structures */
	} u;
} __attribute__((__packed__)) pkt_hdr_t;

typedef struct ipv4_5tuple {
	uint32_t		ip_dst;
	uint32_t		ip_src;
	uint16_t		port_dst;
	uint16_t		port_src;
	uint8_t			proto;
} __attribute__((__packed__)) ipv4_5tuple_t;

typedef struct l3_4route_s {
	ipv4_5tuple_t	key;
	uint8_t			ifid;
} __attribute__ ((packed)) l3_4route_t;

typedef struct ipv6_5tuple_s {
	uint8_t			dst[IPV6_ADDR_LEN];
	uint8_t			src[IPV6_ADDR_LEN];
	uint16_t		sport;
	uint16_t		dport;
	uint8_t			proto;
} __attribute__ ((packed)) ipv6_5tuple_t;

typedef struct l3_6route_s {
	ipv6_5tuple_t	key;
	uint8_t			ifid;
} __attribute__ ((packed)) l3_6route_t;

/*********************************************************************************/
/**
 * Use crc32 instruction to perform a 6 byte hash.
 *
 * @param data
 *   Data to perform hash on.
 * @param data_len
 *   How many bytes to use to calculate hash value. (Not Used)
 * @param init_val
 *   Value to initialize hash generator.
 * @return
 *   32bit calculated hash value.
 */
static inline uint32_t
rte_hash6_crc(const void *data, __attribute__ ((unused)) uint32_t data_len, uint32_t init_val)
{
#ifdef RTE_MACHINE_CPUFLAG_SSE4_2
	const uint32_t *p32 = (const uint32_t *)data;
	const uint16_t	val = *(const uint16_t *)p32;

	return _mm_crc32_u32(val, _mm_crc32_u32(*p32++, init_val));
#else
    return rte_jhash(data, data_len, init_val);
#endif
}

/* ethAddrCopy( u16_t * to, u16_t * from ) - Swap two Ethernet addresses */
static __inline__ void ethAddrCopy( void * t, void * f ) {
	uint16_t	* d = (uint16_t *)t;
	uint16_t	* s = (uint16_t *)f;
	*d++ = *s++; *d++ = *s++; *d = *s;
}

/* ethSwap(u16_t * to, u16_t * from) - Swap two 16 bit values */
static __inline__ void uint16Swap( void * t, void * f ) {
	uint16_t * d = (uint16_t *)t;
	uint16_t * s = (uint16_t *)f;
    uint16_t   v;
    v = *d; *d = *s; *s = v;
}

/* ethAddrSwap( u16_t * to, u16_t * from ) - Swap two ethernet addresses */
static __inline__ void ethAddrSwap( void * t, void * f ) {
	uint16_t	* d = (uint16_t *)t;
	uint16_t	* s = (uint16_t *)f;
    uint16Swap( d++, s++ );
    uint16Swap( d++, s++ );
    uint16Swap( d, s );
}

/* inetAddrCopy( void * t, void * f ) - Copy IPv4 address */
static __inline__ void inetAddrCopy( void * t, void * f ) {
	uint32_t * d = (uint32_t *)t;
	uint32_t * s = (uint32_t *)f;
	*d = *s;
}

/* inetAddrSwap( void * t, void * f ) - Swap two IPv4 addresses */
static __inline__ void inetAddrSwap( void * t, void * f ) {
	uint32_t * d = (uint32_t *)t;
	uint32_t * s = (uint32_t *)f;
	uint32_t   v;
	v  = *d; *d = *s; *s = v;
}

/* mask_size(uint32_t mask) - return the number of bits in mask */
static __inline__ int mask_size(uint32_t mask) {
	if ( mask == 0 )
		return 0;
	else if ( mask == 0xFF000000 )
		return 8;
	else if ( mask == 0xFFFF0000 )
		return 16;
	else if ( mask == 0xFFFFFF00 )
		return 24;
	else if ( mask == 0xFFFFFFFF )
		return 32;
	else {
		int		i;
		for(i = 0; i < 32; i++) {
			if ( (mask & (1 << (31 - i))) == 0 )
				break;
		}
		return i;
	}
}

/* size_to_mask( int len ) - return the mask for the mask size */
static __inline__ uint32_t size_to_mask(int len) {
	uint32_t	mask = 0;
	if ( len == 0 )
		mask = 0x00000000;
	else if ( len == 8 )
		mask = 0xFF000000;
	else if ( len == 16 )
		mask = 0xFFFF0000;
	else if ( len == 24 )
		mask = 0xFFFFFF00;
	else if ( len == 32 )
		mask = 0xFFFFFFFF;
	else {
		int		i;

		for(i=0; i < len; i++)
			mask |= (1 << (31 - i));
	}
	return mask;
}

/* char * inet_ntop4(char * buff, int len, unsigned long ip_addr, unsigned long mask) - Convert IPv4 address to ascii */
static __inline__ char * inet_ntop4(char * buff, int len, unsigned long ip_addr, unsigned long mask) {
	if ( mask == 0xFFFFFFFF )
		snprintf(buff, len, "%lu.%lu.%lu.%lu",
				((ip_addr >> 0) & 0xFF), ((ip_addr >> 8) & 0xFF),
				((ip_addr >> 16) & 0xFF), ((ip_addr >> 24) & 0xFF));
	else
		snprintf(buff, len, "%lu.%lu.%lu.%lu/%d",
				((ip_addr >> 0) & 0xFF), ((ip_addr >> 8) & 0xFF),
				((ip_addr >> 16) & 0xFF), ((ip_addr >> 24) & 0xFF),
				mask_size(mask));
	return buff;
}

/* char * inet_mtoa(char * buff, int len, struct ether_addr * eaddr) - Convert MAC address to ascii */
static __inline__ char * inet_mtoa(char * buff, int len, struct ether_addr * eaddr) {
	snprintf(buff, len, "%02x:%02x:%02x:%02x:%02x:%02x",
			eaddr->addr_bytes[0], eaddr->addr_bytes[1],
			eaddr->addr_bytes[2], eaddr->addr_bytes[3],
			eaddr->addr_bytes[4], eaddr->addr_bytes[5]);
	return buff;
}

/* convert a MAC address from network byte order to host 64bit number */
static __inline__ uint64_t inet_mtoh64(struct ether_addr * eaddr, uint64_t * value) {
    *value = ((uint64_t)eaddr->addr_bytes[5] << 0)
           + ((uint64_t)eaddr->addr_bytes[4] << 8)
           + ((uint64_t)eaddr->addr_bytes[3] << 16)
           + ((uint64_t)eaddr->addr_bytes[2] << 24)
           + ((uint64_t)eaddr->addr_bytes[1] << 32)
           + ((uint64_t)eaddr->addr_bytes[0] << 40);
    return *value;
}

/* convert a host 64bit number to MAC address in network byte order */
static __inline__ struct ether_addr * inet_h64tom(uint64_t value, struct ether_addr * eaddr) {
    eaddr->addr_bytes[5] = ((value >> 0) & 0xFF);
    eaddr->addr_bytes[4] = ((value >> 8) & 0xFF);
    eaddr->addr_bytes[3] = ((value >> 16) & 0xFF);
    eaddr->addr_bytes[2] = ((value >> 24) & 0xFF);
    eaddr->addr_bytes[1] = ((value >> 32) & 0xFF);
    eaddr->addr_bytes[0] = ((value >> 40) & 0xFF);
    return eaddr;
}

#endif /* __WR_INET_H */
