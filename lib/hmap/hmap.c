/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2016-2023 Intel Corporation
 */

#include <stdlib.h>               // for free, calloc, malloc, qsort
#include <string.h>               // for strlen, strncmp, strerror, strnlen
#include <errno.h>                // for EEXIST
#include <bsd/sys/queue.h>        // for TAILQ_FOREACH_SAFE
#include <bsd/string.h>           // for strlcpy
#include <rte_log.h>

#include "hmap.h"
#include "hmap_log.h"
#include "hmap_helper.h"

/* Allocation tracking for performance measurement */
#ifdef HMAP_TRACK_ALLOCS
static _Atomic uint64_t hmap_alloc_count   = 0;
static _Atomic uint64_t hmap_realloc_count = 0;
static _Atomic uint64_t hmap_total_bytes   = 0;
#define HMAP_ALLOC_INC()   (__atomic_add_fetch(&hmap_alloc_count, 1, __ATOMIC_SEQ_CST))
#define HMAP_REALLOC_INC() (__atomic_add_fetch(&hmap_realloc_count, 1, __ATOMIC_SEQ_CST))
#define HMAP_BYTES_ADD(n)  (__atomic_add_fetch(&hmap_total_bytes, (n), __ATOMIC_SEQ_CST))
#else
#define HMAP_ALLOC_INC() \
    do {                 \
    } while (0)
#define HMAP_REALLOC_INC() \
    do {                   \
    } while (0)
#define HMAP_BYTES_ADD(n) \
    do {                  \
    } while (0)
#endif

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

    HMAP_ALLOC_INC();
    HMAP_BYTES_ADD(sizeof(hmap_t));

    strlcpy(hmap->name, name, sizeof(hmap->name));

    /*
     * if max_capacity is zero, set to HMAP_DEFAULT_CAPACITY
     * if max_capacity is less than HMAP_STARTING_CAPACITY, set to HMAP_STARTING_CAPACITY
     */
    if (max_capacity == HMAP_USE_DEFAULT_CAPACITY)
        max_capacity = HMAP_DEFAULT_CAPACITY;
    if (max_capacity < HMAP_STARTING_CAPACITY)
        max_capacity = HMAP_STARTING_CAPACITY;

    hmap->capacity = max_capacity / 4;
    if (hmap->capacity == 0)
        hmap->capacity = HMAP_STARTING_CAPACITY;
    hmap->max_capacity = max_capacity;
    hmap->alloc_count  = 0;

    /* Start out with the initial capacity */
    hmap->map = calloc(hmap->capacity, sizeof(struct hmap_kvp));
    if (!hmap->map)
        HMAP_ERR_GOTO(err_leave, "Failed to allocate KVP for %d capacity\n", hmap->capacity);

    HMAP_ALLOC_INC();
    HMAP_BYTES_ADD(hmap->capacity * sizeof(struct hmap_kvp));
    hmap->alloc_count++;

    if (hmap_set_funcs(hmap, funcs) < 0)
        HMAP_ERR_GOTO(err_leave, "Failed to set function pointers\n");

    if (hmap_mutex_create(&hmap->mutex, PTHREAD_MUTEX_RECURSIVE_NP) < 0)
        HMAP_ERR_GOTO(err_leave, "mutex init(hmap->mutex) failed: %s\n", strerror(errno));

    hmap_list_lock();
    TAILQ_INSERT_TAIL(&hmap_registry, hmap, next);
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
    TAILQ_REMOVE(&hmap_registry, hmap, next);
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

    TAILQ_FOREACH_SAFE (hmap, &hmap_registry, next, tvar) {
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

    /*
     * Values are caller-managed (strings, arrays, pointers).
     * Updating an entry only overwrites the stored pointer/64-bit value.
     */
    kvp->v = *val;

    /* Keep array count unchanged (set by add/update array helpers). */
    if (kvp->type < HMAP_STR_ARRAY_TYPE)
        kvp->count = 0;

    hmap_unlock(hmap);

    return 0;
}

