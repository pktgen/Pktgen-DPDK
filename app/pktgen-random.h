/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_RANDOM_H_
#define _PKTGEN_RANDOM_H_

/**
 * @file
 *
 * Per-port random bitfield mask engine for Pktgen.
 *
 * Allows independent random bits to be applied to arbitrary byte offsets
 * within each transmitted packet, enabling fuzz-style traffic generation
 * without rebuilding the full packet template each burst.
 */

#include <stdint.h>

#include <rte_mbuf.h>

#include "pktgen-seq.h"

#include "xorshift64star.h" /* PRNG function */

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RND_BITFIELDS 32 /**< Maximum simultaneous random bitfield specs per port */

#define BITFIELD_T        uint32_t /**< Underlying integer type for bitfield masks */
#define MAX_BITFIELD_SIZE (sizeof(BITFIELD_T) << 3) /**< Size of BITFIELD_T in bits */

struct port_info_s;

/**
 * Random bitfield specification — describes one randomised bit range in a packet.
 */
typedef struct bf_spec_s {
    uint8_t offset; /**< Offset (in bytes) of where to apply the bitmask */

    BITFIELD_T andMask; /**< Mask to bitwise AND value with */
    BITFIELD_T orMask;  /**< Mask to bitwise OR value with */
    BITFIELD_T rndMask; /**< Which bits will get a random value */
} bf_spec_t;

/**
 * Collection of all active random bitfield specs for one port.
 */
typedef struct rnd_bits_s {
    uint32_t active_specs; /**< Bitmask identifying which spec slots are active */

    bf_spec_t specs[MAX_RND_BITFIELDS]; /**< Per-slot bitfield specifications */
} rnd_bits_t;

/**
 * Allocate and zero-initialise a rnd_bits_t for a port.
 *
 * @param rnd_bits
 *   Address of the pointer to fill; on success *rnd_bits points to the
 *   newly allocated structure.
 */
void pktgen_rnd_bits_init(struct rnd_bits_s **rnd_bits);

/**
 * Configure one random bitfield slot.
 *
 * @param rnd_bits
 *   Pointer to the rnd_bits_t to modify.
 * @param idx
 *   Slot index (0 .. MAX_RND_BITFIELDS-1).
 * @param offset
 *   Byte offset within the packet where the mask is applied.
 * @param mask
 *   String of '0', '1', and 'X' characters (MSB first) describing
 *   fixed-0, fixed-1, and randomised bit positions respectively.
 * @return
 *   0 on success, non-zero on error.
 */
uint32_t pktgen_set_random_bitfield(struct rnd_bits_s *rnd_bits, uint8_t idx, uint8_t offset,
                                    const char *mask);

/**
 * Apply all active random bitfield specs to a burst of packets.
 *
 * @param info
 *   Per-port state (used for port-specific context).
 * @param pkt
 *   Array of mbuf pointers to modify in place.
 * @param cnt
 *   Number of mbufs in @p pkt.
 * @param rbits
 *   Random bitfield configuration to apply.
 */
void pktgen_rnd_bits_apply(struct port_info_s *info, struct rte_mbuf **pkt, size_t cnt,
                           struct rnd_bits_s *rbits);

/**
 * Render the random-bitfields display page to the terminal.
 *
 * @param print_labels
 *   Non-zero to (re)print column headers and labels.
 * @param pid
 *   Port index whose bitfield settings are displayed.
 * @param rnd_bits
 *   Pointer to the rnd_bits_t to display.
 */
void pktgen_page_random_bitfields(uint32_t print_labels, uint16_t pid, struct rnd_bits_s *rnd_bits);

/**
 * Generate a 32-bit random value using the built-in xorshift64* PRNG.
 *
 * @return
 *   32-bit pseudo-random number.
 */
static __inline__ uint32_t
pktgen_default_rnd_func(void)
{
    return (uint32_t)xorshift64star();
}

#ifdef TESTING
/** Function pointer type for a pluggable random-number generator. */
typedef BITFIELD_T (*rnd_func_t)(void);

/**
 * Replace the active PRNG function (test/validation use only).
 *
 * @param rnd_func
 *   New PRNG function to install.
 * @return
 *   Previously installed PRNG function.
 */
rnd_func_t pktgen_set_rnd_func(rnd_func_t rnd_func);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_RANDOM_H_ */
