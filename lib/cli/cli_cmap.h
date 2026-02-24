/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2025>, Intel Corporation.
 */

/**
 * @file
 * CPU core map — parses /proc/cpuinfo into a lcore/socket/core/thread table.
 *
 * Used internally by the CLI cpu-info display to map logical cores to their
 * physical socket, core, and hyper-thread IDs.
 */

#ifndef __CLI_CMAP_H
#define __CLI_CMAP_H

#include <stdint.h>

#define MAX_LINE_SIZE 4096

#define PROC_CPUINFO "/proc/cpuinfo"

/** Per-logical-core topology descriptor packed into a single 32-bit word. */
typedef union {
    struct {
        uint8_t lid; /**< Logical core ID */
        uint8_t sid; /**< CPU socket ID */
        uint8_t cid; /**< Physical CPU core ID */
        uint8_t tid; /**< Hyper-thread ID */
    };
    uint32_t word; /**< Raw 32-bit representation */
} lc_info_t;

/** Linked-list node for a single logical core entry. */
typedef struct lcore {
    struct lcore *next; /**< Next lcore in the singly-linked list */
    lc_info_t u;        /**< Topology info for this lcore */
} lcore_t;

/** Aggregated CPU topology table for the whole system. */
struct cmap {
    uint16_t num_cores; /**< Total number of logical cores */
    uint16_t sid_cnt;   /**< Number of distinct socket IDs */
    uint16_t cid_cnt;   /**< Number of distinct physical core IDs */
    uint16_t tid_cnt;   /**< Number of distinct hyper-thread IDs */
    lc_info_t *linfo;   /**< Flat array of per-lcore topology info */
    char *model;        /**< CPU model name string (from /proc/cpuinfo) */
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
 * Return the count of unique values for a given topology property.
 *
 * Walks the lcore linked list and returns (max_value + 1) for the
 * property selected by @p get (e.g. socket ID, core ID, or thread ID).
 *
 * @param lc
 *   Head of the singly-linked lcore list.
 * @param get
 *   Getter function that extracts the property to count from an lcore_t.
 * @return
 *   Number of unique values (max observed value + 1), or 0 if @p get is NULL.
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

#endif /*_CLI_CMAP_H */
