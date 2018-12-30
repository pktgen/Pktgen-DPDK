/*-
 * Copyright (c) <2016-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2016 by Keith Wiles @ intel.com */

#include "pktgen-random.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <rte_malloc.h>

#include "rte_lua.h"

#include "pktgen-display.h"
#include "pktgen-log.h"

#include "xorshift64star.h"	/* PRNG function */

/* Allow PRNG function to be changed at runtime for testing*/
#ifdef TESTING
static rnd_func_t _rnd_func = NULL;
#endif

/* Forward declaration */
static void pktgen_init_default_rnd(void);

/**************************************************************************//**
 *
 * pktgen_default_rnd_func - Default function used to generate random values
 *
 * DESCRIPTION
 * Default function to use for generating random values. This function is used
 * when no external random function is set using pktgen_set_rnd_func();
 *
 * RETURNS: 32-bit random value.
 *
 * SEE ALSO:
 */
static __inline__ uint32_t
pktgen_default_rnd_func(void)
{
	return (uint32_t)xorshift64star();
}

/**************************************************************************//**
 *
 * pktgen_rnd_bits_init - Initialize random bitfield structures and PRNG
 *
 * DESCRIPTION
 * Initialize the random bitfield structures the PRNG context.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_rnd_bits_init(rnd_bits_t **rnd_bits)
{
	int i;

	*rnd_bits = (rnd_bits_t *)rte_zmalloc_socket(
			"Random bitfield structure",
			sizeof(rnd_bits_t),
			0,
			rte_socket_id());

	pktgen_display_set_color("stats.stat.values");

	/* Initialize mask to all ignore */
	for (i = 0; i < MAX_RND_BITFIELDS; ++i) {
		pktgen_set_random_bitfield(*rnd_bits, i, 0,
					   "????????????????????????????????");	/* 32 ?'s */
		pktgen_set_random_bitfield(*rnd_bits, i, 0, "");
	}

	pktgen_init_default_rnd();
}

/**************************************************************************//**
 *
 * pktgen_set_random_bitfield - Set random bit specification
 *
 * DESCRIPTION
 * Set random bit specification. This extracts the 0, 1 and random bitmasks from
 * the provided bitmask template.
 *
 * RETURNS: Active specifications
 *
 * SEE ALSO:
 */

uint32_t
pktgen_set_random_bitfield(rnd_bits_t *rnd_bits,
			   uint8_t idx,
			   uint8_t offset,
			   const char *mask)
{
	if (idx >= MAX_RND_BITFIELDS)
		goto leave;

	size_t mask_len = strlen(mask);
	if (mask_len > MAX_BITFIELD_SIZE)
		goto leave;

	/* Disable spec number idx when no mask is specified */
	if (mask_len == 0) {
		rnd_bits->active_specs &= ~((uint32_t)1 << idx);
		goto leave;
	}

	/* Iterate over specified mask from left to right. The mask components are
	 * shifted left and filled at the right side.
	 * When the complete mask has been processed, for each position exactly 1
	 * mask component has a 1 bit set.
	 */
	BITFIELD_T mask0 = 0, mask1 = 0, maskRnd = 0;

	uint32_t i;
	for (i = 0; i < mask_len; ++i) {
		mask0   <<= 1;
		mask1   <<= 1;
		maskRnd <<= 1;

		switch (mask[i]) {
		case '0': mask0   += 1; break;
		case '1': mask1   += 1; break;
		case '.': /* ignore bit */ break;
		case 'x':
		case 'X': maskRnd += 1; break;
		default:/* print error: "Unknown char in bitfield spec" */
			goto
			leave;
		}
	}

	/* Shift bitmasks to MSB position, so the bitmask starts at the provided
	 * offset.
	 * This way the mask can be used as a full-width BITFIELD_T, even when the
	 * provided mask is shorter than the number of bits a BITFIELD_T can store.
	 */
	int pad_len = MAX_BITFIELD_SIZE - mask_len;
	mask0   <<= pad_len;
	mask1   <<= pad_len;
	maskRnd <<= pad_len;

	rnd_bits->active_specs |= (1 << idx);

	rnd_bits->specs[idx].offset  = offset;

	/* andMask is used to clear certain bits. Bits corresponding to a 1 bit in
	 * the mask will retain their original value, bits corresponding to a 0 bit
	 * in the mask will be cleared.
	 *
	 * - mask0: all set bits in this mask must be 0 in the result
	 * - maskRnd: all set bits in this mask will be zero'd. This enables merging
	 *       of a random value by doing a bitwise OR with the random value.
	 */
	rnd_bits->specs[idx].andMask = htonl(~(mask0 | maskRnd));

	/* orMask is used to set certain bits.
	 */
	rnd_bits->specs[idx].orMask  = htonl(mask1);

	/* rndMask is used to filter a generated random value, so all bits that
	 * should not be random are 0.
	 * The result of this filtering operation can then be bitwise OR-ed with
	 * the original value to merge the random value in.
	 * In the original value, the bits that must be random need to be 0. This is
	 * taken care of by taking the rndMask into account for the andMask.
	 */
	rnd_bits->specs[idx].rndMask = htonl(maskRnd);

leave:
	return rnd_bits->active_specs;
}

