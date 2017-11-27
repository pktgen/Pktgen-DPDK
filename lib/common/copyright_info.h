/*-
 *   Copyright(c) 2014-2017 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2013 by Keith Wiles @ intel.com */

#ifndef _COPYRIGHT_INFO_H
#define _COPYRIGHT_INFO_H

void print_copyright(const char *appname, const char *created_by);
void logo(int row, int col, const char *appname);
void splash_screen(int row,
			  int col,
			  const char *appname,
			  const char *created_by);

/**
 * Function returning string for Copyright message."
 * @return
 *     string
 */
const char *copyright_msg(void);

/**
 * Function returning short string for Copyright message."
 * @return
 *     string
 */
const char *copyright_msg_short(void);

/**
 * Function returning string for Copyright message."
 * @return
 *     string
 */
const char *powered_by(void);

#endif /* _COPYRIGHT_INFO_H */
