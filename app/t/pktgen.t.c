/*
 * SOURCE: pktgen.c pktgen-ether.c pktgen-udp.c pktgen-ipv4.c pktgen-gre.c
 * STUB: rte_cycles.h rte_eal.h wr_copyright_info.h pktgen-random.h
 * STUB: pktgen-ether.h pktgen-gre.h pktgen-tcp.h pktgen-ipv4.h pktgen-udp.h
 * STUB: wr_cksum.h pktgen-ipv6.h pktgen-arp.h pktgen-vlan.h pktgen-range.h
 * STUB: pktgen-dump.h pktgen-capture.h pktgen-cpu.h pktgen-pcap.h pktgen-seq.h
 * STUB: pktgen-stats.h rte_timer.h pktgen-log.h pktgen-display.h
 */


/* wr_scrn.h fake functions */
#include <wr_scrn.h>
void wr_scrn_center(int16_t r, const char * fmt, ...) { return; }
void wr_scrn_printf(int16_t r, int16_t c, const char * fmt, ...) { return; }
wr_scrn_t *scrn = NULL;


/*
 * rte_ethdev.h fake functions
 */
void rte_eth_dev_stop(uint8_t port_id) { return; }
int rte_eth_dev_start(uint8_t port_id) { return 0; }


int rte_cycles_vmware_tsc_map;
__thread unsigned per_lcore__lcore_id;
struct rte_eth_dev rte_eth_devices[RTE_MAX_ETHPORTS];


/* Test function forward declarations */
void test_pktgen_packet_ctor_IPv4_UDP(void);

void test_pktgen_packet_ctor_IPv4_GRE_Ether(void);


/* Helper function forward declarations */


/***
 * Test driver
 ***/
int main(void) {
	test_pktgen_packet_ctor_IPv4_UDP();
	test_pktgen_packet_ctor_IPv4_GRE_Ether();

	done_testing();
	return 0;
}


/***
 * Test implementations
 ***/
void test_pktgen_packet_ctor_IPv4_UDP(void)
{
	port_info_t info;
	pkt_seq_t pkt;
	uint8_t *data;

	info.seq_pkt = &pkt;
	data = (uint8_t *)&pkt.hdr;

	pkt = (pkt_seq_t) {
		.eth_dst_addr = (struct ether_addr) { .addr_bytes = { 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd } },
		.eth_src_addr = (struct ether_addr) { .addr_bytes = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 } },

		.ip_src_addr  = (192 << 24) + (168 << 16) + (  0 << 8) + (1 << 0),
		.ip_dst_addr  = (192 << 24) + (168 << 16) + (  1 << 8) + (2 << 0),
		.ip_mask      = (255 << 24) + (255 << 16) + (255 << 8) + (0 << 0),

		.sport        = 3333,
		.dport        = 4444,
		.ethType      = ETHER_TYPE_IPv4,
		.ipProto      = PG_IPPROTO_UDP,

		.pktSize      = 60,		// Subtract 4 for FCS
	};


	lives_ok( { pktgen_packet_ctor(&info, 0, 0); }, "pktgen_packet_ctor must generate IPv4/UDP");

	note("... with Ethernet header");
	cmp_mem_lit_incr(data, (0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd), "    ... with correct dest MAC");
	cmp_mem_lit_incr(data, (0x55, 0x55, 0x55, 0x55, 0x55, 0x55), "    ... and correct src MAC");
	cmp_mem_lit_incr(data, (0x08, 0x00), "    ... and correct EtherType");

	note("... and with IP header");
	cmp_mem_lit_incr(data, (0x45), "    ... with correct version and IHL");
	cmp_mem_lit_incr(data, (0x00), "    ... and correct TOS");
	cmp_mem_lit_incr(data, (0x00, 0x2e), "    ... and correct total length");
	data += 2;		// Skip over Identification field
	cmp_mem_lit_incr(data, (0x00, 0x00), "    ... and no flags and fragment offset");
	cmp_mem_lit_incr(data, (0x04), "    ... and default TTL");
	cmp_mem_lit_incr(data, (PG_IPPROTO_UDP), "    ... and UDP payload protocol");
	data += 2;		// Skip over checksum
	cmp_mem_lit_incr(data, (192, 168, 0, 1), "    ... and correct src IP");
	cmp_mem_lit_incr(data, (192, 168, 1, 2), "    ... and correct dest IP");

	note("... and with UDP header");
	cmp_mem_lit_incr(data, (3333 / 256, 3333 % 256), "    ... with correct src port");
	cmp_mem_lit_incr(data, (4444 / 256, 4444 % 256), "    ... and correct dst port");
	cmp_mem_lit_incr(data, (0, 26), "    ... and correct length");
	data += 2;		// Skip over checksum

	// data points to start of payload
}


