/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2019-2023 Intel Corporation
 */

#pragma once

/**
 * @file
 *
 * This library provides an API for the hashmap data structure.
 *
 * PUBLIC API:
 * The hmap library exposes a minimal, type-safe public API using C11 _Generic macros:
 *   - hmap_add_value()    - Add scalar values (automatically dispatches by value type)
 *   - hmap_add_array()    - Add array values (automatically dispatches by array type)
 *   - hmap_update_value() - Update-or-add scalar values (automatically dispatches by value type)
 *   - hmap_update_array() - Update-or-add array values (automatically dispatches by array type)
 *   - hmap_get_value()    - Retrieve scalar values (automatically dispatches by pointer type)
 *   - hmap_get_array()    - Retrieve array values (automatically dispatches by pointer type)
 *
 * INTERNAL FUNCTIONS:
 * Type-specific functions (e.g., _hmap_add_string, _hmap_get_u32) are prefixed with
 * underscore and are not part of the public API. They should not be called directly.
 */

// IWYU pragma: no_include <bits/stdint-uintn.h>

#include <stdbool.h>          // for bool
#include <string.h>           // for NULL
#include <strings.h>          // for strcasecmp
#include <stdint.h>           // for uint32_t, int64_t, uint16_t, uint64_t, uint8_t
#include <stdio.h>            // for FILE
#include <sys/queue.h>        // for TAILQ_ENTRY
#include <pthread.h>          // for pthread_mutex_t

#include "rte_common.h"
#include "rte_log.h"
#include "hmap_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HMAP_MAX_NAME_SIZE        32   /**< MAX size of HMAP name */
#define HMAP_MAX_KEY_SIZE         256  /**< Key size */
#define HMAP_STARTING_CAPACITY    32   /**< Starting capacity not exceeding max_capacity */
#define HMAP_DEFAULT_CAPACITY     1024 /**< Default starting capacity not exceeding max_capacity */
#define HMAP_USE_DEFAULT_CAPACITY 0    /**< Use the default capacity value */

struct hmap;

// clang-format off
typedef enum {
    HMAP_EMPTY_TYPE,
    HMAP_STR_TYPE,
    HMAP_U64_TYPE,
    HMAP_U32_TYPE,
    HMAP_U16_TYPE,
    HMAP_U8_TYPE,
    HMAP_I64_TYPE,
    HMAP_I32_TYPE,
    HMAP_I16_TYPE,
    HMAP_I8_TYPE,
    HMAP_DOUBLE_TYPE,
    HMAP_POINTER_TYPE,
	HMAP_STR_ARRAY_TYPE,
	HMAP_U64_ARRAY_TYPE,
	HMAP_U32_ARRAY_TYPE,
    HMAP_U16_ARRAY_TYPE,
    HMAP_U8_ARRAY_TYPE,
    HMAP_I64_ARRAY_TYPE,
    HMAP_I32_ARRAY_TYPE,
    HMAP_I16_ARRAY_TYPE,
    HMAP_I8_ARRAY_TYPE,
    HMAP_DOUBLE_ARRAY_TYPE,
    HMAP_PTR_ARRAY_TYPE,
    HMAP_NUM_TYPES
} hmap_type_t;
// clang-format on

/**
 * @brief Unified value storage using single 64-bit value
 *
 * All scalar types stored directly, strings/pointers as uintptr_t, arrays via pointer
 */
typedef union {
    uint64_t u64; /**< Universal 64-bit storage for all types */
    double dval;  /**< Double value (64-bit IEEE 754) */
    void *ptr;    /**< Generic pointer (for strings, pointers, arrays) */
} hmap_val_t;

/**
 * @brief Key/value pair stored in the hashmap table.
 */
typedef struct hmap_kvp { /**< Key/value pair (implementation-defined size) */
    hmap_type_t type;     /**< Type of the value stored in kvp */
    uint32_t count;       /**< Array element count (0 for non-arrays, uses padding slot) */
    char *prefix;         /**< Prefix string value (hmap-managed, duplicated internally) */
    char *key;            /**< String key pointer (hmap-managed, duplicated internally) */
    hmap_val_t v;         /**< Values stored in kvp */
} hmap_kvp_t;

typedef uint32_t (*hash_fn_t)(const char *prefix, const char *key);
typedef int (*cmp_fn_t)(const hmap_kvp_t *kvp, const char *prefix, const char *key);
typedef void (*free_fn_t)(hmap_kvp_t *kvp);

typedef struct hmap_funcs {
    hash_fn_t hash_fn; /**< Hash function pointer */
    cmp_fn_t cmp_fn;   /**< Compare function pointer */
    free_fn_t free_fn; /**< User kvp free routine pointer */
} hmap_funcs_t;

/**
 * @brief A structure used to retrieve information of a hmap object
 */
