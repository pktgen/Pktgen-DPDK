/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2026>, Intel Corporation.
 */

#ifndef _CLI_CMDS_H_
#define _CLI_CMDS_H_

/**
 * @file
 * CLI built-in command tree.
 *
 * Provides helpers to populate the default directory structure and common
 * built-in commands (e.g., ls/cd/pwd/help/history).
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add the default set of directories and commands.
 *
 * @note Uses a thread variable called this_cli
 *
 * @return
 *   0 is ok, -1 is error
 */
int cli_default_tree_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_CMDS_H_ */
