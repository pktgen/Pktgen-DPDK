/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2021-2024 Intel Corporation
 */

/**
 * @file
 *
 * This library provides an API for the hashmap data structure
 */

// IWYU pragma: no_include <bits/stdint-uintn.h>

#ifndef __HMAP_H_
#define __HMAP_H_

#include <stdbool.h>          // for bool
#include <string.h>           // for NULL
#include <strings.h>          // for strcasecmp
#include <stdint.h>           // for uint32_t, int64_t, uint16_t, uint64_t, uint8_t
#include <stdio.h>            // for FILE
#include <sys/queue.h>        // for TAILQ_ENTRY
#include <pthread.h>          // for pthread_mutex_t

#ifdef __cplusplus
extern "C" {
#endif

#define HMAP_API __attribute__((visibility("default")))

#define HMAP_MAX_NAME_SIZE     32   /**< MAX size of HMAP name */
#define HMAP_MAX_KEY_SIZE      256  /**< Key size */
#define HMAP_STARTING_CAPACITY 32   /**< Starting capacity not exceeding max_capacity */
#define HMAP_DEFAULT_CAPACITY  1024 /**< Default starting capacity not exceeding max_capacity */

struct hmap;

// clang-format off
typedef enum {
    HMAP_EMPTY_TYPE,
    HMAP_STR_TYPE,
    HMAP_U64_TYPE,
    HMAP_U32_TYPE,
    HMAP_U16_TYPE,
    HMAP_U8_TYPE,
    HMAP_NUM_TYPE,
    HMAP_NUM64_TYPE,
    HMAP_BOOLEAN_TYPE,
    HMAP_POINTER_TYPE,
    HMAP_NUM_TYPES
} hmap_type_t;
// clang-format on

typedef union {
    char *str;     /**< String pointer value */
    uint64_t u64;  /**< uint64_t value */
    uint32_t u32;  /**< uint32_t value */
    uint16_t u16;  /**< uint16_t value */
    uint8_t u8;    /**< uint8_t value */
    int num;       /**< Number type as an int value */
    int64_t num64; /**< Number type as an int64 value */
    bool boolean;  /**< Boolean value */
    void *ptr;     /**< Pointer to a structure or anything */
} hmap_val_t;

/**
 * A structure used to retrieve information of a key-value-pair hmap
 */
typedef struct hmap_kvp { /**< Key-value-pair 24 bytes total 8 byte data */
    hmap_type_t type;     /**< Type of the value stored in kvp */
    char *prefix;         /**< Prefix string value */
    char *key;            /**< String key pointer */
    hmap_val_t v;         /**< Values stored in kvp */
} hmap_kvp_t;

typedef uint32_t (*hash_fn_t)(const char *prefix, const char *key);
typedef int (*cmp_fn_t)(const char *prefix, const char *key, const hmap_kvp_t *kvp);
typedef void (*free_fn_t)(hmap_kvp_t *kvp);

typedef struct hmap_funcs {
    hash_fn_t hash_fn; /**< Hash function pointer */
    cmp_fn_t cmp_fn;   /**< Compare function pointer */
    free_fn_t free_fn; /**< User kvp free routine pointer */
} hmap_funcs_t;

/**
 * A structure used to retrieve information of a hmap
 */
typedef struct hmap {
    TAILQ_ENTRY(hmap) next;        /**< List of next hmap entries */
    char name[HMAP_MAX_NAME_SIZE]; /**< Name of hmap */
    uint32_t capacity;             /**< Total capacity */
    uint32_t max_capacity;         /**< Max capacity should not be exceeded */
    uint32_t curr_capacity;        /**< Current capacity */
    pthread_mutex_t mutex;         /**< Mutex for hmap */
    hmap_funcs_t fns;              /**< Function pointers */
    hmap_kvp_t *map;               /**< Key-value-pair 16 bytes total 8 byte data */
} hmap_t;

#define HMAP_ERR(fmt, ...)   printf("ERR: " fmt, ##__VA_ARGS__)
#define HMAP_WARN(fmt, ...)  printf("WARN: " fmt, ##__VA_ARGS__)
#define HMAP_DEBUG(fmt, ...) printf("DEBUG: " fmt, ##__VA_ARGS__)

/**
 * Generate an Error log message and return value
 *
 * Returning _val  to the calling function.
 */
#define HMAP_ERR_RET_VAL(_val, ...) \
    do {                            \
        HMAP_ERR(__VA_ARGS__);      \
        return _val;                \
    } while ((0))

/**
 * Generate an Error log message and return
 *
 * Returning to the calling function.
 */
#define HMAP_RET(...) HMAP_ERR_RET_VAL(, __VA_ARGS__)

/**
 * Generate an Error log message and return -1
 *
 * Returning a -1 to the calling function.
 */
#define HMAP_ERR_RET(...) HMAP_ERR_RET_VAL(-1, __VA_ARGS__)

/**
 * Generate an Error log message and return NULL
 *
 * Returning a NULL to the calling function.
 */
#define HMAP_NULL_RET(...) HMAP_ERR_RET_VAL(NULL, __VA_ARGS__)

/**
 * Generate a Error log message and goto label
 *
 */
#define HMAP_ERR_GOTO(lbl, ...) \
    do {                        \
        HMAP_ERR(__VA_ARGS__);  \
        goto lbl;               \
    } while ((0))

/**
 * Create a hashmap structure with a fixed capacity hash size
 *
 * @param name
 *   A string name for this hashmap
 * @param capacity
 *   The size of the hash table
 * @param funcs
 *   Optional pointer to hmap_funcs_t structure to allow user to set hmap functions.
 * @return
 *   NULL if error or the hmap_t pointer
 */
HMAP_API hmap_t *hmap_create(const char *name, uint32_t capacity, hmap_funcs_t *funcs);

/**
 * Destroy a hashmap table
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @return
 *   0 - successful or -1 on error
 */
HMAP_API int hmap_destroy(hmap_t *hmap);

/**
 * Destroy the hmap using its name
 *
 * @param name
 *   The name of the hmap
 * @return
 *   0 on success or -1 on error.
 */
HMAP_API int hmap_destroy_by_name(const char *name);

/**
 * Lookup a prefix/key value in the hashmap
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string
 * @param key
 *   The key to search for in the hashmap
 * @return
 *   NULL on error or hmap_kvp_t pointer
 */
HMAP_API hmap_kvp_t *hmap_kvp_lookup(hmap_t *hmap, const char *prefix, const char *key);

/**
 * Lookup a prefix/key value in the hashmap
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string
 * @param key
 *   The key to search for in the hashmap
 * @param val
 *   Pointer to hmap_val_t to return value can be NULL for no return value
 * @return
 *  0 - successful or -1 on error
 */
HMAP_API int hmap_lookup(hmap_t *hmap, const char *prefix, const char *key, hmap_val_t *val);

/**
 * Add a key/value pair the hashmap table
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param type
 *   The hmap_type_t type for the value pointer
 * @param prefix
 *   The prefix string to locate in the hashmap table
 * @param key
 *   The key value string to add
 * @param val
 *   The value pointer to store with the key/value entry.
 * @return
 *   0 on success or -1 on error.
 */
HMAP_API int hmap_add(hmap_t *hmap, hmap_type_t type, const char *prefix, const char *key,
                      hmap_val_t *val);

/**
 * Update the value for the given key/value pair return by hmap_kvp_lookup().
 *
 * @param hmap
 *   Pointer to hmap structure
 * @param kvp
 *   The key/value pair structure to update
 * @param val
 *   The pointer to the new value.
 * @returns
 *   0 - successful update and -1 on error.
 */
HMAP_API int hmap_kvp_update(hmap_t *hmap, hmap_kvp_t *kvp, hmap_val_t *val);

/**
 * Add a key/value pair the hashmap table for all of the types allowed.
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
hmap_add_string(hmap_t *hmap, const char *prefix, const char *key, const char *val)
{
    hmap_val_t v = {.str = (char *)(uintptr_t)val};

    return hmap_add(hmap, HMAP_STR_TYPE, prefix, key, &v);
}

static inline int
hmap_add_u64(hmap_t *hmap, const char *prefix, const char *key, uint64_t val)
{
    hmap_val_t v = {.u64 = val};

    return hmap_add(hmap, HMAP_U64_TYPE, prefix, key, &v);
}

static inline int
hmap_add_u32(hmap_t *hmap, const char *prefix, const char *key, uint32_t val)
{
    hmap_val_t v = {.u32 = val};

    return hmap_add(hmap, HMAP_U32_TYPE, prefix, key, &v);
}

static inline int
hmap_add_u16(hmap_t *hmap, const char *prefix, const char *key, uint16_t val)
{
    hmap_val_t v = {.u16 = val};

    return hmap_add(hmap, HMAP_U16_TYPE, prefix, key, &v);
}

static inline int
hmap_add_u8(hmap_t *hmap, const char *prefix, const char *key, uint8_t val)
{
    hmap_val_t v = {.u8 = val};

    return hmap_add(hmap, HMAP_U8_TYPE, prefix, key, &v);
}

static inline int
hmap_add_num(hmap_t *hmap, const char *prefix, const char *key, int val)
{
    hmap_val_t v = {.num = val};

    return hmap_add(hmap, HMAP_NUM_TYPE, prefix, key, &v);
}

static inline int
hmap_add_num64(hmap_t *hmap, const char *prefix, const char *key, int64_t val)
{
    hmap_val_t v = {.num64 = val};

    return hmap_add(hmap, HMAP_NUM64_TYPE, prefix, key, &v);
}

static inline int
hmap_add_bool(hmap_t *hmap, const char *prefix, const char *key, bool val)
{
    hmap_val_t v = {.boolean = val};

    return hmap_add(hmap, HMAP_BOOLEAN_TYPE, prefix, key, &v);
}

static inline int
hmap_add_pointer(hmap_t *hmap, const char *prefix, const char *key, void *val)
{
    hmap_val_t v = {.ptr = val};

    return hmap_add(hmap, HMAP_POINTER_TYPE, prefix, key, &v);
}

static inline int
hmap_inc_u16(hmap_t *hmap, const char *prefix, const char *key, int val)
{
    hmap_val_t v = {.u16 = val};
    hmap_kvp_t *kvp;

    if ((kvp = hmap_kvp_lookup(hmap, prefix, key)) != NULL) {
        v.u16 = (kvp->v.u16 + val);
        hmap_kvp_update(hmap, kvp, &v);
    }

    return hmap_add(hmap, HMAP_U16_TYPE, prefix, key, &v);
}

/**
 * Delete a hashmap entry
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
HMAP_API int hmap_del(hmap_t *hmap, const char *prefix, const char *key);

/**
 * Iterate over the all of the entries in the hashmap
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param _kvp
 *   The address to place the key/value pair structure pointer
 * @param next
 *   The next entry to iterate as an index value.
 * @return
 *   0 on success or -1 on error
 */
HMAP_API int hmap_iterate(hmap_t *hmap, hmap_kvp_t **_kvp, uint32_t *next);

/**
 * Dump out all of the entries in a hashmap
 *
 * @param f
 *   The file descriptor to use for output of the text
 * @param hmap
 *   Pointer to the hmap structure to dump
 * @param sort
 *   The sort flag is non-zero then sort the output
 */
HMAP_API void hmap_dump(FILE *f, hmap_t *hmap, int sort);

/**
 * Get hmap MAX capacity
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @return
 *   The MAX capacity value.
 */
static inline uint32_t
hmap_capacity(hmap_t *hmap)
{
    return hmap->capacity;
}

/**
 * Return the current hmap capacity
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @return
 *   The current capacity of the hmap
 */
static inline uint32_t
hmap_count(hmap_t *hmap)
{
    return hmap->curr_capacity;
}

/**
 * Get the set of hmap function pointers.
 *
 * @param hmap
 *   The hmap structure pointer
 * @return
 *   NULL on error or pointer to the hmap function structure.
 */
HMAP_API hmap_funcs_t *hmap_get_funcs(hmap_t *hmap);

/**
 * Set the hashing function
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param funcs
 *   Pointer to a set of function pointers
 * @return
 *   -1 on if hmap is NULL or 0 on success
 */
HMAP_API int hmap_set_funcs(hmap_t *hmap, hmap_funcs_t *funcs);

/**
 * Get a key/value pair from the hmap if is exists. (internal)
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the boolean value.
 * @param type
 *   The key value type.
 * @return
 *   The key/value pair structure pointer or NULL if not found.
 */
static inline hmap_kvp_t *
__get_kvp(hmap_t *hmap, const char *prefix, const char *key, hmap_type_t type)
{
    hmap_kvp_t *kvp;

    if (!hmap)
        HMAP_NULL_RET("get failed - hmap not defined %s(%s)\n", prefix ? prefix : "", key);

    kvp = hmap_kvp_lookup(hmap, prefix, key);
    if (!kvp)
        return NULL;
    if (kvp->type != type)
        HMAP_NULL_RET("get failed - type mismatch %s(%s)\n", prefix ? prefix : "", key);

    return kvp;
}

/**
 * Get a boolean value from the hmap.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the boolean value.
 * @param val
 *   The location to store the value.
 * @return
 *   0 - successful or -1 - failed
 */
static inline int
hmap_get_bool(hmap_t *hmap, const char *prefix, const char *key, bool *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_BOOLEAN_TYPE);

    if (!kvp || kvp->type != HMAP_BOOLEAN_TYPE)
        return -1;

    if (val)
        *val = kvp->v.boolean;

    return 0;
}

