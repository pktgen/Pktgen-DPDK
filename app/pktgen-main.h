/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_MAIN_H_
#define _PKTGEN_MAIN_H_

/**
 * @file
 *
 * Pktgen main loop and lifecycle control functions.
 */

#include <stdint.h>
#include <termios.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enter the interactive CLI loop and block until the user quits.
 *
 * Runs the CLI read-eval-print loop on the calling lcore, processing
 * user commands until pktgen_stop_running() signals termination.
 */
void pktgen_interact(void);

/**
 * Return the Lua state pointer for the active Lua instance.
 *
 * @return
 *   Pointer to the lua_State, or NULL if Lua is not enabled.
 */
void *pktgen_get_lua(void);

/**
 * Signal the main loop to stop and exit gracefully.
 */
void pktgen_stop_running(void);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_MAIN_H_ */