/* Open addressed insertion function */
static inline int
__add_value(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key, hmap_val_t *val)
{
    uint32_t hash = hmap_get_hash(hmap, prefix, key);

    for (uint32_t i = 0; i < hmap->capacity; i++) {
        struct hmap_kvp *kvp;

        kvp = &hmap->map[hash];
        if (kvp->type == HMAP_EMPTY_TYPE) {
            /* Duplicate key and prefix strings - hmap now owns them */
            kvp->key = strdup(key);
            if (!kvp->key)
                return -1;

            if (prefix) {
                kvp->prefix = strdup(prefix);
                if (!kvp->prefix) {
                    free(kvp->key);
                    kvp->key = NULL;
                    return -1;
                }
            }

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

static int
_hmap_update_capacity(hmap_t *hmap)
{
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
            return -1;
        }

        HMAP_REALLOC_INC();
        HMAP_BYTES_ADD(hmap->capacity * sizeof(struct hmap_kvp));
        hmap->alloc_count++;

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
                return -1;
            }
        }
        free(old_map);
    }

    return 0;
}

int
hmap_add(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key, hmap_val_t *val)
{
    int ret = -1;

    if (!hmap || !key)        // prefix can be NULL
        return ret;

    /* Do not allow for duplicates in the hash map */
    if (hmap_kvp_lookup(hmap, prefix, key))
        return ret;

    hmap_lock(hmap);

    if ((ret = _hmap_update_capacity(hmap)) == 0)
        ret = __add_value(hmap, type, prefix, key, val);

    hmap_unlock(hmap);
    return ret;
}

int
hmap_update(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key, hmap_val_t *val)
{
    hmap_kvp_t *kvp;
    int ret = -1;

    if (!hmap || !key)        // prefix can be NULL
        return ret;

    hmap_lock(hmap);

    // Update existing key value if it exists
    if ((kvp = hmap_kvp_lookup(hmap, prefix, key)) != NULL) {
        if (kvp->type != type)
            ret = -1;
        else
            ret = hmap_kvp_update(hmap, kvp, val);
    } else {
        // Otherwise add new key/value pair
        if ((ret = _hmap_update_capacity(hmap)) == 0)
            ret = __add_value(hmap, type, prefix, key, val);
    }

    hmap_unlock(hmap);
    return ret;
}