/**
 * Get a uint64_t value from the hmap.
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
hmap_get_u64(hmap_t *hmap, const char *prefix, const char *key, uint64_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U64_TYPE);

    if (!kvp || kvp->type != HMAP_U64_TYPE)
        return -1;

    if (val)
        *val = kvp->v.u64;

    return 0;
}

/**
 * Get a uint32_t value from the hmap.
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
hmap_get_u32(hmap_t *hmap, const char *prefix, const char *key, uint32_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U32_TYPE);

    if (!kvp || kvp->type != HMAP_U32_TYPE)
        return -1;

    if (val)
        *val = kvp->v.u32;

    return 0;
}

/**
 * Get a uint16_t value from the hmap.
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
hmap_get_u16(hmap_t *hmap, const char *prefix, const char *key, uint16_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U16_TYPE);

    if (!kvp || kvp->type != HMAP_U16_TYPE)
        return -1;

    if (val)
        *val = kvp->v.u16;

    return 0;
}

/**
 * Get a uint8_t value from the hmap.
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
hmap_get_u8(hmap_t *hmap, const char *prefix, const char *key, uint8_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_U8_TYPE);

    if (!kvp || kvp->type != HMAP_U8_TYPE)
        return -1;

    if (val)
        *val = kvp->v.u8;

    return 0;
}

/**
 * Get a number value from the hmap.
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
hmap_get_num(hmap_t *hmap, const char *prefix, const char *key, int *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_NUM_TYPE);

    if (!kvp || kvp->type != HMAP_NUM_TYPE)
        return -1;

    if (val)
        *val = kvp->v.num;

    return 0;
}

/**
 * Get a number value from the hmap as a int64_t.
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
hmap_get_num64(hmap_t *hmap, const char *prefix, const char *key, int64_t *val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_NUM64_TYPE);

    if (!kvp || kvp->type != HMAP_NUM64_TYPE)
        return -1;

    if (val)
        *val = kvp->v.num64;

    return 0;
}

/**
 * Get a string value from the hmap.
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
 *   The string value if the key is found or NULL on error.
 */
