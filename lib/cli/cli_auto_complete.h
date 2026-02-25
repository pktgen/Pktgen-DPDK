/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2026>, Intel Corporation.
 */

#ifndef _CLI_AUTO_COMPLETE_H_
#define _CLI_AUTO_COMPLETE_H_

/**
 * @file
 * CLI auto-complete.
 *
 * Handles TAB completion for commands/paths and optionally uses registered
 * cli_map tables to offer context-aware token hints.
 */

#include "cli.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Handle the tab key for auto-complete.
 *
 * This function reads the current input buffer, computes candidate
 * completions, and either inserts text (single match) or prints a candidate
 * list (multiple matches).
 *
 * @return
 *   N/A
 */
void cli_auto_complete(void);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_AUTO_COMPLETE_H_ */
