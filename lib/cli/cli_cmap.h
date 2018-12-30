/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */

#ifndef __CLI_CMAP_H
#define __CLI_CMAP_H

#include <stdint.h>

#define MAX_LINE_SIZE	4096

#define PROC_CPUINFO "/proc/cpuinfo"

typedef union {
	struct {
		uint8_t lid;	/* Logical core ID */
		uint8_t sid;	/* CPU socket ID */
		uint8_t cid;	/* Physical CPU core ID */
		uint8_t tid;	/* Hyper-thread ID */
	};
	uint32_t word;
} lc_info_t;

typedef struct lcore {
	struct lcore *next;
	lc_info_t u;
} lcore_t;

struct cmap {
	uint16_t num_cores;
	uint16_t sid_cnt;
	uint16_t cid_cnt;
	uint16_t tid_cnt;
	lc_info_t *linfo;
	char *model;
};

typedef lcore_t *(*do_line_fn)(const char *line, lcore_t *);
typedef unsigned (*getter_fn)(const lcore_t *);
typedef void (*setter_fn)(lcore_t *, unsigned new_val);

typedef struct action {
	const char *desc;
	do_line_fn fn;
} action_t;

/**
 * Create a cmap structure for the current system
 *
 * @return
 *   The pointer to the cmap structure or NULL on error
 */
struct cmap *cmap_create(void);

/**
 * Return the current CPU model string
 *
 * @return
 *   Pointer to current CPU model string.
 */
char *cmap_cpu_model(void);

/**
 * Free up the resources attached to a cmap structure
 *
 * @param cmap
 *   A valid cmap pointer
 */
void cmap_free(struct cmap *cmap);

/**
 * Return the socket id for a given lcore (Internal)
 *
 * @param lc
 *   Pointer to the given lcore structure
 * @return
 *   The socket ID value
 */
static inline unsigned int
cmap_socket_id(const lcore_t *lc)
{
	return lc->u.sid;
}

/**
 * Set the socket id for a given lcore (Internal)
 *
 * @param lc
 *   Pointer to the given lcore structure
 * @param v
 *   Set the socket id value
 * @return
 *   N/A
 */
static inline void
cmap_set_socket_id(lcore_t *lc, unsigned v)
{
	lc->u.sid = v;
}

/**
 * Return the core id for a given lcore (Internal)
 *
 * @param lc
 *   Pointer to the given lcore structure
 * @return
 *   The core ID value
 */
static inline unsigned int
cmap_core_id(const lcore_t *lc)
{
	return lc->u.cid;
}

/**
 * Set the core id for a given lcore (Internal)
 *
 * @param lc
 *   Pointer to the given lcore structure
 * @param v
 *   Set the core id value
 * @return
 *   N/A
 */
static inline void
cmap_set_core_id(lcore_t *lc, unsigned v)
{
	lc->u.cid = v;
}

/**
 * Return the thread id for a given lcore (Internal)
 *
 * @param lc
 *   Pointer to the given lcore structure
 * @return
 *   The thread ID value
 */
static inline unsigned int
cmap_thread_id(const lcore_t *lc)
{
	return lc->u.tid;
}

/**
 * Returns the number of unique values that exist of the property.
 *
 * @param lc
 *   Pointer to the lcore structure
 * @param get
 *   A function pointer to help count the type of values
 */
static inline unsigned int
cmap_cnt(lcore_t *lc, getter_fn get)
{
	unsigned cnt = 0;

	if (!get)
		return cnt;

	while (lc) {
		if (cnt < get(lc))
			cnt = get(lc);
		lc = lc->next;
	}
	return cnt + 1;
}

#endif  /*_CLI_CMAP_H */
