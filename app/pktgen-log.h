/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_LOG_H_
#define _PKTGEN_LOG_H_

#include <stdint.h>


/* Log levels. Each log level has an associated pktgen_log_<LEVEL>()
 * function. */
#define LOG_LEVEL_ALL		0
#define LOG_LEVEL_TRACE		1
#define LOG_LEVEL_DEBUG		2
#define LOG_LEVEL_INFO		3
#define LOG_LEVEL_WARNING	4
#define LOG_LEVEL_ERROR		5
#define LOG_LEVEL_PANIC		6
#define LOG_LEVEL_NONE		7

/* Set default minimum message level to log if one isn't provided at compile
 * time. All pktgen_log_<LEVEL>() calls with a log level lower than the one
 * specified below won't even be compiled.
 * More detailed logs have a negative performance impact, which is undesirable
 * in a production build.
 */
#ifndef LOG_LEVEL
#define LOG_LEVEL	LOG_LEVEL_INFO
#endif

/* Conditionally generate code for pktgen_log_<LEVEL>() functions, depending on
 * the minimum requested log level.
 */
#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define pktgen_log_trace(fmt, ...) pktgen_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#define pktgen_log_trace(fmt, ...) /* no-op */
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define pktgen_log_debug(fmt, ...) pktgen_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#define pktgen_log_debug(fmt, ...) /* no-op */
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define pktgen_log_info(fmt, ...) pktgen_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#define pktgen_log_info(fmt, ...) /* no-op */
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define pktgen_log_warning(fmt, ...) pktgen_log(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#define pktgen_log_warning(fmt, ...) /* no-op */
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define pktgen_log_error(fmt, ...) pktgen_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#define pktgen_log_error(fmt, ...) /* no-op */
#endif

#if LOG_LEVEL <= LOG_LEVEL_PANIC
#define pktgen_log_panic(fmt, ...) do {                                        \
		pktgen_log(LOG_LEVEL_PANIC, __FILE__, __LINE__, __FUNCTION__,          \
				fmt, ##__VA_ARGS__);                                           \
		rte_panic(fmt, ##__VA_ARGS__);                                         \
	} while (0)
#else
#define pktgen_log_panic(fmt, ...) rte_panic(fmt, ##__VA_ARGS__)
#endif


/* Helper for building log strings.
 * The macro takes an existing string, a printf-like format string and optional
 * arguments. It formats the string and appends it to the existing string, while
 * avoiding possible buffer overruns.
 */
#define strncatf(dest, fmt, ...) do {                                          \
		char _buff[1024];                                                      \
		snprintf(_buff, sizeof(_buff), fmt, ##__VA_ARGS__);                    \
		strncat(dest, _buff, sizeof(dest) - strlen(dest) - 1);                 \
	} while (0)


/* Initialize log data structures */
extern void pktgen_init_log(void);


/**************************************************************************//**
*
* pktgen_log_set_screen_level - Set level of messages that are printed to the screen
*
* DESCRIPTION
* Messages of the specified level or higher are printed to the screen
* in addition to being logged to the log page and optionally to a file.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_log_set_screen_level(int level);


/**************************************************************************//**
*
* pktgen_log - printf-like function for logging
*
* DESCRIPTION
* Log the provided message to the log page and to a file if enabled.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_log(int level, const char *file, long line,
		const char *func, const char *fmt, ...);


/**************************************************************************//**
*
* pktgen_log_set_file - Start logging to a file
*
* DESCRIPTION
* Writes the log to the provided filename. If the file already exists, it is
* truncated.
*
* RETURNS: N/A
*
* SEE ALSO:
*/
extern void pktgen_log_set_file(const char *filename);


/**************************************************************************//**
 *
 * pktgen_page_log - Display the log page.
 *
 * DESCRIPTION
 * Display the log page on the screen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
extern void pktgen_page_log(uint32_t print_labels);


#endif	// _PKTGEN_LOG_H_
