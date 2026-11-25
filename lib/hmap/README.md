# HMAP - Hash Map Library

## Overview

HMAP is a high-performance, type-safe hash map implementation optimized for DPDK applications. It provides a flexible key-value store with support for multiple data types, arrays, and efficient memory management.

## Key Features

- **Type-safe storage**: Native support for 12+ scalar types and their arrays
- **Prefix-based organization**: Group related keys using optional prefixes
- **Zero-copy arrays**: Direct pointer access to array data
- **Cache-aligned structures**: Optimized for modern CPU architectures
- **Thread-safe options**: Built-in mutex support for concurrent access
- **Unified 64-bit storage**: Efficient memory layout with minimal overhead
- **Auto-managed keys**: Keys and prefixes are automatically duplicated and freed

## Important: String Value Lifetime Requirements

**The caller is responsible for managing the lifetime of string VALUES, but NOT keys/prefixes.**

### Memory Ownership Model

HMAP uses a mixed memory ownership model:

- **Keys and Prefixes**: HMAP-managed - automatically duplicated and freed by hmap
- **String Values**: Caller-managed - must remain valid for entry lifetime
- **String Arrays**: Caller-managed - neither the array nor its elements are copied
- **Scalar Values**: Copied by value into hmap's internal storage
- **Scalar Arrays**: Caller-managed - pointer is stored, data must remain valid

### Benefits

This design provides:
- **Flexible keys**: Use dynamic strings, constants, or temporaries for keys/prefixes
- **Efficient values**: No allocation overhead for scalar values
- **Predictable performance**: No hidden allocations except for keys/prefixes
- **Simple cleanup**: Free your value allocations after hmap_destroy()

### String Value Examples

```c
// ✓ CORRECT: String literals (most common and recommended)
hmap_add_value(hmap, "config", "app_name", "MyApp");  // String literal - valid forever

// ✓ CORRECT: Keys can be dynamic (automatically duplicated by hmap)
char key_buf[32];
snprintf(key_buf, sizeof(key_buf), "port_%d", port_num);
hmap_add_value(hmap, "network", key_buf, port_config);  // key_buf duplicated internally

// ✓ CORRECT: Caller-managed dynamic string value
char *description = strdup("My application description");
hmap_add_value(hmap, "config", "description", description);
// ... use hmap ...
hmap_destroy(hmap);
free(description);  // Free AFTER hmap is destroyed

// ✓ CORRECT: Static/global strings
static char global_string[] = "Global Value";
hmap_add_value(hmap, "config", "global", global_string);

// ✗ WRONG: Stack variable (will be invalid after function returns)
void bad_example() {
    char local_buffer[256];
    snprintf(local_buffer, sizeof(local_buffer), "Value %d", 42);
    hmap_add_value(hmap, "test", "value", local_buffer);  // DANGLING POINTER!
}

// ✗ WRONG: Freeing string while still in hmap
char *temp = strdup("temp");
hmap_add_value(hmap, "test", "temp", temp);
free(temp);  // WRONG - hmap still has pointer to freed memory!
char *result;
hmap_get_value(hmap, "test", "temp", &result);  // result now points to freed memory!
```

### String Array Examples

```c
// ✓ CORRECT: Caller-managed string array
char **allowed_ips = malloc(3 * sizeof(char *));
int allocated = 1;
if (allocated) {
	allowed_ips[0] = strdup("192.168.1.1");
	allowed_ips[1] = strdup("10.0.0.1");
	allowed_ips[2] = strdup("172.16.0.1");
} else { // Or use string literals
	allowed_ips[0] = "192.168.1.1";
	allowed_ips[1] = "10.0.0.1";
	allowed_ips[2] = "172.16.0.1";
}

hmap_add_array(hmap, "security", "allowed_ips", allowed_ips, 3);
// ... use hmap ...
hmap_destroy(hmap);

// Free AFTER hmap is destroyed only when entries are allocated by caller
if (allocated) {
	for (int i = 0; i < 3; i++)
		free(allowed_ips[i]);
}
free(allowed_ips);
```

### Key and Prefix Examples

