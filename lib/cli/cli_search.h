/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
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
 * RTE Command line interface
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
	void *voidp;                    /** void * value */
	char chr[8];                    /** 8 characters */
	uint64_t u64;                   /** 64bit value */
	uint32_t u32[2];                /** 2 32bit value */
	uint16_t u16[4];                /** 4 16bit values */
} arg_u;                            /** Union for argument word */

typedef struct {
	arg_u   arg1;                   /* Argument Word 1 */
	arg_u   arg2;                   /* Argument Word 2 */
	arg_u   arg3;                   /* Argument Word 3 */
	arg_u   arg4;                   /* Argument Word 4 */
} args_t;                           /* 32 bytes of arguments */

struct cli;
struct cli_node;

typedef int (*cli_scan_t)(struct cli_node *node,
                          uint32_t flags, args_t *args);
/** typedef of function passed in cli_scan_directory() */

/**
* Scan a directory and call the func with the node found.
*
* @note Uses a thread variable called this_cli.
*
* @param files
*   Type of nodes to find
* @param dir
*   Node pointer to directory to scan
* @param func
*   cli_scan_t function pointer
* @param arg
*   void * used by the function being called.
* @return
*   0 on success or -1 on error
*/
int cli_scan_directory(struct cli_node *dir,
                       cli_scan_t func, uint32_t flags, args_t *args);

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
int cli_find_node( const char *path, struct cli_node **ret);

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
* Count the number of type(s) of nodes available in a given node
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
uint32_t cli_node_list_with_type(struct cli_node *node,
                                 uint32_t flags, void **ret);

/**
* Free a node back to the free list
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
* Find and return the last node in a give path string
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
* Scan a directory for a given string matching name
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
struct cli_node *cli_search_dir(struct cli_node *dir,
                                const char *name, uint32_t type);

/**
* Scan the directory given by the path
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
int cli_scan_path(const char *path, cli_scan_t func,
                  uint32_t flags, args_t *args);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_SEARCH_H_ */
