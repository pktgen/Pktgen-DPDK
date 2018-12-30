/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_RANDOM_H_
#define _PKTGEN_RANDOM_H_

#include <stdint.h>

#include <rte_mbuf.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bitfield size and max. entries */
#define MAX_RND_BITFIELDS       32

#define BITFIELD_T              uint32_t
#define MAX_BITFIELD_SIZE       (sizeof(BITFIELD_T) << 3)

struct port_info_s;

/* Random bitfield specification */
typedef struct bf_spec_s {
	uint8_t offset;	/**< Offset (in bytes) of where to apply the bitmask */

	BITFIELD_T andMask;	/**< Mask to bitwise AND value with */
	BITFIELD_T orMask;	/**< Mask to bitwise OR value with */
	BITFIELD_T rndMask;	/**< Which bits will get random value */
} bf_spec_t;

typedef struct rnd_bits_s {
	uint32_t active_specs;	/**< Boolean vector identifying active specs */

	bf_spec_t specs[MAX_RND_BITFIELDS];	/**< Bitmask specifications */
} rnd_bits_t;

/* Data structure initialization */
void pktgen_rnd_bits_init(struct rnd_bits_s **rnd_bits);

/* Set random bitfield */
uint32_t pktgen_set_random_bitfield(struct rnd_bits_s *rnd_bits,
					   uint8_t idx,
					   uint8_t offset,
					   const char *mask);

/* Apply random bitfields description to packet contents */
void pktgen_rnd_bits_apply(struct port_info_s *info,
				  struct rte_mbuf **pkt,
				  size_t cnt,
				  struct rnd_bits_s *rbits);

/* Display page with random bitfield settings */
void pktgen_page_random_bitfields(uint32_t print_labels,
					 uint16_t pid,
					 struct rnd_bits_s *rnd_bits);

#ifdef TESTING
/* Change PRNG function at runtime */
typedef BITFIELD_T (*rnd_func_t)(void);
rnd_func_t pktgen_set_rnd_func(rnd_func_t rnd_func);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_RANDOM_H_ */