```c
// ✓ CORRECT: String literals (most common and recommended)
hmap_add_value(hmap, "config", "port", 8080);

// ✓ CORRECT: Static/global strings
static const char *prefix = "settings";
static const char *key = "timeout";
hmap_add_value(hmap, prefix, key, 30);

// ✓ CORRECT: Caller-managed dynamic strings
char *my_key = strdup("dynamic_key");
hmap_add_value(hmap, "prefix", my_key, value);
// ... later, AFTER removing from hmap or destroying hmap:
free(my_key);

// ✗ WRONG: Local stack variable (will be invalid after function returns)
void bad_example() {
    char key[32];
    snprintf(key, sizeof(key), "key_%d", 42);
    hmap_add_value(hmap, "test", key, 123);  // DANGLING POINTER!
}
```

### Complete Memory Management Example

```c
hmap_t *config = hmap_create("app-config", 64, NULL);

// Add string value (caller-managed)
char *app_name = strdup("MyApp"); // can be a string literal or dynamic string
hmap_add_value(config, "app", "name", app_name);

// Add string array (caller-managed) with allocated elements
char **ips = malloc(2 * sizeof(char *));
ips[0] = strdup("192.168.1.1");
ips[1] = strdup("10.0.0.1");
hmap_add_array(config, "security", "ips", ips, 2);

// Add scalar values (copied by hmap, no management needed)
hmap_add_value(config, "app", "port", (uint32_t)8080);
hmap_add_value(config, "app", "timeout", (uint32_t)30);

// ... use the hmap ...

// Destroy hmap first
hmap_destroy(config);

// Then free your allocated memory
free(app_name); // because string values are caller-managed
for (int i = 0; i < 2; i++)
    free(ips[i]); // free each string in the array
free(ips);
```

## Performance Optimization

### Memory Allocation Behavior

HMAP uses a **flat array with linear probing** for optimal cache performance:
- ✓ No per-entry allocation - entries occupy pre-allocated array slots
- ✓ Cache-friendly 32-byte KVP structures
- ✓ Only allocates when resizing the array

**Key Insight**: The main allocation overhead comes from resizing, not per-entry operations.

### Best Practice: Pre-Size Your HashMap

**Simply set appropriate capacity at creation to eliminate all resizing:**

```c
// ✗ BAD: Default capacity causes 5+ reallocations for 1000 entries
hmap_t *hmap = hmap_create("config", 0, HMAP_USE_DEFAULT_CAPACITY);
for (int i = 0; i < 1000; i++)
    hmap_add_value(hmap, "test", keys[i], i);  // Multiple resizes!

// ✓ GOOD: Pre-size for expected capacity - ZERO reallocations
hmap_t *hmap = hmap_create("config", 0, 1024);
for (int i = 0; i < 1000; i++)
    hmap_add_value(hmap, "test", keys[i], i);  // No resizing needed
```

**Capacity Guidelines**:
- Set `max_capacity` to ~1.3x expected maximum entries
- Hash table performs best at 60-80% full
- Example: For 1000 entries, use capacity of 1300-1500

### Allocation Tracking

Enable allocation tracking to measure and validate performance:

```c
// Compile with -DHMAP_TRACK_ALLOCS to enable tracking

#ifdef HMAP_TRACK_ALLOCS
uint64_t allocs, reallocs, bytes;
hmap_get_global_alloc_stats(&allocs, &reallocs, &bytes);
printf("Allocations: %lu, Reallocations: %lu, Total bytes: %lu\n",
       allocs, reallocs, bytes);

// Per-hmap tracking
uint64_t hmap_allocs = hmap_get_alloc_count(hmap);
printf("This hmap performed %lu allocations\n", hmap_allocs);
#endif
```

### Performance Comparison

| Approach | Allocs (1000 inserts) | Performance | Use Case |
|----------|----------------------|-------------|----------|
| Default capacity | 5-7 (resizing) | Good | General use |
| **Pre-sized** | **1** | **Excellent** | **Recommended** |

## Basic Usage

### Creating a Hash Map

