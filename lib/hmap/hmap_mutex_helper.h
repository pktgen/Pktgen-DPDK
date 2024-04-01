/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2022-2023 Intel Corporation
 */

#ifndef __HMAP_MUTEX_HELPER_H
#define __HMAP_MUTEX_HELPER_H

/**
 * @file
 * Routines to help create a mutex.
 */

#include <pthread.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* __HMAP_MUTEX_HELPER_H */
