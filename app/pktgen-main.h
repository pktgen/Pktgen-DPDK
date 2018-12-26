/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_MAIN_H_
#define _PKTGEN_MAIN_H_

#include <stdint.h>
#include <termios.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void pktgen_l2p_dump(void);

void pktgen_interact(void);

void *pktgen_get_lua(void);

void pktgen_stop_running(void);

/**************************************************************************//**
 *
 * pktgen_get_lua - Get Lua state pointer.
 *
 * DESCRIPTION
 * Get the Lua state pointer value.
 *
 * RETURNS: Lua pointer
 *
 * SEE ALSO:
 */

void *pktgen_get_lua(void);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_MAIN_H_ */
