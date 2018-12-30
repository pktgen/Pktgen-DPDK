/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
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
