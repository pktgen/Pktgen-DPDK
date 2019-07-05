/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/**
 * @file
 *
 * Compat file for pktgen
 */

#ifndef __DELAY_H_
#define __DELAY_H_

#include <time.h>
#include <rte_version.h>
#include <rte_cycles.h>

#ifdef __cplusplus
extern "C" {
#endif

#if RTE_VERSION < RTE_VERSION_NUM(18,11,0,0)
static inline void
rte_delay_us_sleep(unsigned int us)
{
        struct timespec wait[2];
        int ind = 0;

        wait[0].tv_sec = 0;
        if (us >= US_PER_S) {
                wait[0].tv_sec = us / US_PER_S;
                us -= wait[0].tv_sec * US_PER_S;
        }
        wait[0].tv_nsec = 1000 * us;

        while (nanosleep(&wait[ind], &wait[1 - ind]) && errno == EINTR) {
                /*
                 * Sleep was interrupted. Flip the index, so the 'remainder'
                 * will become the 'request' for a next call.
                 */
                ind = 1 - ind;
        }
}
#endif


#ifdef __cplusplus
}
#endif

#endif /* __DELAY_H_ */
