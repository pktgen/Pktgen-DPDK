/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2022-2023 Intel Corporation
 */

#pragma once

/**
 * @file
 * Routines to help create a mutex.
 */

#include <errno.h>
#include <pthread.h>
#include <rte_log.h>
#include "hmap_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Global registry of all created hmaps. */
static TAILQ_HEAD(hmap_hmap_list, hmap) hmap_registry;
static pthread_mutex_t hmap_list_mutex;

/**
 * Helper routine to create a mutex with a specific type.
 *
 * @param mutex
 *   The pointer to the mutex to create.
 * @param flags
 *   The attribute flags used to create the mutex i.e. recursive attribute
 * @return
 *   0 on success or -1 on failure errno is set
 */
static inline int
hmap_mutex_create(pthread_mutex_t *mutex, int flags)
{
    pthread_mutexattr_t attr;
    int inited = 0, ret = EFAULT;

    if (!mutex)
        goto err;

#define __do(_exp)    \
    do {              \
        ret = _exp;   \
        if (ret)      \
            goto err; \
    } while (0 /* CONSTCOND */)

    __do(pthread_mutexattr_init(&attr));
    inited = 1;

    __do(pthread_mutexattr_settype(&attr, flags));

    __do(pthread_mutex_init(mutex, &attr));

    __do(pthread_mutexattr_destroy(&attr));

#undef __do

    return 0;
err:
    if (inited) {
        /* Do not lose the previous error value */
        if (pthread_mutexattr_destroy(&attr))
            HMAP_DEBUG("Failed to destroy mutex attribute, but is not the root cause\n");
    }

    errno = ret;
    return -1;
}

/**
 * Destroy a mutex
 *
 * @param mutex
 *   Pointer to mutex to destroy.
 * @return
 *   0 on success and -1 on error with errno set.
 */
static inline int
hmap_mutex_destroy(pthread_mutex_t *mutex)
{
    int ret = 0;

    if (mutex)
        ret = pthread_mutex_destroy(mutex);

    errno = ret;
    return (ret != 0) ? -1 : 0;
}

static inline void
hmap_list_lock(void)
{
    int ret = pthread_mutex_lock(&hmap_list_mutex);

    if (ret)
        HMAP_LOG("failed: %s\n", strerror(ret));
}

static inline void
hmap_list_unlock(void)
{
    int ret = pthread_mutex_unlock(&hmap_list_mutex);

    if (ret)
        HMAP_WARN("failed: %s\n", strerror(ret));
}

static inline void
hmap_lock(hmap_t *hmap)
{
    int ret = pthread_mutex_lock(&hmap->mutex);

    if (ret)
        HMAP_WARN("failed: %s\n", strerror(ret));
}

static inline void
hmap_unlock(hmap_t *hmap)
{
    int ret = pthread_mutex_unlock(&hmap->mutex);

    if (ret)
        HMAP_WARN("failed: %s\n", strerror(ret));
}

static uint32_t
_hmap_hash(const char *prefix, const char *key)
{
    const char *k;
    uint32_t hash;

    hash = 0;
    if (prefix) {
        k = prefix;
        for (int i = 0; k[i]; i++) {
            hash += k[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
    }

    k = key;
    for (int i = 0; i < k[i]; i++) {
        hash += k[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

/* Return true or false, true or non-zero for a match, else zero */
static inline int
_hmap_cmp(const hmap_kvp_t *kvp, const char *prefix, const char *key)
{
    if (prefix && kvp->prefix)
        return strcmp(prefix, kvp->prefix) ? 0 : strcmp(key, kvp->key) ? 0 : 1;
    else if (!prefix && !kvp->prefix)
        return strcmp(key, kvp->key) ? 0 : 1;
    else
        return 0;
}

/* The hmap lock should be held while freeing the kvp */
static void
_hmap_free(hmap_kvp_t *kvp)
{
    if (!kvp)
        return;

    /* All values (strings, arrays, pointers) are caller-managed (not freed by hmap) */

    /* Free hmap-managed key and prefix strings */
    if (kvp->key) {
        free(kvp->key);
        kvp->key = NULL;
    }
    if (kvp->prefix) {
        free(kvp->prefix);
        kvp->prefix = NULL;
    }

    kvp->v.u64 = 0;
    kvp->count = 0;
    kvp->type  = HMAP_EMPTY_TYPE;
}

static inline uint32_t
hmap_get_hash(hmap_t *hmap, const char *prefix, const char *key)
{
    return hmap->fns.hash_fn(prefix, key) % hmap->capacity;
}

static inline int
hmap_compare(hmap_t *hmap, const hmap_kvp_t *kvp, const char *prefix, const char *key)
{
    return hmap->fns.cmp_fn(kvp, prefix, key);
}

static inline void
hmap_free_kvp(hmap_t *hmap, hmap_kvp_t *kvp)
{
    hmap->fns.free_fn(kvp);
}

#ifdef __cplusplus
}
#endif