hmap_kvp_t *
hmap_kvp_lookup(hmap_t *hmap, const char *prefix, const char *key)
{
    if (hmap && hmap->map && key) {
        uint32_t hash = hmap_get_hash(hmap, prefix, key);

        hmap_lock(hmap);
        for (uint32_t i = 0; i < hmap->capacity; i++) {
            struct hmap_kvp *kvp = &hmap->map[hash];

            if (kvp->key && hmap->fns.cmp_fn(kvp, prefix, key)) {
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
        uint32_t hash = hmap_get_hash(hmap, prefix, key);

        hmap_lock(hmap);
        for (uint32_t i = 0; i < hmap->capacity; i++) {
            struct hmap_kvp *kvp = &hmap->map[hash];

            if (kvp->key && hmap->fns.cmp_fn(kvp, prefix, key)) {
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

    // First compare prefixes (primary sort key)
    if (k1->prefix && k2->prefix) {
        // Both have prefixes, compare them first
        int prefix_cmp = strcmp(k1->prefix, k2->prefix);
        if (prefix_cmp != 0)
            return prefix_cmp;
        // Prefixes are equal, compare keys as secondary sort
        return strcmp(k1->key, k2->key);
    } else if (!k1->prefix && !k2->prefix) {
        // Neither has prefix, just compare keys
        return strcmp(k1->key, k2->key);
    } else {
        // One has prefix, one doesn't - items with prefixes come first
        return (k1->prefix && !k2->prefix) ? -1 : 1;
    }
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

    fprintf(f, "%-42s ", buff);

    switch (kvp->type) {
    case HMAP_EMPTY_TYPE:
        fprintf(f, "%-8s:\n", "Empty");
        break;
    case HMAP_U64_TYPE:
        fprintf(f, "%-8s: %lu\n", "u64", kvp->v.u64);
        break;
    case HMAP_U32_TYPE:
        fprintf(f, "%-8s: %u\n", "u32", (uint32_t)kvp->v.u64);
        break;
    case HMAP_U16_TYPE:
        fprintf(f, "%-8s: %u\n", "u16", (uint16_t)kvp->v.u64);
        break;
    case HMAP_U8_TYPE: {
        uint8_t val = (uint8_t)kvp->v.u64;
        fprintf(f, "%-8s: %c(%u)\n", "u8", (val >= ' ' && val <= '~') ? val : '.', val);
    } break;
    case HMAP_I64_TYPE:
        fprintf(f, "%-8s: %ld\n", "i64", (int64_t)kvp->v.u64);
        break;
    case HMAP_I32_TYPE:
        fprintf(f, "%-8s: %d\n", "i32", (int32_t)kvp->v.u64);
        break;
    case HMAP_I16_TYPE:
        fprintf(f, "%-8s: %d\n", "i16", (int16_t)kvp->v.u64);
        break;
    case HMAP_I8_TYPE:
        fprintf(f, "%-8s: %d\n", "i8", (int8_t)kvp->v.u64);
        break;
    case HMAP_DOUBLE_TYPE:
        fprintf(f, "%-8s: %g\n", "double", kvp->v.dval);
        break;
    case HMAP_STR_TYPE:
        fprintf(f, "%-8s: '%s'\n", "String", (char *)kvp->v.ptr);
        break;
    case HMAP_POINTER_TYPE:
        fprintf(f, "%-8s: %p\n", "Pointer", kvp->v.ptr);
        break;
    case HMAP_STR_ARRAY_TYPE: {
        char **strs = (char **)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "Strings", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "          %3u: '%s'\n", i, strs[i]);
    } break;
    case HMAP_U64_ARRAY_TYPE: {
        uint64_t *vals = (uint64_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "u64s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "         %3u: %lu\n", i, vals[i]);
    } break;
    case HMAP_U32_ARRAY_TYPE: {
        uint32_t *vals = (uint32_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "u32s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "         %3u: %u\n", i, vals[i]);
    } break;
    case HMAP_U16_ARRAY_TYPE: {
        uint16_t *vals = (uint16_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "u16s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "         %3u: %u\n", i, vals[i]);
    } break;
    case HMAP_U8_ARRAY_TYPE: {
        uint8_t *vals = (uint8_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "u8s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "         %3u: %c(%u)\n", i,
                    (vals[i] >= ' ' && vals[i] <= '~') ? vals[i] : '.', vals[i]);
    } break;
    case HMAP_I64_ARRAY_TYPE: {
        int64_t *vals = (int64_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "I64s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "          %3u: %ld\n", i, vals[i]);
    } break;
    case HMAP_I32_ARRAY_TYPE: {
        int32_t *vals = (int32_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "I32s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "          %3u: %d\n", i, vals[i]);
    } break;
    case HMAP_I16_ARRAY_TYPE: {
        int16_t *vals = (int16_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "I16s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "          %3u: %d\n", i, vals[i]);
    } break;
    case HMAP_I8_ARRAY_TYPE: {
        int8_t *vals = (int8_t *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "I8s", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "          %3u: %d\n", i, vals[i]);
    } break;
    case HMAP_DOUBLE_ARRAY_TYPE: {
        double *vals = (double *)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "Doubles", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "          %3u: %g\n", i, vals[i]);
    } break;
    case HMAP_PTR_ARRAY_TYPE: {
        void **vals = (void **)kvp->v.ptr;
        fprintf(f, "%-8s: (%u)\n", "Pointers", kvp->count);
        for (uint32_t i = 0; i < kvp->count; i++)
            fprintf(f, "          %3u: %p\n", i, vals[i]);
    } break;
    default:
        HMAP_ERR("*** Unknown type %d\n", kvp->type);
        break;
    }
}

void
hmap_list(FILE *f, hmap_t *hmap, bool sort)
{
    uint32_t next = 0;
    hmap_kvp_t *kvp;

    if (!hmap)
        return;
    if (!f)
        f = stdout;

    fprintf(f, "\n**** Hashmap dump (%s) %s ****\n", hmap->name, sort ? "Sorted" : "Not Sorted");
    fprintf(f, " %-5s: %-42s %s\n", "Index", "Prefix:Key", "Type:Value");

    if (sort) {
        // Sorted output using qsort
        struct hmap_kvp **kvp_list;
        int cnt = hmap_count(hmap);

        kvp_list = (struct hmap_kvp **)malloc(cnt * sizeof(struct hmap_kvp *));
        if (!kvp_list)
            return;

        for (int i = 0; hmap_iterate(hmap, &kvp, &next); i++)
            kvp_list[i] = kvp;

        qsort(kvp_list, cnt, sizeof(struct hmap_kvp *), kvp_cmp);

        for (int i = 0; i < cnt; i++) {
            fprintf(f, " %5d: ", i);
            _print_kvp(f, kvp_list[i]);
        }

        free(kvp_list);
    } else {
        // Unsorted output using simple iteration
        for (int i = 0; hmap_iterate(hmap, &kvp, &next); i++) {
            fprintf(f, " %5d: ", i);
            _print_kvp(f, kvp);
        }
    }

    fprintf(f, "Total count: %u\n", hmap_count(hmap));
}

void
hmap_list_names(FILE *f)
{
    hmap_t *hmap, *tvar;

    if (!f)
        f = stdout;

    printf("\n**** Hashmap List ****\n");
    TAILQ_FOREACH_SAFE (hmap, &hmap_registry, next, tvar)
        printf("   %s\n", hmap->name);
}

void
hmap_list_by_name(FILE *f, char *name, bool sort)
{
    hmap_t *hmap, *tvar;

    if (!f)
        f = stdout;

    printf("\n**** Hashmap List ****\n");
    TAILQ_FOREACH_SAFE (hmap, &hmap_registry, next, tvar)
        if (!strncmp(name, hmap->name, strlen(name)))
            hmap_list(f, hmap, sort);
}

void
hmap_list_all(FILE *f, bool sort)
{
    hmap_t *hmap, *tvar;

    if (!f)
        f = stdout;

    TAILQ_FOREACH_SAFE (hmap, &hmap_registry, next, tvar)
        hmap_list(f, hmap, sort);
}

RTE_INIT_PRIO(hmap_constructor, LOG)
{
    TAILQ_INIT(&hmap_registry);

    if (hmap_mutex_create(&hmap_list_mutex, PTHREAD_MUTEX_RECURSIVE_NP) < 0)
        HMAP_RET("mutex init(hmap_list_mutex) failed\n");
}

/* Allocation tracking functions - always available */
void
hmap_get_global_alloc_stats(uint64_t *alloc_count, uint64_t *realloc_count, uint64_t *total_bytes)
{
#ifdef HMAP_TRACK_ALLOCS
    if (alloc_count)
        *alloc_count = __atomic_load_n(&hmap_alloc_count, __ATOMIC_SEQ_CST);
    if (realloc_count)
        *realloc_count = __atomic_load_n(&hmap_realloc_count, __ATOMIC_SEQ_CST);
    if (total_bytes)
        *total_bytes = __atomic_load_n(&hmap_total_bytes, __ATOMIC_SEQ_CST);
#else
    if (alloc_count)
        *alloc_count = 0;
    if (realloc_count)
        *realloc_count = 0;
    if (total_bytes)
        *total_bytes = 0;
#endif
}

void
hmap_reset_global_alloc_stats(void)
{
#ifdef HMAP_TRACK_ALLOCS
    __atomic_store_n(&hmap_alloc_count, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&hmap_realloc_count, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&hmap_total_bytes, 0, __ATOMIC_SEQ_CST);
#endif
}
