/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2021-2024 Intel Corporation
 */

#include <bsd/sys/queue.h>        // for TAILQ_FOREACH_SAFE
#include <bsd/string.h>           // for strlcpy
#include <stdlib.h>               // for free, calloc, malloc, qsort
#include <string.h>               // for strlen, strncmp, strerror, strdup, strnlen
#include <pthread.h>              // for pthread_mutex_init, pthread_mutex_lock

#include <rte_common.h>

#include "hmap.h"
#include "hmap_mutex_helper.h"

static TAILQ_HEAD(pktgen_hmap_list, hmap) hmap_list;
static pthread_mutex_t hmap_list_mutex;

static inline void
hmap_list_lock(void)
{
    int ret = pthread_mutex_lock(&hmap_list_mutex);

    if (ret)
        HMAP_WARN("failed: %s\n", strerror(ret));
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

/* The hmap lock should be held while freeing the kvp */
static void
_hmap_free(hmap_kvp_t *kvp)
{
    if (kvp && (kvp->type != HMAP_EMPTY_TYPE)) {
        /* Assume each was allocated by the system */
        free(kvp->prefix);
        free(kvp->key);

        if (kvp->type == HMAP_STR_TYPE)
            free(kvp->v.str);

        kvp->prefix  = NULL;
        kvp->key     = NULL;
        kvp->v.num64 = 0;
        kvp->type    = HMAP_EMPTY_TYPE;
    }
}

/* Return true or false, true or non-zero for a match, else zero */
static inline int
_hmap_cmp(const char *prefix, const char *key, const hmap_kvp_t *kvp)
{
    if (prefix && kvp->prefix)
        return strncmp(prefix, kvp->prefix, strlen(prefix)) ? 0
               : strncmp(key, kvp->key, strlen(key))        ? 0
                                                            : 1;
    else if (!prefix && !kvp->prefix)
        return strncmp(key, kvp->key, strlen(key)) ? 0 : 1;
    else
        return 0;
}

int
hmap_set_funcs(hmap_t *hmap, hmap_funcs_t *fns)
{
    // clang-format off
    hmap_funcs_t default_funcs = {
        .hash_fn = _hmap_hash,
        .cmp_fn  = _hmap_cmp,
        .free_fn = _hmap_free
    };
    // clang-format on

    if (!hmap)
        return -1;

    if (!fns)
        fns = &default_funcs;

    /*
     * Test each function pointer to make sure the user passed a valid function pointer,
     * if they did not use the default function.
     */
    hmap->fns.hash_fn = (fns->hash_fn) ? fns->hash_fn : default_funcs.hash_fn;
    hmap->fns.free_fn = (fns->free_fn) ? fns->free_fn : default_funcs.free_fn;
    hmap->fns.cmp_fn  = (fns->cmp_fn) ? fns->cmp_fn : default_funcs.cmp_fn;

    return 0;
}

hmap_funcs_t *
hmap_get_funcs(hmap_t *hmap)
{
    return (hmap) ? &hmap->fns : NULL;
}

/* Hmap's need a hash function, an equality function, and a destructor */
hmap_t *
hmap_create(const char *name, uint32_t max_capacity, hmap_funcs_t *funcs)
{
    hmap_t *hmap = NULL;

    if (!name)
        name = "HMAP";

    if (strnlen(name, HMAP_MAX_NAME_SIZE) >= HMAP_MAX_NAME_SIZE)
        HMAP_NULL_RET("Name is greater than or equal to %d bytes\n", HMAP_MAX_NAME_SIZE);

    hmap = calloc(1, sizeof(hmap_t));
    if (!hmap)
        HMAP_ERR_GOTO(err_leave, "Failed to allocate hmap structure\n");

    strlcpy(hmap->name, name, sizeof(hmap->name));

    /*
     * if max_capacity is zero, set to HMAP_DEFAULT_CAPACITY
     * if max_capacity is less than HMAP_STARTING_CAPACITY, set to HMAP_STARTING_CAPACITY
     */
    if (max_capacity == 0)
        max_capacity = HMAP_DEFAULT_CAPACITY;
    if (max_capacity < HMAP_STARTING_CAPACITY)
        max_capacity = HMAP_STARTING_CAPACITY;

    hmap->capacity     = HMAP_STARTING_CAPACITY;
    hmap->max_capacity = max_capacity;

    /* Start out with the initial capacity */
    hmap->map = calloc(hmap->capacity, sizeof(struct hmap_kvp));
    if (!hmap->map)
        HMAP_ERR_GOTO(err_leave, "Failed to allocate KVP for %d capacity\n", hmap->capacity);

    if (hmap_set_funcs(hmap, funcs) < 0)
        HMAP_ERR_GOTO(err_leave, "Failed to set function pointers\n");

    if (hmap_mutex_create(&hmap->mutex, PTHREAD_MUTEX_RECURSIVE) < 0)
        HMAP_ERR_GOTO(err_leave, "mutex init(hmap->mutex) failed: %s\n", strerror(errno));

    hmap_list_lock();
    TAILQ_INSERT_TAIL(&hmap_list, hmap, next);
    hmap_list_unlock();

    return hmap;

err_leave:
    if (hmap) {
        free(hmap->map);
        free(hmap);
    }

    return NULL;
}

int
hmap_destroy(hmap_t *hmap)
{
    if (!hmap)
        return 0;

    hmap_lock(hmap);

    if (hmap->map) {
        struct hmap_kvp *kvp = hmap->map;

        for (uint32_t i = 0; i < hmap->capacity; i++, kvp++)
            hmap->fns.free_fn(kvp);

        /* Indicate the hmap is not usable anymore, possible race condition */
        hmap->max_capacity = hmap->capacity = hmap->curr_capacity = 0;

        free(hmap->map);
    }
    hmap_list_lock();
    TAILQ_REMOVE(&hmap_list, hmap, next);
    hmap_list_unlock();

    hmap_unlock(hmap);

    if (hmap_mutex_destroy(&hmap->mutex))
        HMAP_ERR_RET("Destroy of mutex failed\n");

    free(hmap);

    return 0;
}

int
hmap_destroy_by_name(const char *name)
{
    hmap_t *hmap, *tvar;

    TAILQ_FOREACH_SAFE (hmap, &hmap_list, next, tvar) {
        if (!strncmp(name, hmap->name, strlen(name)))
            return hmap_destroy(hmap);
    }

    return -1;
}

int
hmap_kvp_update(hmap_t *hmap, hmap_kvp_t *kvp, hmap_val_t *val)
{
    if (!hmap || !kvp || !val)
        return -1;

    hmap_lock(hmap);

    // clang-format off
    switch(kvp->type) {
    case HMAP_EMPTY_TYPE:   break;
    case HMAP_STR_TYPE:
        if (kvp->v.str)
            free(kvp->v.str);
        kvp->v.str = strdup(val->str);
        break;
    case HMAP_U64_TYPE:     kvp->v.u64 = val->u64; break;
    case HMAP_U32_TYPE:     kvp->v.u32 = val->u32; break;
    case HMAP_U16_TYPE:     kvp->v.u16 = val->u16; break;
    case HMAP_U8_TYPE:      kvp->v.u8 = val->u8; break;
    case HMAP_NUM_TYPE:     kvp->v.num = val->num; break;
    case HMAP_NUM64_TYPE:   kvp->v.num64 = val->num64; break;
    case HMAP_BOOLEAN_TYPE: kvp->v.boolean = val->boolean; break;
    case HMAP_POINTER_TYPE: kvp->v.ptr = val->ptr; break;
    default:
        hmap_unlock(hmap);
        return -1;
    }
    // clang-format on

    hmap_unlock(hmap);

    return 0;
}

/* Open addressed insertion function */
static inline int
__add_value(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key, hmap_val_t *val)
{
    uint32_t hash = hmap->fns.hash_fn(prefix, key) % hmap->capacity;

    for (uint32_t i = 0; i < hmap->capacity; i++) {
        struct hmap_kvp *kvp;

        kvp = &hmap->map[hash];
        if (kvp->type == HMAP_EMPTY_TYPE) {
            kvp->key = strdup(key);
            if (prefix)
                kvp->prefix = strdup(prefix);

            kvp->type = type;
            if (hmap_kvp_update(hmap, kvp, val)) {
                hmap->fns.free_fn(kvp);
                return -1;
            }

            hmap->curr_capacity++;
            return 0;
        }
        hash = (hash + 1) % hmap->capacity;
    }
    return -1;
}

int
hmap_add(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key, hmap_val_t *val)
{
    hmap_kvp_t *kvp = NULL;
    int ret         = -1;

    if (!hmap || !key)
        return ret;

    hmap_lock(hmap);

    /* Update entry if found */
    if ((kvp = hmap_kvp_lookup(hmap, prefix, key)) != NULL) {
        hmap_kvp_update(hmap, kvp, val);
        hmap_unlock(hmap);
        return 0;
    }

    /* Increase the map if free space becomes too small and does not exceed max_capacity */
    if ((hmap->curr_capacity >= (hmap->capacity - (HMAP_STARTING_CAPACITY / 4))) &&
        ((hmap->capacity + HMAP_STARTING_CAPACITY) <= hmap->max_capacity)) {
        struct hmap_kvp *kvp, *old_map, *new_map;
        uint32_t old_capacity;

        old_capacity = hmap->capacity;
        old_map      = hmap->map;

        hmap->capacity += HMAP_STARTING_CAPACITY;

        new_map = calloc(hmap->capacity, sizeof(struct hmap_kvp));
        if (new_map == NULL) {
            hmap->capacity = old_capacity;
            goto try_update;
        }
        hmap->map           = new_map;
        hmap->curr_capacity = 0;

        /* Add all of the old values into the new map. */
        kvp = old_map;
        for (uint32_t i = 0; i < old_capacity; i++, kvp++) {
            if (kvp->type == HMAP_EMPTY_TYPE)
                continue;
            if (__add_value(hmap, kvp->type, kvp->prefix, kvp->key, &kvp->v)) {
                free(new_map);
                hmap->map      = old_map;
                hmap->capacity = old_capacity;
                goto try_update;
            }
        }
        free(old_map);
    }
try_update:
    ret = __add_value(hmap, type, prefix, key, val);

    hmap_unlock(hmap);
    return ret;
}

hmap_kvp_t *
hmap_kvp_lookup(hmap_t *hmap, const char *prefix, const char *key)
{
    if (hmap && hmap->map && key) {
        uint32_t hash = hmap->fns.hash_fn(prefix, key) % hmap->capacity;

        hmap_lock(hmap);
        for (uint32_t i = 0; i < hmap->capacity; i++) {
            struct hmap_kvp *kvp = &hmap->map[hash];

            if (kvp->key && hmap->fns.cmp_fn(prefix, key, kvp)) {
                hmap_unlock(hmap);
                return kvp;
            }

            hash = (hash + 1) % hmap->capacity;
        }
        hmap_unlock(hmap);
    }

    return NULL;
}

int
hmap_lookup(hmap_t *hmap, const char *prefix, const char *key, hmap_val_t *val)
{
    if (hmap && key) {
        uint32_t hash = hmap->fns.hash_fn(prefix, key) % hmap->capacity;

        hmap_lock(hmap);
        for (uint32_t i = 0; i < hmap->capacity; i++) {
            struct hmap_kvp *kvp = &hmap->map[hash];

            if (kvp->key && hmap->fns.cmp_fn(prefix, key, kvp)) {
                if (val)
                    val->u64 = kvp->v.u64;
                hmap_unlock(hmap);
                return 0;
            }

            hash = (hash + 1) % hmap->capacity;
        }
        hmap_unlock(hmap);
    }

    return -1;
}

int
hmap_del(hmap_t *hmap, const char *prefix, const char *key)
{
    if (hmap && key) {
        hmap_kvp_t *kvp = hmap_kvp_lookup(hmap, prefix, key);

        hmap->fns.free_fn(kvp);
        hmap->curr_capacity--;
        return 0;
    }
    return -1;
}

int
hmap_iterate(hmap_t *hmap, struct hmap_kvp **_kvp, uint32_t *next)
{
    if (hmap && hmap->map && next) {
        hmap_lock(hmap);
        for (uint32_t i = *next; i < hmap->capacity; i++) {
            struct hmap_kvp *kvp = &hmap->map[i];

            if (!kvp->key)
                continue;
            *next = ++i;

            if (_kvp)
                *_kvp = kvp;
            hmap_unlock(hmap);
            return 1;
        }
        hmap_unlock(hmap);
    }
    return 0;
}

static int
kvp_cmp(const void *p1, const void *p2)
{
    struct hmap_kvp *k1 = *(struct hmap_kvp *const *)p1;
    struct hmap_kvp *k2 = *(struct hmap_kvp *const *)p2;

    if (k1->prefix && k2->prefix) {
        int ret = strncmp(k1->prefix, k2->prefix, strlen(k1->prefix));

        return ret ? ret : strncmp(k1->key, k2->key, strlen(k1->key));
    } else if (!k1->prefix && !k2->prefix)
        return strncmp(k1->key, k2->key, strlen(k1->key));
    else
        return (k1->prefix && !k2->prefix) ? 1 : -1;
}

static void
_print_kvp(FILE *f, hmap_kvp_t *kvp)
{
    char buff[128];

    if (!kvp)
        return;
    if (kvp->prefix)
        snprintf(buff, sizeof(buff), "%s:%s", kvp->prefix, kvp->key);
    else
        snprintf(buff, sizeof(buff), "%s", kvp->key);

    fprintf(f, "Key: %-42s  value: ", buff);

    switch (kvp->type) {
    case HMAP_EMPTY_TYPE:
        fprintf(f, "Empty\n");
        break;
    case HMAP_BOOLEAN_TYPE:
        fprintf(f, "'%s'\n", kvp->v.boolean ? "true" : "false");
        break;
    case HMAP_U64_TYPE:
        fprintf(f, "%lu\n", kvp->v.u64);
        break;
    case HMAP_U32_TYPE:
        fprintf(f, "%u\n", kvp->v.u32);
        break;
    case HMAP_U16_TYPE:
        fprintf(f, "%u\n", kvp->v.u16);
        break;
    case HMAP_U8_TYPE:
        fprintf(f, "%c(%u)\n", kvp->v.u8, kvp->v.u8);
        break;
    case HMAP_NUM_TYPE:
        fprintf(f, "%d\n", kvp->v.num);
        break;
    case HMAP_NUM64_TYPE:
        fprintf(f, "%ld\n", kvp->v.num64);
        break;
    case HMAP_STR_TYPE:
        fprintf(f, "'%s'\n", kvp->v.str);
        break;
    case HMAP_POINTER_TYPE:
        fprintf(f, "%p\n", kvp->v.ptr);
        break;
    default:
        HMAP_ERR("Unknown type %d\n", kvp->type);
        break;
    }
}

void
hmap_dump(FILE *f, hmap_t *hmap, int sort)
{
    uint32_t next = 0;
    hmap_kvp_t *kvp;

    if (!hmap)
        return;
    if (!f)
        f = stdout;

    fprintf(f, "\n**** Hashmap dump (%s) %s ****\n", hmap->name, sort ? "Sorted" : "Not Sorted");

    hmap_lock(hmap);
    if (sort == 0) {
        int i = 0;

        while (hmap_iterate(hmap, &kvp, &next)) {
            fprintf(f, "  %5d: ", i++);
            _print_kvp(f, kvp);
        }
    } else {
        struct hmap_kvp **kvp_list;
        int cnt = hmap_count(hmap), i;

        kvp_list = (struct hmap_kvp **)malloc(cnt * sizeof(struct hmap_kvp *));
        if (!kvp_list)
            return;

        i = 0;
        while (hmap_iterate(hmap, &kvp, &next))
            kvp_list[i++] = kvp;

        qsort(kvp_list, cnt, sizeof(struct hmap_kvp *), kvp_cmp);

        for (i = 0; i < cnt; i++) {
            fprintf(f, "  %5d: ", i);
            _print_kvp(f, kvp_list[i]);
        }

        free(kvp_list);
    }
    hmap_unlock(hmap);

    fprintf(f, "Total count: %u\n", hmap_count(hmap));
}

void
hmap_dump_map(FILE *f, const char *name, int sort)
{
    hmap_t *hmap, *tvar = NULL;

    if (!f)
        f = stdout;

    TAILQ_FOREACH_SAFE (hmap, &hmap_list, next, tvar) {
        if (!strncmp(hmap->name, name, strlen(name)))
            hmap_dump(f, hmap, sort);
    }
}

void
hmap_dump_all(FILE *f, int sort)
{
    hmap_t *hmap, *tvar = NULL;

    if (!f)
        f = stdout;

    TAILQ_FOREACH_SAFE (hmap, &hmap_list, next, tvar)
        hmap_dump(f, hmap, sort);
}

void
hmap_list_maps(FILE *f)
{
    hmap_t *hmap, *tvar;

    if (!f)
        f = stdout;
    fprintf(f, "\n**** Hashmap list ****\n");
    TAILQ_FOREACH_SAFE (hmap, &hmap_list, next, tvar)
        fprintf(f, "  %s\n", hmap->name);
}

RTE_INIT_PRIO(hmap_constructor, LOG)
{
    TAILQ_INIT(&hmap_list);

    if (hmap_mutex_create(&hmap_list_mutex, PTHREAD_MUTEX_RECURSIVE) < 0)
        HMAP_RET("mutex init(hmap_list_mutex) failed\n");
}
