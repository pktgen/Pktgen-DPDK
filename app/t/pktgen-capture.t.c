/*
 * SOURCE: pktgen-capture.c
 * STUB: pktgen-log.h
 * LIBS:
 * SYSLIBS: pcap
 */

#include <rte_mbuf.h>
#include <rte_memzone.h>

#include <string.h>

#include "pktgen-cmds.h"

/* pktgen global variable stub */
pktgen_t pktgen;


/* wr_scrn.h function stub */
void wr_scrn_fprintf(int16_t r, int16_t c, FILE * f, const char * fmt, ...) { return; }


/* Mock data structures */
capture_t _c;

char _mz_pool[1024];
struct rte_memzone _mz;

struct rte_mbuf _mb;
struct rte_mbuf *_mb_p;

port_info_t _info;


/* Mock functions called by capture code */
const struct rte_memzone* rte_memzone_reserve (const char * name, size_t len, int socket_id, unsigned flags)
{
	strncpy(_mz.name, name, sizeof(_mz.name));
	_mz.name[sizeof(_mz.name) - 1] = '\0';

	_mz.phys_addr   = (phys_addr_t) &_mz_pool;
	_mz.len         = len ? len : sizeof(_mz_pool);
	_mz.hugepage_sz = 0;
	_mz.socket_id   = socket_id;
	_mz.flags       = flags;
	_mz.addr        = &_mz_pool;

	return &_mz;
}


/*
 * Test environment
 */
void test_setup(void)
{
	ZERO(pktgen);
	ZERO(_c);
	ZERO(_mz_pool);
	ZERO(_mz);

	ZERO(_mb);
	_mb_p = &_mb;

	ZERO(_info);
}

/*
 * Tests
 */
void test_pktgen_packet_capture_init(void)
{
	lives_ok({ pktgen_packet_capture_init(NULL, 0); },
			"pktgen_packet_capture_init() must live when called with NULL parameter");

	// Distinct socket ID
	const int32_t socket_id = 5;

	lives_ok({ pktgen_packet_capture_init(&_c, socket_id); },
			"pktgen_packet_capture_init() must live when called with a capture_t* parameter");
	cmp_ok(_c.lcore, ">", 0, "... and set lcore member");
	cmp_ok(_c.port,  ">", 0, "... and set port member");
	cmp_ok(_c.mem_used, "==", 0, "... and set no memory used");
	ok(_c.mz != NULL, "... and initialize memzone");
	is(_c.mz->name, "Capture_MZ_5", "    ... with a descriptive name");
	cmp_ok(_c.mz->len, "==", sizeof(_mz_pool), "    ... and a reasonably sized memory pool");
	cmp_ok(_c.mz->socket_id, "==", socket_id, "    ... on the requested socket");
	ok(_c.mz->flags & RTE_MEMZONE_1GB, "    ... and preferring 1GB hugepages");
	ok(_c.mz->flags & RTE_MEMZONE_SIZE_HINT_ONLY, "    ... but allowing fallback to available page size");
}

void test_pktgen_set_capture(void)
{
	lives_ok({ pktgen_set_capture(&_info, DISABLE_STATE); }, "pktgen_set_capture() must handle disabling a disabled capture");

	//lives_ok({ pktgen_set_capture(&_info, ENABLE_STATE); }, "pktgen_set_capture() must handle enabling a disabled capture");
}

void test_pktgen_packet_capture_bulk(void)
{
	int nb_dump = 0;

	lives_ok({ pktgen_packet_capture_bulk(&_mb_p, nb_dump, &_c); },
			"pktgen_packet_capture_bulk() must not crash with real parameters");

}


/* Test driver */
int main(void) {
	//plan(1);

	test_setup();

	test_pktgen_packet_capture_init();
	test_pktgen_set_capture();
	test_pktgen_packet_capture_bulk();



	done_testing();
	return 0;
}