typedef struct hmap {
    TAILQ_ENTRY(hmap) next;        /**< List of next hmap entries */
    char name[HMAP_MAX_NAME_SIZE]; /**< Name of hmap */
    uint32_t capacity;             /**< Total capacity */
    uint32_t max_capacity;         /**< Max capacity should not be exceeded */
    uint32_t curr_capacity;        /**< Current capacity */
    pthread_mutex_t mutex;         /**< Mutex for hmap */
    hmap_funcs_t fns;              /**< Function pointers */
    hmap_kvp_t *map;               /**< Pointer to the key/value table */
    uint64_t alloc_count;          /**< Allocation counter for this hmap */
} hmap_t;

/**
 * @brief Create a hashmap.
 *
 * @param name
 *   Optional name for this hashmap (used for debugging/registry). If NULL, a default name is used.
 * @param capacity
 *   Maximum capacity in entries. If 0, a default is used. The internal table may start smaller and
 *   grow up to this limit.
 * @param funcs
 *   Optional pointer to hmap_funcs_t to override hash/compare/free callbacks. If NULL, defaults
 *   are used.
 * @return
 *   Pointer to a new hmap_t on success, NULL on error.
 */
hmap_t *hmap_create(const char *name, uint32_t capacity, hmap_funcs_t *funcs);

/**
 * @brief Get allocation statistics for a specific hmap
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @return
 *   Number of allocations performed by this hmap
 */
static inline uint64_t
hmap_get_alloc_count(hmap_t *hmap)
{
    return hmap ? hmap->alloc_count : 0;
}

/**
 * @brief Destroy a hashmap.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @return
 *   0 - successful or -1 on error
 */
int hmap_destroy(hmap_t *hmap);

/**
 * @brief Destroy a hashmap by name.
 *
 * @param name
 *   The name of the hmap
 * @return
 *   0 on success or -1 on error.
 */
int hmap_destroy_by_name(const char *name);

/**
 * @brief Lookup an entry by prefix/key.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string (can be NULL for global keys)
 * @param key
 *   The key to search for in the hashmap
 * @return
 *   Pointer to the entry if found, NULL if not found or invalid arguments.
 *
 * @note The returned pointer is owned by the hmap and is only valid while the entry exists.
 */
hmap_kvp_t *hmap_kvp_lookup(hmap_t *hmap, const char *prefix, const char *key);

/**
 * @brief Lookup an entry by prefix/key and optionally copy out its raw value.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string (can be NULL for global keys)
 * @param key
 *   The key to search for in the hashmap
 * @param val
 *   Pointer to hmap_val_t to return value can be NULL for no return value
 * @return
 *   0 if found, -1 if not found or invalid arguments.
 */
int hmap_lookup(hmap_t *hmap, const char *prefix, const char *key, hmap_val_t *val);

/**
 * @brief Add a key/value pair to the hashmap.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param type
 *   The hmap_type_t type for the value pointer
 * @param prefix
 *   The prefix string to locate in the hashmap table (can be NULL)
 * @param key
 *   The key value string to add
 * @param val
 *   The value pointer to store with the key/value entry.
 * @return
 *   0 on success or -1 on error (including duplicate prefix/key).
 */
int hmap_add(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key, hmap_val_t *val);

/**
 * @brief Update a key/value pair if it exists, otherwise add it.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param type
 *   The hmap_type_t type for the value pointer
 * @param prefix
 *   The prefix string to locate in the hashmap table (can be NULL)
 * @param key
 *   The key value string to update
 * @param val
 *   The value pointer to store with the key/value entry.
 * @return
 *   0 on success or -1 on error.
 */
int hmap_update(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key,
                hmap_val_t *val);

/**
 * @brief Update the value for an existing key/value pair returned by hmap_kvp_lookup().
 *
 * @param hmap
 *   Pointer to hmap structure
 * @param kvp
 *   The key/value pair structure to update
 * @param val
 *   The pointer to the new value.
 * @return
 *   0 on success, -1 on error.
 */
int hmap_kvp_update(hmap_t *hmap, hmap_kvp_t *kvp, hmap_val_t *val);

/**
 * @brief Internal type-specific add helpers.
 *
 * These inline functions are used by the public `hmap_add_value()` and `hmap_add_array()` macros.
 * They are not intended to be called directly.
 */

/**
 * @brief Add a string value to the hmap. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string to locate in the hashmap table
 * @param key
 *   The key value string to add
 * @param val
 *   The value to be stored in the hmap table.
 * @return
 *   0 on success or -1 on error.
 */
static inline int
_hmap_add_string(hmap_t *hmap, const char *prefix, const char *key, const char *val)
{
    hmap_val_t v = {.ptr = (void *)(uintptr_t)val};

    return hmap_add(hmap, HMAP_STR_TYPE, prefix, key, &v);
}

/**
 * @brief Internal type-specific update helpers.
 *
 * These inline functions are used by the public `hmap_update_value()` and
 * `hmap_update_array()` macros. They are not intended to be called directly.
 */

