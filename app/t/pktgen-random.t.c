/*
 * SOURCE: pktgen-random.c
 * STUB: rte_malloc.h wr_copyright_info.h pktgen-log.h
 * STUB: pktgen-display.h
 *
 * LIBS:
 * SYSLIBS:
 */

#include <arpa/inet.h>


/* wr_scrn.h function stub */
void wr_scrn_printf(int16_t r, int16_t c, const char * fmt, ...) { return; }
void wr_scrn_center(int16_t r, const char * fmt, ...) { return; }


/* Test fixtures */
BITFIELD_T all_bits_0(void) {
	return 0;
}

BITFIELD_T all_bits_1(void) {
	return ~(BITFIELD_T)0;
}


/*
 * Helper functions and macro's
 */


/* Mimic functions from rte_mbuf.h:
 *	rte_pktmbuf_alloc()
 *	rte_pktmbuf_reset()
 * The functions from librte_mbuf.a can't be used here, because they require
 * a fully initialized EAL (at least the hugepages part).
 */
struct rte_mbuf * _alloc_mbuf(uint16_t data_len)
{
	struct rte_mbuf *m = malloc(sizeof(struct rte_mbuf));

	m->pkt.next = NULL;
	m->pkt.pkt_len = data_len;
	m->pkt.vlan_macip.data = 0;
	m->pkt.nb_segs = 1;
	m->pkt.in_port = 0xff;

	m->ol_flags = 0;
	m->pkt.data = malloc(data_len);
	m->pkt.data_len = data_len;

	m->pkt.hash.sched = 0;

	return m;
}

void _free_mbuf(struct rte_mbuf * m)
{
	free(m->pkt.data);
	free(m);
}


/* Test function forward declarations */
void test_pktgen_rnd_bits_init(void);
void test_pktgen_set_random_bitfield(void);
void test_pktgen_rnd_bits_apply(void);



/***
 * Test driver
 ***/
int main(void) {
	test_pktgen_rnd_bits_init();
	test_pktgen_set_random_bitfield();
	test_pktgen_rnd_bits_apply();

    done_testing();
    return 0;
}



/***
 * Test implementations
 ***/
void test_pktgen_rnd_bits_init(void)
{
	rnd_bits_t *rnd = NULL;
	int i;

	lives_ok({ pktgen_rnd_bits_init(&rnd); }, "pktgen_rnd_bits_init() must not crash");
	ok(rnd != NULL, "... and allocate memory");
	cmp_ok(rnd->active_specs, "==" ,0 , "... and disable all specs");

	for (i = 0; i < MAX_RND_BITFIELDS; ++i) {
		cmp_ok(rnd->specs[i].offset,  "==", 0, "Spec %d offset must be cleared", i);
		cmp_ok(rnd->specs[i].andMask, "==", 0, "... and andMask must be cleared");
		cmp_ok(rnd->specs[i].orMask , "==", 0, "... and orMask must be cleared");
		cmp_ok(rnd->specs[i].rndMask, "==", 0, "... and rndMask must be cleared");
	};
}


void test_pktgen_set_random_bitfield(void)
{
	rnd_bits_t *rnd = NULL;
	int i;

	pktgen_rnd_bits_init(&rnd);

	/* Fail gracefully on wrong input (mask of length 33) */
	//                                               1         2         3
	//                                      123456789012345678901234567890123
	pktgen_set_random_bitfield(rnd, 0, 50, "000000000000000000000000000000000");
	cmp_ok(rnd->active_specs, "==", 0, "Too long mask must not change spec state");

	pktgen_set_random_bitfield(rnd, MAX_RND_BITFIELDS + 1, 50, "01.X");
	cmp_ok(rnd->active_specs, "==", 0, "Too large bitfield index must not change spec state");

	pktgen_set_random_bitfield(rnd, 0, 50, "01&");
	cmp_ok(rnd->active_specs, "==", 0, "Wrong character in mask must not change spec state");

	/* Work correctly on correct input */
	for (i = 0; i < MAX_RND_BITFIELDS; ++i) {
		pktgen_set_random_bitfield(rnd, i, i, "01.X");
		cmp_ok(rnd->active_specs,     "==", (1 << i), "Spec with index %d must be activated with valid mask", i, i);
		cmp_ok(rnd->specs[i].offset,  "==", i,        "... and the correct offset must be set");
		cmp_ok(rnd->specs[i].andMask, "!=", 0U,       "... and andMask must be set");
		cmp_ok(rnd->specs[i].orMask,  "!=", 0U,       "... and orMask must be set");
		cmp_ok(rnd->specs[i].rndMask, "!=", 0U,       "... and rndMask must be set");

		pktgen_set_random_bitfield(rnd, i, 0, "");
		cmp_ok(rnd->active_specs, "==", 0, "Spec with index %d must be deactivated with empty mask", i);
	}
}


