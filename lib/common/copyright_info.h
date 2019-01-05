/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2013 by Keith Wiles @ intel.com */

#ifndef _COPYRIGHT_INFO_H
#define _COPYRIGHT_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Print out a copyright string
 *
 * @param appname
 *   The name of the application
 * @param created_by
 *   The created_by string
 */
void print_copyright(const char *appname, const char *created_by);

/**
 * Function returning string for Copyright message.
 * @return
 *     string
 */
const char *copyright_msg(void);

/**
 * Function returning short string for Copyright message.
 * @return
 *     string
 */
const char *copyright_msg_short(void);

/**
 * Function returning string for Copyright message.
 * @return
 *     string
 */
const char *powered_by(void);

#ifdef __cplusplus
}
#endif

#endif /* _COPYRIGHT_INFO_H */