/** @brief Update-or-add a string value in the hmap. (internal) */
static inline int
_hmap_update_string(hmap_t *hmap, const char *prefix, const char *key, const char *val)
{
    hmap_val_t v = {.ptr = (void *)(uintptr_t)val};

    return hmap_update(hmap, HMAP_STR_TYPE, prefix, key, &v);
}

/** @brief Update-or-add a uint64_t value in the hmap. (internal) */
static inline int
_hmap_update_u64(hmap_t *hmap, const char *prefix, const char *key, uint64_t val)
{
    hmap_val_t v = {.u64 = val};

    return hmap_update(hmap, HMAP_U64_TYPE, prefix, key, &v);
}

/** @brief Update-or-add a uint32_t value in the hmap. (internal) */
static inline int
_hmap_update_u32(hmap_t *hmap, const char *prefix, const char *key, uint32_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_update(hmap, HMAP_U32_TYPE, prefix, key, &v);
}

/** @brief Update-or-add a uint16_t value in the hmap. (internal) */
static inline int
_hmap_update_u16(hmap_t *hmap, const char *prefix, const char *key, uint16_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_update(hmap, HMAP_U16_TYPE, prefix, key, &v);
}

/** @brief Update-or-add a uint8_t value in the hmap (also used for booleans). (internal) */
static inline int
_hmap_update_u8(hmap_t *hmap, const char *prefix, const char *key, uint8_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_update(hmap, HMAP_U8_TYPE, prefix, key, &v);
}

/** @brief Update-or-add an int64_t value in the hmap. (internal) */
static inline int
_hmap_update_i64(hmap_t *hmap, const char *prefix, const char *key, int64_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_update(hmap, HMAP_I64_TYPE, prefix, key, &v);
}

/** @brief Update-or-add an int32_t value in the hmap. (internal) */
static inline int
_hmap_update_i32(hmap_t *hmap, const char *prefix, const char *key, int32_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)(uint32_t)val};

    return hmap_update(hmap, HMAP_I32_TYPE, prefix, key, &v);
}

/** @brief Update-or-add an int16_t value in the hmap. (internal) */
static inline int
_hmap_update_i16(hmap_t *hmap, const char *prefix, const char *key, int16_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)(uint16_t)val};

    return hmap_update(hmap, HMAP_I16_TYPE, prefix, key, &v);
}

/** @brief Update-or-add an int8_t value in the hmap. (internal) */
static inline int
_hmap_update_i8(hmap_t *hmap, const char *prefix, const char *key, int8_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)(uint8_t)val};

    return hmap_update(hmap, HMAP_I8_TYPE, prefix, key, &v);
}

/** @brief Update-or-add a double value in the hmap. (internal) */
static inline int
_hmap_update_double(hmap_t *hmap, const char *prefix, const char *key, double val)
{
    hmap_val_t v = {.dval = val};

    return hmap_update(hmap, HMAP_DOUBLE_TYPE, prefix, key, &v);
}

/** @brief Update-or-add a pointer value in the hmap. (internal) */
static inline int
_hmap_update_pointer(hmap_t *hmap, const char *prefix, const char *key, void *val)
{
    hmap_val_t v = {.ptr = val};

    return hmap_update(hmap, HMAP_POINTER_TYPE, prefix, key, &v);
}

