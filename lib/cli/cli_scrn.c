/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */

/* Created by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <rte_atomic.h>
#include <rte_malloc.h>
#include <rte_spinlock.h>

#include <cli.h>
#include "cli_scrn.h"
#include "cli_input.h"

RTE_DEFINE_PER_LCORE(struct cli_scrn *, scrn);

void
__attribute__((format(printf, 3, 4)))
scrn_printf(int16_t r, int16_t c, const char *fmt, ...)
{
	va_list vaList;

	if ( (r != 0) && (c != 0) )
		scrn_pos(r, c);
	va_start(vaList, fmt);
	vfprintf(this_scrn->fd_out, fmt, vaList);
	va_end(vaList);
	fflush(this_scrn->fd_out);
}

void
__attribute__((format(printf, 3, 4)))
scrn_cprintf(int16_t r, int16_t ncols, const char *fmt, ...)
{
	va_list vaList;
	char str[512];

	if (ncols == -1)
		ncols = this_scrn->ncols;

	va_start(vaList, fmt);
	vsnprintf(str, sizeof(str), fmt, vaList);
	va_end(vaList);

	scrn_pos(r, scrn_center_col(ncols, str));
	scrn_puts("%s", str);
}

void
__attribute__((format(printf, 4, 5)))
scrn_fprintf(int16_t r, int16_t c, FILE *f, const char *fmt, ...)
{
	va_list vaList;

	if ( (r != 0) && (c != 0) )
		scrn_pos(r, c);
	va_start(vaList, fmt);
	vfprintf(f, fmt, vaList);
	va_end(vaList);
	fflush(f);
}

static void
scrn_set_io(FILE *in, FILE *out)
{
	struct cli_scrn *scrn = this_scrn;

	if (scrn) {
		if (scrn->fd_in && (scrn->fd_in != stdin))
			fclose(scrn->fd_in);

		if (scrn->fd_out && (scrn->fd_out != stdout))
			fclose(scrn->fd_in);

		scrn->fd_in = in;
		scrn->fd_out = out;
	}
}

static int
scrn_stdin_setup(void)
{
	struct cli_scrn *scrn = this_scrn;
	struct termios term;

	if (!scrn)
		return -1;

	scrn_set_io(stdin, stdout);

	memset(&scrn->oldterm, 0, sizeof(term));
	if (tcgetattr(fileno(scrn->fd_in), &scrn->oldterm) ||
	    tcgetattr(fileno(scrn->fd_in), &term)) {
		fprintf(stderr, "%s: setup failed for tty\n", __func__);
		return -1;
	}

	term.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);

	if (tcsetattr(fileno(scrn->fd_in), TCSANOW, &term)) {
		fprintf(stderr, "%s: failed to set tty\n", __func__);
		return -1;
	}

	return 0;
}

static void
scrn_stdin_restore(void)
{
	struct cli_scrn *scrn = this_scrn;

	if (!scrn)
		return;

	if (tcsetattr(fileno(scrn->fd_in), TCSANOW, &scrn->oldterm))
		fprintf(stderr, "%s: failed to set tty\n", __func__);

	if (system("stty sane"))
		fprintf(stderr, "%s: system command failed\n", __func__);
}

static void
handle_winch(int sig)
{
	struct winsize w;

	if (sig != SIGWINCH)
		return;

	ioctl(0, TIOCGWINSZ, &w);

	this_scrn->nrows = w.ws_row;
	this_scrn->ncols = w.ws_col;

	/* Need to refreash the screen */
	//cli_clear_screen();
	cli_clear_line(-1);
	cli_redisplay_line();
}

int
scrn_create(int scrn_type, int theme)
{
	struct winsize w;
	struct cli_scrn *scrn = this_scrn;
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handle_winch;
	sigaction(SIGWINCH, &sa, NULL);

	if (!scrn) {
		scrn = malloc(sizeof(struct cli_scrn));
		if (!scrn)
			return -1;
		memset(scrn, 0, sizeof(struct cli_scrn));

		this_scrn = scrn;
	}

	rte_atomic32_set(&scrn->pause, SCRN_SCRN_PAUSED);

	ioctl(0, TIOCGWINSZ, &w);

	scrn->nrows = w.ws_row;
	scrn->ncols = w.ws_col;
	scrn->theme = theme;
	scrn->type  = scrn_type;

	if (scrn_type == SCRN_STDIN_TYPE) {
		if (scrn_stdin_setup()) {
			free(scrn);
			return -1;
		}
	} else {
		free(scrn);
		return -1;
	}

	scrn_color(SCRN_DEFAULT_FG, SCRN_DEFAULT_BG, SCRN_OFF);

	return 0;
}

int
scrn_create_with_defaults(int theme)
{
	return scrn_create(SCRN_STDIN_TYPE,
	                   (theme)? SCRN_THEME_ON : SCRN_THEME_OFF);
}

void
scrn_destroy(void)
{
	if (this_scrn && (this_scrn->type == SCRN_STDIN_TYPE))
		scrn_stdin_restore();
}