```c
#include "hmap.h"

// Create a hash map with default capacity (1024)
hmap_t *hmap = hmap_create("my-config", 0, HMAP_DEFAULT_CAPACITY);
if (!hmap) {
    // Handle error
}

// Create with custom capacity
hmap_t *hmap2 = hmap_create("large-map", 0, 4096);
```

### Adding Values

Using C11 `_Generic` macros for type-safe, readable code:

```c
// Scalar values - type is automatically inferred
hmap_add_value(hmap, "app", "name", "my-application");
hmap_add_value(hmap, "settings", "port", 8080);
hmap_add_value(hmap, "stats", "counter", -42L);
hmap_add_value(hmap, "metrics", "ratio", 3.14159);
hmap_add_value(hmap, "flags", "debug", true);

// Add without prefix (global keys)
hmap_add_value(hmap, NULL, "version", 1);

// Arrays - type is automatically inferred from array pointer
uint32_t ports[] = {8080, 8081, 8082, 8083};
char *names[] = {"alice", "bob", "charlie"};
double ratios[] = {1.5, 2.5, 3.75};

hmap_add_array(hmap, "config", "ports", ports, 4);
hmap_add_array(hmap, "users", "names", names, 3);
hmap_add_array(hmap, "stats", "ratios", ratios, 3);
```

### Retrieving Values

Using C11 `_Generic` macros for type-safe retrieval:

```c
// Scalar values - type is automatically inferred from pointer
uint32_t port;
char *name;
uint8_t debug;  // Boolean: 0=false, 1=true (stored as uint8_t)
double ratio;

hmap_get_value(hmap, "settings", "port", &port);
hmap_get_value(hmap, "app", "name", &name);
hmap_get_value(hmap, "flags", "debug", &debug);
hmap_get_value(hmap, "metrics", "ratio", &ratio);

// Arrays - type is automatically inferred from pointer
uint32_t *ports;
char **names;

int port_count = hmap_get_array(hmap, "config", "ports", &ports);
int name_count = hmap_get_array(hmap, "users", "names", &names);

// Access array elements
for (int i = 0; i < port_count; i++) {
    printf("Port[%d] = %u\n", i, ports[i]);
}
```

### Updating Values

```c
// Update existing value
hmap_kvp_t *kvp = hmap_kvp_lookup(hmap, "settings", "port");
if (kvp) {
    hmap_val_t new_val = {.u64 = 9090};
    hmap_kvp_update(hmap, kvp, &new_val);
} else {
	// Key not found
	hmap_add_value(hmap, "settings", "port", 9090);
}
```

### Checking Existence

```c
hmap_kvp_t *kvp = hmap_kvp_lookup(hmap, "settings", "port");
if (kvp) {
    printf("Key exists with type: %d\n", kvp->type);
} else {
    printf("Key not found\n");
}
```

### Iteration

```c
// List all entries (unsorted)
hmap_list(hmap, stdout, false);

// List all entries (sorted by key)
hmap_list(hmap, stdout, true);

// Iterate programmatically
hmap_kvp_t *kvp_list[HMAP_DEFAULT_CAPACITY];
int count = hmap_kvp_list(hmap, kvp_list, HMAP_DEFAULT_CAPACITY, false);

for (int i = 0; i < count; i++) {
    hmap_kvp_t *kvp = kvp_list[i];
    printf("Prefix: %s, Key: %s, Type: %d\n",
           kvp->prefix ? kvp->prefix : "(none)",
           kvp->key,
           kvp->type);
}
```

### Cleanup

```c
// Destroy the hash map and free all resources
hmap_destroy(hmap);
```

## Complete Example

