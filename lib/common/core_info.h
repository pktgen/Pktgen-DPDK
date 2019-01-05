/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2013 by Keith Wiles @ intel.com */

#ifndef _CORE_INFO_H
#define _CORE_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 *
 * Return the first and last lcore index values into the char pointers args.
 *
 * \returns number of lcores enabled.
 */
static __inline__ uint32_t
lcore_mask(uint16_t *first, uint16_t *last) {
	int32_t cnt, lid;

	lid  = rte_get_master_lcore();
	if (first)
		*first  = lid;

	/* Count the number of lcores being used. */
	for (cnt = 0; lid < RTE_MAX_LCORE; lid++) {
		if (!rte_lcore_is_enabled(lid) )
			continue;
		cnt++;
		if (last)
			*last = lid;
	}

	return cnt;
}

uint32_t sct_convert(char *sct[]);

#ifdef __cplusplus
}
#endif

#endif /* _CORE_INFO_H */
