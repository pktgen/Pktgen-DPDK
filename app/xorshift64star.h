/*
 * Marsaglia, George (July 2003). "Xorshift RNGs".
 * Journal of Statistical Software.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* https://en.wikipedia.org/wiki/Xorshift */

#ifndef _XORSHIFT64STAR_H_
#define _XORSHIFT64STAR_H_

/**
 * @file
 *
 * Xorshift64* fast pseudo-random number generator.
 *
 * Based on Marsaglia (2003) "Xorshift RNGs", Journal of Statistical Software.
 * See https://en.wikipedia.org/wiki/Xorshift for the algorithm description.
 * The state must be seeded with a non-zero value before the first call.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Generator state (must be seeded with a non-zero value before first use). */
extern uint64_t xor_state[1];

/**
 * Generate the next 64-bit pseudo-random value using the xorshift64* algorithm.
 *
 * Updates @c xor_state[0] in place and returns the scrambled output.
 *
 * @return
 *   64-bit pseudo-random number.
 */
static inline uint64_t
xorshift64star(void)
{
    uint64_t x = xor_state[0]; /* The state must be seeded with a nonzero value. */
    x ^= x >> 12;              // a
    x ^= x << 25;              // b
    x ^= x >> 27;              // c
    xor_state[0] = x;
    return x * 0x2545F4914F6CDD1D;
}

#ifdef __cplusplus
}
#endif

#endif /* _XORSHIFT64STAR_H_ */