```c
#include <stdio.h>
#include "hmap.h"

int main(void)
{
    // Create hash map
    hmap_t *config = hmap_create("app-config", 0, 64);
    if (!config)
        return -1;

    // Add configuration values - keys and prefixes are automatically duplicated by hmap
    // String values need dynamic allocation (caller-managed)
    char *app_name = strdup("MyApp");
    hmap_add_value(config, "app", "name", app_name);

    // Scalar values are copied by hmap (no management needed)
    hmap_add_value(config, "app", "version", (uint32_t)2);
    hmap_add_value(config, "server", "port", (uint32_t)8080);
    hmap_add_value(config, "server", "ssl", (uint8_t)1);  // Boolean as uint8_t: 0=false, 1=true
    hmap_add_value(config, "server", "timeout", (uint32_t)30);

    // Add array of allowed IPs (caller-managed allocation)
    char **allowed_ips = malloc(3 * sizeof(char *));
    allowed_ips[0] = strdup("192.168.1.1");
    allowed_ips[1] = strdup("10.0.0.1");
    allowed_ips[2] = strdup("172.16.0.1");
    hmap_add_array(config, "security", "allowed_ips", allowed_ips, 3);

    // Retrieve and display values using simplified API
    char *retrieved_name;
    uint32_t version, port, timeout;
    uint8_t ssl;  // Boolean: 0=false, 1=true

    hmap_get_value(config, "app", "name", &retrieved_name);
    hmap_get_value(config, "app", "version", &version);
    hmap_get_value(config, "server", "port", &port);
    hmap_get_value(config, "server", "ssl", &ssl);
    hmap_get_value(config, "server", "timeout", &timeout);

    printf("Application: %s v%u\n", retrieved_name, version);
    printf("Server: port=%u, ssl=%s, timeout=%us\n",
           port, ssl ? "enabled" : "disabled", timeout);

    // Retrieve and display allowed IPs
    char **ips;
    int ip_count = hmap_get_array(config, "security", "allowed_ips", &ips);

    printf("Allowed IPs (%d):\n", ip_count);
    for (int i = 0; i < ip_count; i++) {
        printf("  %d. %s\n", i + 1, ips[i]);
    }

    // List all configuration
    printf("\nComplete configuration:\n");
    hmap_list(config, stdout, true);

    // Cleanup: Destroy hmap (automatically frees keys/prefixes), then free caller-managed memory
    hmap_destroy(config);

    // Free caller-managed string value
    free(app_name);

    // Free caller-managed string array and its elements
    for (int i = 0; i < 3; i++)
        free(allowed_ips[i]);
    free(allowed_ips);

    return 0;
}
```

## Supported Types

### Scalar Types
- **Strings**: `char *` - Null-terminated C strings
- **Integers**: `int8_t`, `int16_t`, `int32_t`, `int64_t`
- **Unsigned**: `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`
- **Floating**: `double` - 64-bit IEEE 754
- **Boolean**: `uint8_t` - Stored as 0 (false) or 1 (true), use uint8_t type
- **Pointer**: `void *` - Generic pointer

### Array Types
All scalar types have corresponding array versions:
- `char **` - String arrays
- `uint32_t *`, `int64_t *`, etc. - Numeric arrays
- `uint8_t *` - Boolean arrays (0=false, 1=true)
- `void **` - Pointer arrays

## Advanced Features

### Custom Hash Functions

```c
// Define custom hash function
uint32_t my_hash(const char *prefix, const char *key) {
    // Your hash implementation
    return custom_hash_value;
}

// Create map with custom functions
hmap_funcs_t funcs = {
    .hash_fn = my_hash,
    .cmp_fn = default_cmp,
    .free_fn = default_free
};

hmap_t *hmap = hmap_create_with_funcs("custom", 0, 128, &funcs);
```

### Prefix-Based Organization

Prefixes allow logical grouping of related keys:

```c
// Group by functional area
hmap_add_u32(hmap, "network", "port", 8080);
hmap_add_u32(hmap, "network", "timeout", 30);
hmap_add_bool(hmap, "network", "ssl", true);

hmap_add_string(hmap, "database", "host", "localhost");
hmap_add_u32(hmap, "database", "port", 5432);
hmap_add_string(hmap, "database", "name", "mydb");

// Retrieve by prefix
uint32_t net_port, db_port;
hmap_get_value(hmap, "network", "port", &net_port);    // 8080
hmap_get_value(hmap, "database", "port", &db_port);    // 5432
```

### Memory Management