/** @brief Update-or-add a string array in the hmap. (internal) */
static inline int
_hmap_update_string_array(hmap_t *hmap, const char *prefix, const char *key, char **val,
                          uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_STR_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add a uint64_t array in the hmap. (internal) */
static inline int
_hmap_update_u64_array(hmap_t *hmap, const char *prefix, const char *key, uint64_t *val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_U64_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add a uint32_t array in the hmap. (internal) */
static inline int
_hmap_update_u32_array(hmap_t *hmap, const char *prefix, const char *key, uint32_t *val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_U32_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add a uint16_t array in the hmap. (internal) */
static inline int
_hmap_update_u16_array(hmap_t *hmap, const char *prefix, const char *key, uint16_t *val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_U16_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add a uint8_t array in the hmap. (internal) */
static inline int
_hmap_update_u8_array(hmap_t *hmap, const char *prefix, const char *key, uint8_t *val,
                      uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_U8_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add an int64_t array in the hmap. (internal) */
static inline int
_hmap_update_i64_array(hmap_t *hmap, const char *prefix, const char *key, int64_t *val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_I64_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add an int32_t array in the hmap. (internal) */
static inline int
_hmap_update_i32_array(hmap_t *hmap, const char *prefix, const char *key, int32_t *val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_I32_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add an int16_t array in the hmap. (internal) */
static inline int
_hmap_update_i16_array(hmap_t *hmap, const char *prefix, const char *key, int16_t *val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_I16_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add an int8_t array in the hmap. (internal) */
static inline int
_hmap_update_i8_array(hmap_t *hmap, const char *prefix, const char *key, int8_t *val,
                      uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_I8_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add a double array in the hmap. (internal) */
static inline int
_hmap_update_double_array(hmap_t *hmap, const char *prefix, const char *key, double *val,
                          uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_DOUBLE_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Update-or-add a pointer array in the hmap. (internal) */
static inline int
_hmap_update_pointer_array(hmap_t *hmap, const char *prefix, const char *key, void **val,
                           uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_update(hmap, HMAP_PTR_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add a uint64_t value to the hmap. (internal) */
static inline int
_hmap_add_u64(hmap_t *hmap, const char *prefix, const char *key, uint64_t val)
{
    hmap_val_t v = {.u64 = val};

    return hmap_add(hmap, HMAP_U64_TYPE, prefix, key, &v);
}

/** @brief Add a uint32_t value to the hmap. (internal) */
static inline int
_hmap_add_u32(hmap_t *hmap, const char *prefix, const char *key, uint32_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_add(hmap, HMAP_U32_TYPE, prefix, key, &v);
}

/** @brief Add a uint16_t value to the hmap. (internal) */
static inline int
_hmap_add_u16(hmap_t *hmap, const char *prefix, const char *key, uint16_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_add(hmap, HMAP_U16_TYPE, prefix, key, &v);
}

/** @brief Add a uint8_t value to the hmap (also used for booleans). (internal) */
static inline int
_hmap_add_u8(hmap_t *hmap, const char *prefix, const char *key, uint8_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_add(hmap, HMAP_U8_TYPE, prefix, key, &v);
}

/** @brief Add an int64_t value to the hmap. (internal) */
static inline int
_hmap_add_i64(hmap_t *hmap, const char *prefix, const char *key, int64_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)val};

    return hmap_add(hmap, HMAP_I64_TYPE, prefix, key, &v);
}

/** @brief Add an int32_t value to the hmap. (internal) */
static inline int
_hmap_add_i32(hmap_t *hmap, const char *prefix, const char *key, int32_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)(uint32_t)val};

    return hmap_add(hmap, HMAP_I32_TYPE, prefix, key, &v);
}

/** @brief Add an int16_t value to the hmap. (internal) */
static inline int
_hmap_add_i16(hmap_t *hmap, const char *prefix, const char *key, int16_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)(uint16_t)val};

    return hmap_add(hmap, HMAP_I16_TYPE, prefix, key, &v);
}

/** @brief Add an int8_t value to the hmap. (internal) */
static inline int
_hmap_add_i8(hmap_t *hmap, const char *prefix, const char *key, int8_t val)
{
    hmap_val_t v = {.u64 = (uint64_t)(uint8_t)val};

    return hmap_add(hmap, HMAP_I8_TYPE, prefix, key, &v);
}

/** @brief Add a double value to the hmap. (internal) */
static inline int
_hmap_add_double(hmap_t *hmap, const char *prefix, const char *key, double val)
{
    hmap_val_t v = {.dval = val};

    return hmap_add(hmap, HMAP_DOUBLE_TYPE, prefix, key, &v);
}

/** @brief Add a pointer value to the hmap. (internal) */
static inline int
_hmap_add_pointer(hmap_t *hmap, const char *prefix, const char *key, void *val)
{
    hmap_val_t v = {.ptr = val};

    return hmap_add(hmap, HMAP_POINTER_TYPE, prefix, key, &v);
}

/** @brief Add a string array to the hmap. (internal) */
static inline int
_hmap_add_string_array(hmap_t *hmap, const char *prefix, const char *key, char **val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_STR_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add a uint64_t array to the hmap. (internal) */
static inline int
_hmap_add_u64_array(hmap_t *hmap, const char *prefix, const char *key, uint64_t *val,
                    uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_U64_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add a uint32_t array to the hmap. (internal) */
static inline int
_hmap_add_u32_array(hmap_t *hmap, const char *prefix, const char *key, uint32_t *val,
                    uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_U32_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add a uint16_t array to the hmap. (internal) */
static inline int
_hmap_add_u16_array(hmap_t *hmap, const char *prefix, const char *key, uint16_t *val,
                    uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_U16_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/**
 * @brief Add a uint8_t array to the hmap. (internal)
 *
 * Note: Boolean arrays are stored as uint8_t arrays (0=false, 1=true).
 *       Use this function to store boolean arrays.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string
 * @param val
 *   Pointer to the uint8_t array
 * @param count
 *   Number of elements in the array
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_add_u8_array(hmap_t *hmap, const char *prefix, const char *key, uint8_t *val, uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_U8_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add an int64_t array to the hmap. (internal) */
static inline int
_hmap_add_i64_array(hmap_t *hmap, const char *prefix, const char *key, int64_t *val, uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_I64_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add an int32_t array to the hmap. (internal) */
static inline int
_hmap_add_i32_array(hmap_t *hmap, const char *prefix, const char *key, int32_t *val, uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_I32_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add an int16_t array to the hmap. (internal) */
static inline int
_hmap_add_i16_array(hmap_t *hmap, const char *prefix, const char *key, int16_t *val, uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_I16_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add an int8_t array to the hmap. (internal) */
static inline int
_hmap_add_i8_array(hmap_t *hmap, const char *prefix, const char *key, int8_t *val, uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_I8_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add a double array to the hmap. (internal) */
static inline int
_hmap_add_double_array(hmap_t *hmap, const char *prefix, const char *key, double *val,
                       uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_DOUBLE_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/** @brief Add a pointer array to the hmap. (internal) */
static inline int
_hmap_add_pointer_array(hmap_t *hmap, const char *prefix, const char *key, void **val,
                        uint32_t count)
{
    hmap_val_t v = {.ptr = val};
    hmap_kvp_t *kvp;
    int ret = hmap_add(hmap, HMAP_PTR_ARRAY_TYPE, prefix, key, &v);
    if (ret == 0) {
        kvp = hmap_kvp_lookup(hmap, prefix, key);
        if (kvp)
            kvp->count = count;
    }
    return ret;
}

/**
 * @brief Delete a hashmap entry
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string for the hashmap
 * @param key
 *   The Key to search for in the table
 * @return
 *   -1 on error or 0 on success
 */
int hmap_del(hmap_t *hmap, const char *prefix, const char *key);

/**
 * @brief Iterate over entries in the hashmap.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param _kvp
 *   The address to place the key/value pair structure pointer
 * @param next
 *   The next entry to iterate as an index value. Initialize to 0 before the first call.
 * @return
 *   1 if an entry is returned, 0 when there are no more entries (or on invalid arguments).
 *
 * Example:
 *   uint32_t next = 0;
 *   hmap_kvp_t *kvp;
 *   while (hmap_iterate(hmap, &kvp, &next)) {
 *       // use kvp
 *   }
 */
int hmap_iterate(hmap_t *hmap, hmap_kvp_t **_kvp, uint32_t *next);

/**
 * @brief List (dump) out all of the entries in a hashmap.
 * Renamed from hmap_dump() to hmap_list() to better reflect intent.
 *
 * @param f
 *   The file descriptor to use for output of the text
 * @param hmap
 *   Pointer to the hmap structure to list
 * @param sort
 *   If true sort the output by prefix/key before listing
 */
void hmap_list(FILE *f, hmap_t *hmap, bool sort);

/**
 * @brief Get the current table capacity (number of slots).
 *
 * @param hmap
 *   Pointer to the hmap structure.
 * @return
 *   Current table capacity.
 */
static inline uint32_t
hmap_capacity(hmap_t *hmap)
{
    return hmap->capacity;
}

/**
 * @brief Get the number of populated entries currently stored.
 *
 * @param hmap
 *   Pointer to the hmap structure.
 * @return
 *   Number of active entries.
 */
static inline uint32_t
hmap_count(hmap_t *hmap)
{
    return hmap->curr_capacity;
}

/**
 * @brief Get the set of hmap function pointers.
 *
 * @param hmap
 *   The hmap structure pointer
 * @return
 *   NULL on error or pointer to the hmap function structure.
 */
hmap_funcs_t *hmap_get_funcs(hmap_t *hmap);

/**
 * @brief Set hmap callback functions.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param funcs
 *   Pointer to a set of function pointers. Any NULL members fall back to defaults.
 * @return
 *   0 on success, -1 on error.
 */
int hmap_set_funcs(hmap_t *hmap, hmap_funcs_t *funcs);

/**
 * @brief Get a key/value pair from the hmap if it exists. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure.
 * @param prefix
 *   The prefix string pointer (can be NULL for global keys).
 * @param key
 *   The key string.
 * @param type
 *   Expected type of the entry.
 * @return
 *   Pointer to matching key/value pair or NULL if not found or type mismatch.
 */
static inline hmap_kvp_t *
__get_kvp(hmap_t *hmap, const char *prefix, const char *key, hmap_type_t type)
{
    hmap_kvp_t *kvp;

    if (!hmap)
        HMAP_NULL_RET("get failed - hmap not defined %s(%s)\n", prefix ? prefix : "", key);

    kvp = hmap_kvp_lookup(hmap, prefix, key);
    if (!kvp || kvp->type != type)
        HMAP_NULL_RET("get failed for %s(%s)\n", prefix ? prefix : "", key);

    return kvp;
}

/**
 * @brief Get a uint64_t value from the hmap. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the uint64_t value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_u64(hmap_t *hmap, const char *prefix, const char *key, uint64_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U64_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = kvp->v.u64;

    return 0;
}

/**
 * @brief Get a uint32_t value from the hmap. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the uint32_t value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_u32(hmap_t *hmap, const char *prefix, const char *key, uint32_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U32_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (uint32_t)kvp->v.u64;

    return 0;
}

/**
 * @brief Get a uint16_t value from the hmap. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the uint16_t value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_u16(hmap_t *hmap, const char *prefix, const char *key, uint16_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U16_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (uint16_t)kvp->v.u64;

    return 0;
}

/**
 * @brief Get a uint8_t value from the hmap. (internal)
 *
 * Note: Boolean values are stored as uint8_t (0=false, 1=true).
 *       Use this function to retrieve boolean values.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the uint8_t value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_u8(hmap_t *hmap, const char *prefix, const char *key, uint8_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U8_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (uint8_t)kvp->v.u64;

    return 0;
}

/**
 * @brief Get a number value from the hmap as a int64_t. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the number value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_i64(hmap_t *hmap, const char *prefix, const char *key, int64_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I64_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int64_t)kvp->v.u64;

    return 0;
}

/**
 * @brief Get a number value from the hmap as a int32_t. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the number value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_i32(hmap_t *hmap, const char *prefix, const char *key, int32_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I32_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int32_t)kvp->v.u64;

    return 0;
}

/**
 * @brief Get a number value from the hmap as a int16_t. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the number value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_i16(hmap_t *hmap, const char *prefix, const char *key, int16_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I16_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int16_t)kvp->v.u64;

    return 0;
}

/**
 * @brief Get a number value from the hmap as a int8_t. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the number value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
_hmap_get_i8(hmap_t *hmap, const char *prefix, const char *key, int8_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I8_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int8_t)kvp->v.u64;

    return 0;
}

/** @brief Get a double value from the hmap. (internal) */
static inline int
_hmap_get_double(hmap_t *hmap, const char *prefix, const char *key, double *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_DOUBLE_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = kvp->v.dval;

    return 0;
}

/**
 * @brief Get a string value from the hmap. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the string value.
 * @param val
 *   A pointer to the return value or NULL if no return value is needed.
 * @return
 *   0 on success, -1 on error.
 */
static inline int
_hmap_get_string(hmap_t *hmap, const char *prefix, const char *key, char **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_STR_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (char *)kvp->v.ptr;

    return 0;
}

/**
 * @brief Get a pointer value from the hmap. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key string.
 * @param val
 *   A pointer to the return value or NULL if no return value is needed.
 * @return
 *   0 - successful or -1 on error
 */
static inline int
_hmap_get_pointer(hmap_t *hmap, const char *prefix, const char *key, void **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_POINTER_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = kvp->v.ptr;

    return 0;
}

/**
 * @brief Get a string array from the hmap. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure.
 * @param prefix
 *   The prefix string pointer (can be NULL for global keys).
 * @param key
 *   The key string.
 * @param val
 *   Pointer to store the array pointer.
 * @return
 *   Number of elements on success, -1 on failure.
 */
static inline int
_hmap_get_string_array(hmap_t *hmap, const char *prefix, const char *key, char ***val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_STR_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (char **)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get a uint64_t array from the hmap. (internal) */
static inline int
_hmap_get_u64_array(hmap_t *hmap, const char *prefix, const char *key, uint64_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U64_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (uint64_t *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get a uint32_t array from the hmap. (internal) */
static inline int
_hmap_get_u32_array(hmap_t *hmap, const char *prefix, const char *key, uint32_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U32_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (uint32_t *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get a uint16_t array from the hmap. (internal) */
static inline int
_hmap_get_u16_array(hmap_t *hmap, const char *prefix, const char *key, uint16_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U16_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (uint16_t *)kvp->v.ptr;

    return kvp->count;
}
/**
 * @brief Get a uint8_t array from the hmap. (internal)
 *
 * Note: Boolean arrays are stored as uint8_t arrays (0=false, 1=true).
 *       Use this function to retrieve boolean arrays.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string
 * @param val
 *   Pointer to store the array pointer
 * @return
 *   Array count on success, -1 on failure
 */
static inline int
_hmap_get_u8_array(hmap_t *hmap, const char *prefix, const char *key, uint8_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U8_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (uint8_t *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get an int64_t array from the hmap. (internal) */
static inline int
_hmap_get_i64_array(hmap_t *hmap, const char *prefix, const char *key, int64_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I64_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int64_t *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get an int32_t array from the hmap. (internal) */
static inline int
_hmap_get_i32_array(hmap_t *hmap, const char *prefix, const char *key, int32_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I32_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int32_t *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get an int16_t array from the hmap. (internal) */
static inline int
_hmap_get_i16_array(hmap_t *hmap, const char *prefix, const char *key, int16_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I16_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int16_t *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get an int8_t array from the hmap. (internal) */
static inline int
_hmap_get_i8_array(hmap_t *hmap, const char *prefix, const char *key, int8_t **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_I8_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (int8_t *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get a double array from the hmap. (internal) */
static inline int
_hmap_get_double_array(hmap_t *hmap, const char *prefix, const char *key, double **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_DOUBLE_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (double *)kvp->v.ptr;

    return kvp->count;
}

/** @brief Get a pointer array from the hmap. (internal) */
static inline int
_hmap_get_pointer_array(hmap_t *hmap, const char *prefix, const char *key, void ***val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_PTR_ARRAY_TYPE);

    if (!kvp)
        return -1;

    if (val)
        *val = (void **)kvp->v.ptr;

    return kvp->count;
}

/**
 * @brief List all of the hashmaps
 *
 * @param f
 *   The file descriptor to dump the text data.
 */
void hmap_list_names(FILE *f);

/**
 * @brief Dump a hashmap by name
 *
 * @param f
 *   The file descriptor to dump the text data.
 * @param name
 *  The name of the hashmap to dump
 * @param sort
 *   if true then sort the output
 */
void hmap_list_by_name(FILE *f, char *name, bool sort);

/**
 * @brief Dump all of the hmap lists
 *
 * @param f
 *   The file descriptor to dump the text data.
 * @param sort
 *   if true then sort the output
 */
void hmap_list_all(FILE *f, bool sort);

/**
 * @brief Type-safe generic getter macro for scalar values.
 *
 * This macro uses C11 _Generic to automatically dispatch to the correct
 * internal _hmap_get_*() function based on the type of the value pointer.
 *
 * @param hmap   hmap handle
 * @param prefix Prefix string (can be NULL)
 * @param key    Key string
 * @param value  Pointer to store the value (type determines which function is called)
 * @return       0 on success, -1 on failure
 *
 * Example:
 *   uint32_t count;
 *   hmap_get_value(hmap, "config", "count", &count);  // Calls _hmap_get_u32
 *
 *   uint8_t enabled;  // boolean stored as uint8_t (0=false, 1=true)
 *   hmap_get_value(hmap, NULL, "enabled", &enabled);  // Calls _hmap_get_u8
 */
#define hmap_get_value(hmap, prefix, key, value) \
    _Generic((value),                            \
        char **: _hmap_get_string,               \
        uint64_t *: _hmap_get_u64,               \
        uint32_t *: _hmap_get_u32,               \
        uint16_t *: _hmap_get_u16,               \
        uint8_t *: _hmap_get_u8,                 \
        int64_t *: _hmap_get_i64,                \
        int32_t *: _hmap_get_i32,                \
        int16_t *: _hmap_get_i16,                \
        int8_t *: _hmap_get_i8,                  \
        double *: _hmap_get_double,              \
        void **: _hmap_get_pointer)((hmap), (prefix), (key), (value))

/**
 * @brief Type-safe generic getter macro for array values.
 *
 * This macro uses C11 _Generic to automatically dispatch to the correct
 * internal _hmap_get_*_array() function based on the type of the value pointer.
 * Returns the array count and populates the pointer.
 *
 * @param hmap   hmap handle
 * @param prefix Prefix string (can be NULL)
 * @param key    Key string
 * @param value  Pointer to store array pointer (type determines which function is called)
 * @return       Array count on success, -1 on failure
 *
 * Example:
 *   uint32_t *ports;
 *   int count = hmap_get_array(hmap, "config", "ports", &ports);  // Calls _hmap_get_u32_array
 */
#define hmap_get_array(hmap, prefix, key, value) \
    _Generic((value),                            \
        char ***: _hmap_get_string_array,        \
        uint64_t **: _hmap_get_u64_array,        \
        uint32_t **: _hmap_get_u32_array,        \
        uint16_t **: _hmap_get_u16_array,        \
        uint8_t **: _hmap_get_u8_array,          \
        int64_t **: _hmap_get_i64_array,         \
        int32_t **: _hmap_get_i32_array,         \
        int16_t **: _hmap_get_i16_array,         \
        int8_t **: _hmap_get_i8_array,           \
        double **: _hmap_get_double_array,       \
        void ***: _hmap_get_pointer_array)((hmap), (prefix), (key), (value))

/**
 * @brief Type-safe generic setter macro for scalar values.
 *
 * This macro uses C11 _Generic to automatically dispatch to the correct
 * internal _hmap_add_*() function based on the type of the value.
 *
 * @param hmap   hmap handle
 * @param prefix Prefix string (can be NULL)
 * @param key    Key string
 * @param value  Value to store (type determines which function is called)
 * @return       0 on success, -1 on failure
 *
 * Example:
 *   hmap_add_value(hmap, "config", "port", 8080);           // Calls _hmap_add_u32
 *   hmap_add_value(hmap, "app", "name", "MyApp");           // Calls _hmap_add_string
 *   hmap_add_value(hmap, "flags", "enabled", 1);            // Calls _hmap_add_i32
 *   hmap_add_value(hmap, "stats", "ratio", 3.14);           // Calls _hmap_add_double
 */
#define hmap_add_value(hmap, prefix, key, value) \
    _Generic((value),                            \
        char *: _hmap_add_string,                \
        const char *: _hmap_add_string,          \
        uint64_t: _hmap_add_u64,                 \
        uint32_t: _hmap_add_u32,                 \
        uint16_t: _hmap_add_u16,                 \
        uint8_t: _hmap_add_u8,                   \
        int64_t: _hmap_add_i64,                  \
        int32_t: _hmap_add_i32,                  \
        int16_t: _hmap_add_i16,                  \
        int8_t: _hmap_add_i8,                    \
        double: _hmap_add_double,                \
        float: _hmap_add_double,                 \
        void *: _hmap_add_pointer,               \
        default: _hmap_add_i32)((hmap), (prefix), (key), (value))

/**
 * @brief Type-safe generic update macro for scalar values.
 *
 * This macro uses C11 _Generic to automatically dispatch to the correct
 * internal _hmap_update_*() function based on the type of the value.
 *
 * Semantics match hmap_update(): update if key exists, otherwise add.
 *
 * @param hmap   hmap handle
 * @param prefix Prefix string (can be NULL)
 * @param key    Key string
 * @param value  Value to store (type determines which function is called)
 * @return       0 on success, -1 on failure
 */
#define hmap_update_value(hmap, prefix, key, value) \
    _Generic((value),                               \
        char *: _hmap_update_string,                \
        const char *: _hmap_update_string,          \
        uint64_t: _hmap_update_u64,                 \
        uint32_t: _hmap_update_u32,                 \
        uint16_t: _hmap_update_u16,                 \
        uint8_t: _hmap_update_u8,                   \
        int64_t: _hmap_update_i64,                  \
        int32_t: _hmap_update_i32,                  \
        int16_t: _hmap_update_i16,                  \
        int8_t: _hmap_update_i8,                    \
        double: _hmap_update_double,                \
        float: _hmap_update_double,                 \
        void *: _hmap_update_pointer,               \
        default: _hmap_update_i32)((hmap), (prefix), (key), (value))

/**
 * @brief Type-safe generic setter macro for array values.
 *
 * This macro uses C11 _Generic to automatically dispatch to the correct
 * internal _hmap_add_*_array() function based on the type of the array pointer.
 *
 * @param hmap   hmap handle
 * @param prefix Prefix string (can be NULL)
 * @param key    Key string
 * @param array  Pointer to array data (type determines which function is called)
 * @param count  Number of elements in the array
 * @return       0 on success, -1 on failure
 *
 * Example:
 *   uint32_t ports[] = {8080, 8081, 8082};
 *   hmap_add_array(hmap, "config", "ports", ports, 3);  // Calls _hmap_add_u32_array
 *
 *   char *names[] = {"alice", "bob"};
 *   hmap_add_array(hmap, "users", "names", names, 2);   // Calls _hmap_add_string_array
 */
#define hmap_add_array(hmap, prefix, key, array, count) \
    _Generic((array),                                   \
        char **: _hmap_add_string_array,                \
        uint64_t *: _hmap_add_u64_array,                \
        uint32_t *: _hmap_add_u32_array,                \
        uint16_t *: _hmap_add_u16_array,                \
        uint8_t *: _hmap_add_u8_array,                  \
        int64_t *: _hmap_add_i64_array,                 \
        int32_t *: _hmap_add_i32_array,                 \
        int16_t *: _hmap_add_i16_array,                 \
        int8_t *: _hmap_add_i8_array,                   \
        double *: _hmap_add_double_array,               \
        void **: _hmap_add_pointer_array)((hmap), (prefix), (key), (array), (count))

/**
 * @brief Type-safe generic update macro for array values.
 *
 * This macro uses C11 _Generic to automatically dispatch to the correct
 * internal _hmap_update_*_array() function based on the type of the array pointer.
 *
 * Semantics match hmap_update(): update if key exists, otherwise add.
 *
 * @param hmap   hmap handle
 * @param prefix Prefix string (can be NULL)
 * @param key    Key string
 * @param array  Pointer to array data (type determines which function is called)
 * @param count  Number of elements in the array
 * @return       0 on success, -1 on failure
 */
#define hmap_update_array(hmap, prefix, key, array, count) \
    _Generic((array),                                      \
        char **: _hmap_update_string_array,                \
        uint64_t *: _hmap_update_u64_array,                \
        uint32_t *: _hmap_update_u32_array,                \
        uint16_t *: _hmap_update_u16_array,                \
        uint8_t *: _hmap_update_u8_array,                  \
        int64_t *: _hmap_update_i64_array,                 \
        int32_t *: _hmap_update_i32_array,                 \
        int16_t *: _hmap_update_i16_array,                 \
        int8_t *: _hmap_update_i8_array,                   \
        double *: _hmap_update_double_array,               \
        void **: _hmap_update_pointer_array)((hmap), (prefix), (key), (array), (count))

/**
 * @brief Get global allocation statistics
 *
 * Returns zeros if HMAP_TRACK_ALLOCS is not defined at compile time
 *
 * @param alloc_count
 *   Pointer to store total allocation count (can be NULL)
 * @param realloc_count
 *   Pointer to store reallocation count (can be NULL)
 * @param total_bytes
 *   Pointer to store total bytes allocated (can be NULL)
 */
void hmap_get_global_alloc_stats(uint64_t *alloc_count, uint64_t *realloc_count,
                                 uint64_t *total_bytes);

/**
 * @brief Reset global allocation statistics
 *
 * No effect if HMAP_TRACK_ALLOCS is not defined at compile time
 */
void hmap_reset_global_alloc_stats(void);

#ifdef __cplusplus
}
#endif