void test_pktgen_rnd_bits_apply(void)
{
	rnd_bits_t *rnd = NULL;
	int i;

	/* Expected values in tests */
	const uint8_t all_0x00[64] = { 0 };
	const uint8_t all_0xff[64] = { [0 ... 63] = 0xff };

	struct rte_mbuf *mbuf = _alloc_mbuf(64);
	uint8_t *data_ptr = (uint8_t *)mbuf->pkt.data;

	pktgen_rnd_bits_init(&rnd);

	/* Work correctly when no bitmasks are enabled */
	/* all 0's */
	memset(mbuf->pkt.data, 0x00, 64);
	pktgen_rnd_bits_apply(&mbuf, 1, rnd);

	cmp_mem(data_ptr, all_0x00, 64, "Must not set bits when called with no bitmasks enabled");

	/* all 1's */
	memset(mbuf->pkt.data, 0xff, 64);
	pktgen_rnd_bits_apply(&mbuf, 1, rnd);

	cmp_mem(data_ptr, all_0xff, 64, "Must not clear bits when called with no bitmasks enabled");


	/* Work correctly for all valid mask lengths and all mask values */
	for (i = 1; i < MAX_BITFIELD_SIZE; ++i) {
		/* + 1 for trailing \0 */
		char mask0[i + 1], mask1[i + 1], maskIgn[i + 1], maskRnd[i + 1];
		int j;
		for (j = 0; j < i; ++j) {
			mask0[j]   = '0';
			mask1[j]   = '1';
			maskIgn[j] = '.';
			maskRnd[j] = 'X';
		}
		mask0[i] = mask1[i] = maskRnd[i] = maskIgn[i] = '\0';

		/* Mask of 0's
		 *
		 * Example of the resulting bitmasks (for length 5):
		 *                         1                   2                   3
		 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * &  |0|0|0|0|0|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |  |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * R  |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *
		 * &: AND mask
		 * |: OR mask
		 * R: random mask
		 */
		pktgen_set_random_bitfield(rnd, i, i, mask0);
		cmp_ok(rnd->active_specs,     "==", (1 << i),
				"Mask [%s] with length %d in position %d must be accepted", mask0, i, i);
		cmp_ok(rnd->specs[i].offset,  "==", i,                        "... with the correct offset");
		cmp_ok(rnd->specs[i].andMask, "==", htonl(~(uint32_t)0 >> i), "... and the correct AND bitmask");
		cmp_ok(rnd->specs[i].orMask,  "==", htonl(0U),                "... and the correct OR bitmask");
		cmp_ok(rnd->specs[i].rndMask, "==", htonl(0U),                "... and the correct random bitmask");

		/* Set memory to all 0's and check if only bits at the specified offset
		 * matching the mask are touched.
		 */
		memset(mbuf->pkt.data, 0x00, 64);

		lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 0 bits");
		cmp_mem(data_ptr, all_0x00, i, "    ... without touching anything before the offset");
		cmp_ok(*(uint32_t *) &(data_ptr[i]), "==", 0, "    ... and affecting the bits at the offset");
		cmp_mem(data_ptr + i + 4, all_0x00 + i + 4, 64 - (i + 4), "    ... and without touching anything after the offset");

		/* Set memory to all 1's and check if only bits at the specified offset
		 * matching the mask are touched.
		 */
		memset(mbuf->pkt.data, 0xff, 64);

		lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 1 bits");
		cmp_mem(data_ptr, all_0xff, i, "    ... without touching anything before the offset");
		cmp_ok(*(uint32_t *) &(data_ptr[i]), "==", htonl(~(uint32_t)0 >> i), "    ... and affecting the bits at the offset");
		cmp_mem(data_ptr + i + 4, all_0xff + i + 4, 64 - (i + 4), "    ... and without touching anything after the offset");


		/* clean up for next test */
		pktgen_set_random_bitfield(rnd, i, 0, "");


		/* Mask of 1's
		 *
		 * Example of the resulting bitmasks (for length 5):
		 *                         1                   2                   3
		 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * &  |1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |  |1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * R  |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *
		 * &: AND mask
		 * |: OR mask
		 * R: random mask
		 */
		pktgen_set_random_bitfield(rnd, i, i, mask1);
		cmp_ok(rnd->active_specs,     "==", (1 << i),
				"Mask [%s] with length %d must be accepted", mask1, i);
		cmp_ok(rnd->specs[i].offset,  "==", i,                   "... with the correct offset");
		cmp_ok(rnd->specs[i].andMask, "==", htonl(~(uint32_t)0), "... with the correct AND bitmask");
		cmp_ok(rnd->specs[i].orMask,  "==", htonl(~(uint32_t)0 << (MAX_BITFIELD_SIZE - i)),
				"... and the correct OR bitmask");
		cmp_ok(rnd->specs[i].rndMask, "==", htonl(0U),           "... and the correct random bitmask");

		/* Set memory to all 0's and check if only bits at the specified offset
		 * matching the mask are touched.
		 */
		memset(mbuf->pkt.data, 0x00, 64);

		lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 0 bits");
		cmp_mem(data_ptr, all_0x00, i, "    ... without touching anything before the offset");
		cmp_ok(*(uint32_t *) &(data_ptr[i]), "==", htonl((~(uint32_t)0) << (MAX_BITFIELD_SIZE - i)),
				"    ... and affecting the bits at the offset");
		cmp_mem(data_ptr + i + 4, all_0x00 + i + 4, 64 - (i + 4), "    ... and without touching anything after the offset");

		/* Set memory to all 1's and check if only bits at the specified offset
		 * matching the mask are touched.
		 */
		memset(mbuf->pkt.data, 0xff, 64);

		lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 1 bits");
		cmp_mem(data_ptr, all_0xff, i, "    ... without touching anything before the offset");
		cmp_ok(*(uint32_t *) &(data_ptr[i]), "==", ~(uint32_t)0,
				"    ... and affecting the bits at the offset");
		cmp_mem(data_ptr + i + 4, all_0xff + i + 4, 64 - (i + 4), "    ... and without touching anything after the offset");


		/* clean up for next test */
		pktgen_set_random_bitfield(rnd, i, 0, "");


		/* Mask of .'s (ignore)
		 *
		 * Example of the resulting bitmasks (for length 5):
		 *                         1                   2                   3
		 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * &  |1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |  |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * R  |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *
		 * &: AND mask
		 * |: OR mask
		 * R: random mask
		 */
		pktgen_set_random_bitfield(rnd, i, i, maskIgn);
		cmp_ok(rnd->active_specs,     "==", (1 << i),     "Mask [%s] with length %d must be accepted", maskIgn, i);
		cmp_ok(rnd->specs[i].offset,  "==", i,            "... with the correct offset");
		cmp_ok(rnd->specs[i].andMask, "==", ~(uint32_t)0, "... with the correct AND bitmask");
		cmp_ok(rnd->specs[i].orMask,  "==", 0U,           "... and the correct OR bitmask");
		cmp_ok(rnd->specs[i].rndMask, "==", 0U,           "... and the correct random bitmask");

		/* Set memory to all 0's and check if only bits at the specified offset
		 * matching the mask are touched.
		 */
		memset(mbuf->pkt.data, 0x00, 64);

		lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 0 bits");
		cmp_mem(data_ptr, all_0x00, 64, "    ... without touching any bits");

		/* Set memory to all 1's and check if only bits at the specified offset
		 * matching the mask are touched.
		 */
		memset(mbuf->pkt.data, 0xff, 64);

		lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 1 bits");
		cmp_mem(data_ptr, all_0xff, 64, "    ... without touching any bits");


		/* clean up for next test */
		pktgen_set_random_bitfield(rnd, i, 0, "");


		/* Mask of X's (random)
		 *
		 * Example of the resulting bitmasks (for length 5):
		 *                         1                   2                   3
		 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * &  |0|0|0|0|0|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |  |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * R  |1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
		 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *
		 * &: AND mask
		 * |: OR mask
		 * R: random mask
		 */
		pktgen_set_random_bitfield(rnd, i, i, maskRnd);
		cmp_ok(rnd->active_specs,     "==", (1 << i),                 "Mask [%s] with length %d must be accepted", maskRnd, i);
		cmp_ok(rnd->specs[i].offset,  "==", i,                        "... with the correct offset");
		cmp_ok(rnd->specs[i].andMask, "==", htonl(~(uint32_t)0 >> i), "... with the correct AND bitmask");
		cmp_ok(rnd->specs[i].orMask,  "==", htonl(0U),                "... and the correct OR bitmask");
		cmp_ok(rnd->specs[i].rndMask, "==", htonl(~(uint32_t)0 << (MAX_BITFIELD_SIZE - i)),
				"... and the correct random bitmask");

		int rnd_func;
		for (rnd_func = 0; rnd_func < 2; ++rnd_func) {
			/* Test twice: once with random function returning all 0's, once
			 * with random function returning all 1's.
			 */
			if (rnd_func == 0) {
				pktgen_set_rnd_func(all_bits_0);
			}
			else {
				pktgen_set_rnd_func(all_bits_1);
			}

			/* Set memory to all 0's and check if only bits at the specified offset
			 * matching the mask are touched.
			 */
			memset(mbuf->pkt.data, 0x00, 64);

			lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 0 bits");
			cmp_mem(data_ptr, all_0x00, i, "    ... without touching anything before the offset");
			cmp_ok(*(uint32_t *) &(data_ptr[i]), "==",
					htonl((rnd_func == 0) ? 0 : (~(uint32_t)0 << (MAX_BITFIELD_SIZE - i))),
					"    ... and affecting the bits at the offset");
			cmp_mem(data_ptr + i + 4, all_0x00 + i + 4, 64 - (i + 4), "    ... and and without touching anything after the offset");

			/* Set memory to all 1's and check if only bits at the specified offset
			 * matching the mask are touched.
			 */
			memset(mbuf->pkt.data, 0xff, 64);

			lives_ok( { pktgen_rnd_bits_apply(&mbuf, 1, rnd); }, "... and must be applied to all 1 bits");
			cmp_mem(data_ptr, all_0xff, i,
					"    ... without touching anything before the offset");
			cmp_ok(*(uint32_t *) &(data_ptr[i]), "==",
					htonl((rnd_func == 0) ? (~(uint32_t)0 >> i) : ~(uint32_t)0),
					"    ... and affecting the bits at the offset");
			cmp_mem(data_ptr + i + 4, all_0xff + i + 4, 64 - (i + 4),
					"    ... and without touching anything after the offset");
		}

		/* clean up for next test */
		pktgen_set_random_bitfield(rnd, i, 0, "");
	}

	/* Multiple bitfield specs active at the same time */
	pktgen_set_random_bitfield(rnd, 4, 50, "01");
	pktgen_set_random_bitfield(rnd, 2, 60, "10");
	cmp_ok(rnd->active_specs,    "==", (1 << 4) + (1 << 2), "Multiple valid specs must be activated");

	pktgen_set_random_bitfield(rnd, 4, 0, "");
	cmp_ok(rnd->active_specs,    "==", (1 << 2), "... and active_specs must be updated correctly when disabling 1 spec");
}
