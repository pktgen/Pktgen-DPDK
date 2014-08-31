/* From http://xorshift.di.unimi.it/xorshift128plus.c */

#ifndef _XORSHIFT128PLUS_H_
#define _XORSHIFT128PLUS_H_

/*  Written in 2014 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>

/* This is the fastest generator passing BigCrush without systematic
   errors, but due to the relatively short period it is acceptable only
   for applications with a very mild amount of parallelism; otherwise, use
   a xorshift1024* generator. */

/* The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to pass it twice through MurmurHash3's
   avalanching function. */

uint64_t xor_seed[ 2 ];

static inline uint64_t xor_next(void) {
	uint64_t s1 = xor_seed[ 0 ];
	const uint64_t s0 = xor_seed[ 1 ];
	xor_seed[ 0 ] = s0;
	s1 ^= s1 << 23; // a
	return ( xor_seed[ 1 ] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0; // b, c
}


#endif  // _XORSHIFT128PLUS_H_