HMAP manages memory automatically:
- **Strings**: Copies are made internally, freed on destroy
- **Arrays**: Stores pointers only - caller manages array memory
- **String arrays**: Deep copies made for string array elements
- **Updates**: Old values automatically freed when updated

```c
// String is copied internally
hmap_add_string(hmap, "app", "name", "MyApp");
// Safe to free or modify original string

// Array pointer is stored (not copied)
uint32_t ports[] = {8080, 8081};
hmap_add_u32_array(hmap, "config", "ports", ports, 2);
// Caller must keep 'ports' valid while in hmap

// String array elements are deep copied
char *names[] = {"alice", "bob"};
hmap_add_string_array(hmap, "users", "names", names, 2);
// Safe to free 'names' array after adding
```

### Performance Considerations

- **Initial Capacity**: Choose based on expected entries to minimize rehashing
- **Cache Alignment**: Structures optimized for 64-byte cache lines
- **Open Addressing**: Fast lookups with good cache locality
- **Type-Tagged Storage**: Zero overhead for most types (64-bit values)
- **Count in KVP**: Arrays have zero allocation overhead for metadata

## Error Handling

Most functions return:
- `0` on success, `-1` on failure (add/update/get operations)
- `NULL` on failure (creation, lookup operations)
- Array count or `-1` on failure (array get operations)

```c
if (hmap_add_u32(hmap, "config", "port", 8080) != 0) {
    fprintf(stderr, "Failed to add port\n");
}

hmap_kvp_t *kvp = hmap_kvp_lookup(hmap, "config", "port");
if (!kvp) {
    fprintf(stderr, "Key not found\n");
}
```

## Best Practices

1. **Use the generic macros**: `hmap_add_value()`, `hmap_add_array()`, `hmap_get_value()`, and `hmap_get_array()` provide type-safe, readable code
2. **Choose meaningful prefixes**: Group related configuration logically
3. **Set appropriate capacity**: Avoid rehashing by estimating entry count
4. **Check return values**: Always verify operations succeeded
5. **Manage array lifetimes**: Keep array data valid while referenced by hmap
6. **Use sorted listings**: Enable sorting for better debugging/inspection
7. **Destroy when done**: Always call `hmap_destroy()` to prevent leaks

## API Reference

### Creation & Destruction
- `hmap_create()` - Create with default functions
- `hmap_create_with_funcs()` - Create with custom functions
- `hmap_create_with_ext_storage()` - Create with external storage
- `hmap_destroy()` - Free all resources

### Adding Values
- **`hmap_add_value(hmap, prefix, key, value)`** - Type-safe scalar setter
  - Supports: `char*`, `bool`, `uint64_t`, `uint32_t`, `uint16_t`, `uint8_t`, `int64_t`, `int32_t`, `int16_t`, `int8_t`, `double`, `float`, `void*`
  - Returns: `0` on success, `-1` on failure

- **`hmap_add_array(hmap, prefix, key, array, count)`** - Type-safe array setter
  - Supports arrays of all scalar types above
  - Returns: `0` on success, `-1` on failure

### Getting Values
- **`hmap_get_value(hmap, prefix, key, &value)`** - Type-safe scalar getter
  - Type inferred from value pointer
  - Returns: `0` on success, `-1` on failure

- **`hmap_get_array(hmap, prefix, key, &array)`** - Type-safe array getter
  - Type inferred from array pointer
  - Returns: Array count on success, `-1` on failure

### Lookup & Update
- `hmap_kvp_lookup()` - Find key-value pair
- `hmap_kvp_update()` - Update existing value

### Inspection
- `hmap_list()` - Print to file stream
- `hmap_kvp_list()` - Get array of pointers
- `hmap_iterate()` - Callback-based iteration
- `hmap_get_funcs()` - Get function pointers

### Implementation Details

The generic macros use C11 `_Generic` to dispatch to internal type-specific functions (prefixed with `_hmap_`). These internal functions are not part of the public API and should not be called directly.

## See Also

- [JCFG Library](../jcfg/) - JSON configuration parser built on HMAP
- [Examples](../../../examples/) - Complete usage examples
- [hmap.h](hmap.h) - Full API documentation