static inline int
hmap_get_string(hmap_t *hmap, const char *prefix, const char *key, char **val)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_STR_TYPE);

    if (!kvp || kvp->type != HMAP_STR_TYPE)
        return -1;

    if (val)
        *val = kvp->v.str;

    return 0;
}

/**
 * Get a pointer value from the hmap.
 *
 * @param hmap
 *   Pointer to the hmap structure
 * @param prefix
 *   The prefix string pointer, can be NULL if a global value
 * @param key
 *   The key value string to get the string value.
 * @return
 *   0 - successful or -1 on error
 */
static inline void *
hmap_get_pointer(hmap_t *hmap, const char *prefix, const char *key)
{
    hmap_kvp_t *kvp = __get_kvp(hmap, prefix, key, HMAP_POINTER_TYPE);

    if (!kvp || kvp->type != HMAP_POINTER_TYPE)
        return NULL;

    return kvp->v.ptr;
}

/**
 * Dump all of the hmap values to the console.
 *
 * @param f
 *   The file to write to.
 * @param sort
 *   0 - no sorting, 1 - alphabetical
 */
void hmap_dump_all(FILE *f, int sort);

/**
 * Dump all of the hmap lists
 *
 * @param f
 *   The file descriptor to dump the text data.
 * @param sort
 *   if not zero then sort the output
 */
void hmap_dump_map(FILE *f, const char *name, int sort);

/**
 * Dump all of the hmap names
 *
 * @param f
 *   The file descriptor to dump the text data.
 */
void hmap_list_maps(FILE *f);

/**
 * Dump out a specific hmap
 *
 * @param f
 *   The file descriptor to dump the text data.
 * @param name
 *   The name of the hmap to dump.
 * @param sort
 *   if not zero then sort the output
 */
void hmap_dump_map(FILE *f, const char *name, int sort);

#ifdef __cplusplus
}
#endif

#endif /* __HMAP_H_*/
