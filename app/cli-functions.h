/*-
 * Copyright(c) <2020-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _CLI_COMMANDS_H_
#define _CLI_COMMANDS_H_

/**
 * @file
 *
 * Pktgen interactive CLI command tree creation.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build and register the full Pktgen CLI command tree.
 *
 * Registers all command maps, directory nodes, and help strings with the
 * CLI engine.  Must be called once during initialisation before entering
 * the interactive shell loop.
 *
 * @return
 *   0 on success, -1 on failure.
 */
int pktgen_cli_create(void);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_COMMANDS_H_ */
