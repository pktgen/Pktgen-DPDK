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

#include <stdio.h>
#include <poll.h>
#include <string.h>

#include <rte_version.h>
#include <rte_timer.h>
#include <rte_log.h>
#include <rte_string_fns.h>

#include "cli.h"
#include "cli_input.h"
#include "cli_string_fns.h"

int (*lua_dofile)(void *, const char *);

int
cli_yield_io(void)
{
	if (!this_cli)
		return 1;
	return this_cli->flags & CLI_YIELD_IO;
}

/* The CLI write routine, using write() call */
int
cli_write(const void *msg, int len)
{
	return scrn_write(msg, len);
}

int
cli_read(char *buf, int len)
{
	return scrn_read(buf, len);
}

static void
handle_input_display(char c)
{
	/* Only allow printable characters */
	if ((c >= ' ') && (c <= '~')) {
		/* Output the character typed */
		cli_write(&c, 1);

		/* Add the character to the buffer */
		gb_insert(this_cli->gb, c);
		if (!gb_point_at_end(this_cli->gb))
			cli_set_flag(DISPLAY_LINE);
		else if (!gb_point_at_start(this_cli->gb))
			cli_set_flag(DISPLAY_LINE);
	}
	cli_display_line();
}

/* Process the input for the CLI from the user */
void
cli_input(char *str, int n)
{
	RTE_ASSERT(this_cli->gb != NULL);
	RTE_ASSERT(str != NULL);

	while (n--) {
		char c = *str++;

		int ret = vt100_parse_input(this_cli->vt, c);

		if (ret > 0) {	/* Found a vt100 key sequence */
			vt100_do_cmd(ret);
			handle_input_display(0);
		} else if (ret == VT100_DONE)
			handle_input_display(c);
	}
}

/* Poll the I/O routine for characters */
int
cli_poll(char *c)
{
	struct pollfd fds;

	fds.fd      = fileno(this_scrn->fd_in);
	fds.events  = POLLIN;
	fds.revents = 0;

	if (cli_use_timers())
		rte_timer_manage();

	if (poll(&fds, 1, 0)) {
		if ((fds.revents & (POLLERR | POLLNVAL)) == 0) {
			if ((fds.revents & POLLHUP))
				this_cli->quit_flag = 1;
			else if ((fds.revents & POLLIN)) {
				int n = read(fds.fd, c, 1);
				if (n > 0)
					return 1;
			}
		} else
			cli_quit();
	}
	return 0;
}

/* Display a prompt and wait for a key press */
char
cli_pause(const char *msg, const char *keys)
{
	char prompt[128], c;

	prompt[0] = '\0';

	if (msg) {
		strcpy(prompt, msg);
		strcat(prompt, ": ");
		cli_printf("%s", prompt);
	}

	if (!keys)
		keys = " qQ\n\r" ESC;

	do {
		if (cli_poll(&c))
			if (strchr(keys, c)) {
				/* clear the line of the prompt */
				cli_printf("\r%*s\r", (int)strlen(prompt), " ");
				return c;
			}
	} while (this_cli->quit_flag == 0);

	return '\0';
}

