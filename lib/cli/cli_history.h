/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CLI_HISTORY_H_
#define _CLI_HISTORY_H_

/**
 * @file
 * RTE Command line history
 *
 */

#include "cli.h"
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CLI_DEFAULT_HISTORY = -1,       /** Use the default history count */
    CLI_NO_HISTORY = 0,             /** No history */
};

struct cli_hist {
    CIRCLEQ_ENTRY(cli_hist) next;   /** link list of history lines */
    char *line;                     /** line is strdup of original line */
};

struct cli;

/**
* Create and allocate a history structure
*
* @note Uses the thread variable this_cli
*
* @return
*   pointer to history structure or NULL on error
*/
struct cli_hist *cli_hist_alloc(void);

/**
* Free a CLI history structure and other memory
*
* @note Uses the thread variable this_cli
*
* @param hist
*   Pointer to the history structure
* @return
*   N/A
*/
void cli_hist_free(struct cli_hist *hist);

/**
* Add line to history at the end
*
* @note Uses the thread variable this_cli
*
* @param line
*   Pointer to string to add to the history list
* @return
*   N/A
*/
void cli_history_add(char *line);

/**
* Delete a history entry
*
* @note Uses the thread variable this_cli
*
* @return
*   N/A
*/
void cli_history_del(void);

/**
* Return the history command from line number
*
* @note Uses the thread variable this_cli
*
* @param lineno
*   The line number of the command to return.
* @return
*   Pointer to line or NULL on error
*/
char *cli_history_line(int lineno);

/**
* Clear all of the history lines from the list
*
* @note Uses the thread variable this_cli
*
* @return
*   N/A
*/
void cli_history_clear(void);

/**
* Delete the history lines and structure
*
* @note Uses the thread variable this_cli
*
* @return
*   N/A
*/
void cli_history_delete(void);

/**
* Set the number of lines max in the history list
*
* @param cli
*   Pointer to the allocated cli structure
* @param nb_hist
*   Number of lines max in the history list
* @return
*   0 is ok, -1 is error
*/
int cli_set_history(uint32_t nb_hist);

/**
* Return the previous history line
*
* @note Uses the thread variable this_cli
*
* @return
*   pointer to the pervious history line wrap if needed
*/
char *cli_history_prev(void);

/**
* Return the next history line
*
* @param cli
*   Pointer to the allocated cli structure
* @return
*   pointer to the next history line wrap if needed
*/
char * cli_history_next(void);

/**
* Reset the current history pointer to the last entry.
*
* @note Uses the thread variable this_cli
*/
void cli_history_reset(void);

/**
* Add the default set of directories and commands
*
* @note Uses the thread variable this_cli
*
* @return
*   0 is ok, -1 is error
*/
void cli_history_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_HISTORY_H_ */