/**************************************************************************//**
 *
 * pktgen_rnd_bits_apply - Set random bitfields in packet.
 *
 * DESCRIPTION
 * Set bitfields in packet specified packet to random values according to the
 * requested bitfield specification.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_rnd_bits_apply(port_info_t *info,
		      struct rte_mbuf **pkts,
		      size_t cnt,
		      rnd_bits_t *rbits)
{
	rnd_bits_t *rnd_bits;
	size_t mbuf_cnt;
	uint32_t active_specs;
	uint32_t *pkt_data;
	BITFIELD_T rnd_value;
	bf_spec_t *bf_spec;

	/* the info pointer could be null. */
	rnd_bits = (rbits) ? rbits : info->rnd_bitfields;
	if ((active_specs = rnd_bits->active_specs) == 0)
		return;

	for (mbuf_cnt = 0; mbuf_cnt < cnt; ++mbuf_cnt) {
		bf_spec = rnd_bits->specs;

		while (active_specs > 0) {
			if (likely(active_specs & 1) ) {
				/* Get pointer to byte <offset> in mbuf data as uint32_t*, so */
				/* the masks can be applied. */
				pkt_data = (uint32_t *)(&rte_pktmbuf_mtod(pkts[mbuf_cnt], uint8_t *)[bf_spec->offset]);

				*pkt_data &= bf_spec->andMask;
				*pkt_data |= bf_spec->orMask;

				if (bf_spec->rndMask) {
#ifdef TESTING
					/* Allow PRNG to be set when testing */
					rnd_value  = _rnd_func ? _rnd_func() :
						pktgen_default_rnd_func();
#else
					/* ... but allow inlining for production build */
					rnd_value  = pktgen_default_rnd_func();
#endif
					rnd_value &= bf_spec->rndMask;
					*pkt_data |= rnd_value;
				}
			}

			++bf_spec;		/* prepare to use next bitfield spec */
			active_specs >>= 1;	/* use next bit in active spec bitfield */
		}

		active_specs = rnd_bits->active_specs;
	}
}

/**************************************************************************//**
 *
 * pktgen_page_random_bitfields - Display the random bitfields data page.
 *
 * DESCRIPTION
 * Display the random bitfields data page on the screen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_page_random_bitfields(uint32_t print_labels,
			     uint16_t pid,
			     rnd_bits_t *rnd_bits)
{
	uint32_t row, bitmask_idx, i, curr_bit;
	char mask[36];	/* 4*8 bits, 3 delimiter spaces, \0 */
	bf_spec_t *curr_spec;

	if (!print_labels)
		return;

	mask[35] = '\0';
	mask[8] = mask[17] = mask[26] = ' ';

	pktgen_display_set_color("top.page");
	display_topline("<Random bitfield Page>");

	pktgen_display_set_color("top.ports");
	scrn_printf(1, 3, "Port %d", pid);

	row = PORT_STATE_ROW;

	if (rnd_bits == NULL) {
		scrn_cprintf(
		        10,
		        this_scrn->ncols,
		        "** Port is not active - no random bitfields set **");
		row = 28;
		goto leave;
	}
	pktgen_display_set_color("stats.stats.label");
	/* Header line */
	scrn_printf(
	        row++,
	        1,
	        "%8s %8s %8s  %s",
	        "Index",
	        "Offset",
	        "Act?",
	        "Mask [0 = 0 bit, 1 = 1 bit, X = random bit, . = ignore]");

	pktgen_display_set_color("stats.stat.values");
	for (bitmask_idx = 0; bitmask_idx < MAX_RND_BITFIELDS; ++bitmask_idx) {
		curr_spec = &rnd_bits->specs[bitmask_idx];

		memset(mask, 0, sizeof(mask));
		memset(mask, ' ', sizeof(mask) - 1);
		/* Compose human readable bitmask representation */
		for (i = 0; i < MAX_BITFIELD_SIZE; ++i) {
			curr_bit = (uint32_t)1 << (MAX_BITFIELD_SIZE - i - 1);

			/* + i >> 3 for space delimiter after every 8 bits.
			 * Need to check rndMask before andMask: for random bits, the
			 * andMask is also 0. */
			mask[i + (i >> 3)] =
				((ntohl(curr_spec->rndMask) & curr_bit) != 0) ? 'X' :
				((ntohl(curr_spec->andMask) & curr_bit) == 0) ? '0' :
				((ntohl(curr_spec->orMask)  & curr_bit) != 0) ? '1' : '.';
		}

		scrn_printf(row++, 1, "%8d %8d %7s   %s",
		        bitmask_idx, curr_spec->offset,
		        (rnd_bits->active_specs & (1 << bitmask_idx)) ? "Yes" : "No",
		        mask);
	}

leave:
	display_dashline(++row);
	pktgen_display_set_color(NULL);
}

static void
pktgen_init_default_rnd(void)
{
	FILE *dev_random;
	int ret;

	if ((dev_random = fopen("/dev/urandom", "r")) == NULL) {
		pktgen_log_error("Could not open /dev/urandom for reading");
		return;
	}

	/* Use contents of /dev/urandom as seed for ISAAC */
	ret = fread(xor_state, 1, sizeof(xor_state[0]), dev_random);
	if (ret != sizeof(xor_state[0]))
		pktgen_log_warning(
			"Could not read enough random data for PRNG seed (%d)", ret);

	fclose(dev_random);
}

#ifdef TESTING
/**************************************************************************//**
 *
 * pktgen_set_rnd_func - Set function to use as random number generator
 *
 * DESCRIPTION
 * Set function to use as random number generator.
 *
 * RETURNS: Previous random number function (or NULL if the default function was
 *          used).
 *
 * SEE ALSO:
 */

rnd_func_t
pktgen_set_rnd_func(rnd_func_t rnd_func)
{
	rnd_func_t prev_rnd_func = _rnd_func;

	_rnd_func = rnd_func;

	return prev_rnd_func;
}

#endif
