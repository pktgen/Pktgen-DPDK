/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2022-2026 Intel Corporation
 */

#pragma once

/**
 * @file
 *
 * HMAP internal helper routines: mutex creation/destruction, the global
 * hmap registry, per-hmap locking, hash computation, key comparison, and
 * KVP memory management.
 */

#include <errno.h>
#include <pthread.h>
#include <rte_log.h>
#include "hmap_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Global TAILQ registry of all created hmap instances. */
static TAILQ_HEAD(hmap_hmap_list, hmap) hmap_registry;
/** Mutex protecting @ref hmap_registry. */
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

/**
 * Acquire the global hmap registry mutex.
 *
 * Logs an error if the lock cannot be obtained.
 */
static inline void
hmap_list_lock(void)
{
    int ret = pthread_mutex_lock(&hmap_list_mutex);

    if (ret)
        HMAP_LOG("failed: %s\n", strerror(ret));
}

/**
 * Release the global hmap registry mutex.
 *
 * Logs a warning if the unlock fails.
 */
static inline void
hmap_list_unlock(void)
{
    int ret = pthread_mutex_unlock(&hmap_list_mutex);

    if (ret)
        HMAP_WARN("failed: %s\n", strerror(ret));
}

/**
 * Acquire the per-hmap mutex.
 *
 * @param hmap
 *   Pointer to the hmap whose mutex should be locked.
 */
static inline void
hmap_lock(hmap_t *hmap)
{
    int ret = pthread_mutex_lock(&hmap->mutex);

    if (ret)
        HMAP_WARN("failed: %s\n", strerror(ret));
}

/**
 * Release the per-hmap mutex.
 *
 * @param hmap
 *   Pointer to the hmap whose mutex should be unlocked.
 */
static inline void
hmap_unlock(hmap_t *hmap)
{
    int ret = pthread_mutex_unlock(&hmap->mutex);

    if (ret)
        HMAP_WARN("failed: %s\n", strerror(ret));
}

/**
 * Default hash function used when no custom hash is provided.
 *
 * Computes a Jenkins-style one-at-a-time hash over the concatenation of
 * @p prefix (if non-NULL) and @p key.
 *
 * @param prefix
 *   Optional namespace prefix string, or NULL.
 * @param key
 *   Key string to hash.
 * @return
 *   32-bit hash value.
 */
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

/**
 * Default key comparison function used when no custom comparator is provided.
 *
 * Returns non-zero if @p kvp matches the given prefix and key pair, zero
 * otherwise. Both prefix and kvp->prefix must be NULL or non-NULL together
 * for a match to occur.
 *
 * @param kvp
 *   Key-value pair to compare against.
 * @param prefix
 *   Optional namespace prefix, or NULL.
 * @param key
 *   Key string to match.
 * @return
 *   Non-zero on match, 0 on mismatch.
 */
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

/**
 * Free hmap-managed storage within a KVP and reset its fields.
 *
 * Releases the key and prefix strings allocated by hmap. Caller-managed
 * value storage (arrays, pointers) is NOT freed here. The hmap lock must
 * be held by the caller before invoking this function.
 *
 * @param kvp
 *   Pointer to the key-value pair to free. If NULL, the call is a no-op.
 */
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

/**
 * Compute a bucket index for the given prefix/key pair using the hmap's hash function.
 *
 * @param hmap
 *   Pointer to the hmap.
 * @param prefix
 *   Optional namespace prefix, or NULL.
 * @param key
 *   Key string.
 * @return
 *   Bucket index in the range [0, hmap->capacity).
 */
static inline uint32_t
hmap_get_hash(hmap_t *hmap, const char *prefix, const char *key)
{
    return hmap->fns.hash_fn(prefix, key) % hmap->capacity;
}

/**
 * Compare a KVP against the given prefix/key pair using the hmap's comparator.
 *
 * @param hmap
 *   Pointer to the hmap.
 * @param kvp
 *   Key-value pair to compare.
 * @param prefix
 *   Optional namespace prefix, or NULL.
 * @param key
 *   Key string.
 * @return
 *   Non-zero on match, 0 on mismatch.
 */
static inline int
hmap_compare(hmap_t *hmap, const hmap_kvp_t *kvp, const char *prefix, const char *key)
{
    return hmap->fns.cmp_fn(kvp, prefix, key);
}

/**
 * Free a KVP using the hmap's registered free function.
 *
 * @param hmap
 *   Pointer to the hmap.
 * @param kvp
 *   Pointer to the key-value pair to free.
 */
static inline void
hmap_free_kvp(hmap_t *hmap, hmap_kvp_t *kvp)
{
    hmap->fns.free_fn(kvp);
}

#ifdef __cplusplus
}
#endif
