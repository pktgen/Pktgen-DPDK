/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2026>, Intel Corporation.
 */

#ifndef _CLI_SEARCH_H_
#define _CLI_SEARCH_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>

/**
 * @file
 * CLI node search and directory scanning.
 *
 * Provides helpers to locate nodes in the CLI tree (commands/files/directories)
 * and to enumerate nodes in a directory or along the executable search path.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Generic 64-bit argument word that can be interpreted several ways. */
typedef union {
    void *voidp;     /**< Pointer value */
    char chr[8];     /**< Up to 8 raw bytes */
    uint64_t u64;    /**< 64-bit unsigned integer */
    uint32_t u32[2]; /**< Two 32-bit unsigned integers */
    uint16_t u16[4]; /**< Four 16-bit unsigned integers */
} arg_u;

/** Four-word argument block passed to cli_scan_t callbacks. */
typedef struct {
    arg_u arg1; /**< Argument word 1 */
    arg_u arg2; /**< Argument word 2 */
    arg_u arg3; /**< Argument word 3 */
    arg_u arg4; /**< Argument word 4 */
} args_t;

struct cli;
struct cli_node;

/** Callback invoked by cli_scan_directory() for each matching node. */
typedef int (*cli_scan_t)(struct cli_node *node, uint32_t flags, args_t *args);

/**
 * Scan a directory and call the func with the node found.
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param dir
 *   Node pointer to directory to scan
 * @param func
 *   cli_scan_t function pointer
 * @param flags
 *   Bitmap of node types to include (e.g., CLI_CMD_NODE|CLI_DIR_NODE)
 * @param args
 *   Optional argument block passed to @p func
 * @return
 *   0 on success or -1 on error
 */
int cli_scan_directory(struct cli_node *dir, cli_scan_t func, uint32_t flags, args_t *args);

/**
 * Find a node by path
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param path
 *   Path to node
 * @param ret
 *   Pointer to pointer of a cli_node if found
 * @return
 *   1 if found else 0
 */
int cli_find_node(const char *path, struct cli_node **ret);

/**
 * Search the local and bin directories for a command
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param path
 *   String for the command to use
 * @return
 *   Pointer to the command node or NULL
 */
struct cli_node *cli_find_cmd(const char *path);

/**
 * Count the number of nodes of the given type(s) in a directory.
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param n
 *   node or NULL for current working directory
 * @return
 *   Number of nodes found of this type in the directory
 */
uint32_t cli_dir_item_count(struct cli_node *node, uint32_t types);

/**
 * Count the number of commands in the execute path
 *
 * @note Uses a thread variable called this_cli.
 *
 * @return
 *   Number of nodes found of this type in the directory
 */
uint32_t cli_path_cmd_count(void);

/**
 * Return a list of nodes matching given information
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param node
 *   Node to start search or use the path list.
 * @param flags
 *   Type of nodes to return
 * @param ret
 *   Pointer to an array of pointer for return value
 * @return
 *   Number of nodes found of this type in the directory
 */
uint32_t cli_node_list_with_type(struct cli_node *node, uint32_t flags, void **ret);

/**
 * Free a list returned by cli_node_list_with_type().
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param node
 *   Pointer to the node to free
 * @return
 *   N/A
 */
void cli_node_list_free(void *nodes);

/**
 * Count the number of commands in the execute path
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param types
 *   The number of nodes to count
 * @return
 *   Number of nodes found of this type in the directory
 */
uint32_t cli_path_item_count(uint32_t types);

/**
 * Find and return the last directory node in a given path string.
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param path
 *   Path string to scan
 * @return
 *   Pointer to last directory node in path
 */
struct cli_node *cli_last_dir_in_path(const char *path);

/**
 * Scan a directory for a node matching a name and type.
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param dir
 *   Pointer to directory node to start with in scanning
 * @param name
 *   String to match the nodes with
 * @param type
 *   Type of nodes to include in scan.
 * @return
 *   Number of nodes found of this type in the directory
 */
struct cli_node *cli_search_dir(struct cli_node *dir, const char *name, uint32_t type);

/**
 * Scan the directory given by @p path.
 *
 * @note Uses a thread variable called this_cli.
 *
 * @param path
 *   The path string to use
 * @param func
 *   The function to call when a match is found.
 * @param flags
 *   Type of files to include in match
 * @param args
 *   Arguments to include with function call.
 * @return
 *   Number of nodes found of this type in the directory
 */
int cli_scan_path(const char *path, cli_scan_t func, uint32_t flags, args_t *args);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_SEARCH_H_ */