void test_pktgen_packet_ctor_IPv4_GRE_Ether(void)
{
	port_info_t info;
	pkt_seq_t pkt;
	uint8_t *data;

	info.seq_pkt = &pkt;
	data = (uint8_t *)&pkt.hdr;

	pktgen_set_port_flags(&info, SEND_GRE_ETHER_HEADER);

	pkt = (pkt_seq_t) {
		.eth_dst_addr = (struct ether_addr) { .addr_bytes = { 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd } },
		.eth_src_addr = (struct ether_addr) { .addr_bytes = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 } },

		.ip_src_addr  = (192 << 24) + (168 << 16) + (  0 << 8) + (1 << 0),
		.ip_dst_addr  = (192 << 24) + (168 << 16) + (  1 << 8) + (2 << 0),
		.ip_mask      = (255 << 24) + (255 << 16) + (255 << 8) + (0 << 0),

		.sport        = 3333,
		.dport        = 4444,
		.ethType      = ETHER_TYPE_IPv4,
		.ipProto      = PG_IPPROTO_UDP,

		.pktSize      = 102,		// Subtract 4 for FCS
	};

	pktgen_packet_ctor(&info, 0, 0);

	lives_ok( { pktgen_packet_ctor(&info, 0, 0); }, "pktgen_packet_ctor must generate IPv4 GRE Ethernet frame");

	note("... with outer Ethernet header");
	cmp_mem_lit_incr(data, (0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd), "    ... with correct dest MAC");
	cmp_mem_lit_incr(data, (0x55, 0x55, 0x55, 0x55, 0x55, 0x55), "    ... and correct src MAC");
	cmp_mem_lit_incr(data, (0x08, 0x00), "    ... and correct EtherType");

	note("... and with outer IP header");
	cmp_mem_lit_incr(data, (0x45), "    ... with correct version and IHL");
	cmp_mem_lit_incr(data, (0x00), "    ... and correct TOS");
	cmp_mem_lit_incr(data, (0x00, 0x58), "    ... and correct total length");
	data += 2;		// Skip over Identification field
	cmp_mem_lit_incr(data, (0x00, 0x00), "    ... and no flags and fragment offset");
	cmp_mem_lit_incr(data, (0x40), "    ... and default TTL");
	cmp_mem_lit_incr(data, (PG_IPPROTO_GRE), "    ... and GRE payload protocol");
	data += 2;		// Skip over checksum
	cmp_mem_lit_incr(data, (10, 10, 1, 1), "    ... and correct src IP");
	cmp_mem_lit_incr(data, (10, 10, 1, 2), "    ... and correct dest IP");

	note("... and with GRE header");
	cmp_mem_lit_incr(data, (0x20, 0x00), "    ... with correct flags and version");
	cmp_mem_lit_incr(data, (0x65, 0x58), "    ... and correct protocol type");
	data += 4;		// TODO check flags for checksum, key, seq. nr. and test accordingly

	note("... with inner Ethernet header");
	cmp_mem_lit_incr(data, (0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd), "    ... with correct dest MAC");
	cmp_mem_lit_incr(data, (0x55, 0x55, 0x55, 0x55, 0x55, 0x55), "    ... and correct src MAC");
	cmp_mem_lit_incr(data, (0x08, 0x00), "    ... and correct EtherType");

	note("... and with inner IP header");
	cmp_mem_lit_incr(data, (0x45), "    ... with correct version and IHL");
	cmp_mem_lit_incr(data, (0x00), "    ... and correct TOS");
	cmp_mem_lit_incr(data, (0x00, 0x2e), "    ... and correct total length");
	data += 2;		// Skip over Identification field
	cmp_mem_lit_incr(data, (0x00, 0x00), "    ... and no flags and fragment offset");
	cmp_mem_lit_incr(data, (0x04), "    ... and default TTL");
	cmp_mem_lit_incr(data, (PG_IPPROTO_UDP), "    ... and UDP payload protocol");
	data += 2;		// Skip over checksum
	cmp_mem_lit_incr(data, (192, 168, 0, 1), "    ... and correct src IP");
	cmp_mem_lit_incr(data, (192, 168, 1, 2), "    ... and correct dest IP");

	note("... and with UDP header");
	cmp_mem_lit_incr(data, (3333 / 256, 3333 % 256), "    ... with correct src port");
	cmp_mem_lit_incr(data, (4444 / 256, 4444 % 256), "    ... and correct dst port");
	cmp_mem_lit_incr(data, (0, 26), "    ... and correct length");
	data += 2;		// Skip over checksum

	// data points to start of payload
}
