/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */

#include "cli.h"

struct cli_hist *
cli_hist_alloc(void)
{
	struct cli *cli = this_cli;
	struct cli_hist *hist = NULL;

	if (!cli)
		return hist;

	if (!CIRCLEQ_EMPTY(&cli->free_hist)) {
		hist = (struct cli_hist *)CIRCLEQ_FIRST(&cli->free_hist);
		CIRCLEQ_REMOVE(&cli->free_hist, hist, next);
	}
	return hist;
}

void
cli_hist_free(struct cli_hist *hist)
{
	struct cli *cli = this_cli;

	if (!cli || !hist)
		return;

	free(hist->line);

	hist->line = NULL;

	CIRCLEQ_INSERT_TAIL(&cli->free_hist, hist, next);
}

void
cli_history_add(char *line)
{
	struct cli *cli = this_cli;
	struct cli_hist *h;

	if (!cli || !cli->hist_mem || !line)
		return;

	/* Do not allow duplicate lines compared to the last line */
	if (!CIRCLEQ_EMPTY(&cli->hd_hist)) {
		h = CIRCLEQ_LAST(&cli->hd_hist);
		if (strcmp(h->line, line) == 0)
			return;
	}

	h = cli_hist_alloc();
	if (!h) {
		h = CIRCLEQ_FIRST(&cli->hd_hist);

		CIRCLEQ_REMOVE(&cli->hd_hist, h, next);
	}

	free(h->line);
	h->line = strdup(line);
	CIRCLEQ_INSERT_TAIL(&cli->hd_hist, h, next);
}

void
cli_history_del(void)
{
	struct cli *cli = this_cli;
	struct cli_hist *h;

	if (!cli || !cli->hist_mem)
		return;

	if (!CIRCLEQ_EMPTY(&cli->hd_hist)) {
		h = CIRCLEQ_LAST(&cli->hd_hist);
		CIRCLEQ_REMOVE(&cli->hd_hist, h, next);
	}
}

char *
cli_history_line(int lineno)
{
	struct cli *cli = this_cli;
	struct cli_hist *h;
	int i = 0;

	if (!cli || !cli->hist_mem)
		return NULL;

	if (!CIRCLEQ_EMPTY(&cli->hd_hist)) {
		CIRCLEQ_FOREACH(h, &cli->hd_hist, next) {
			if (i++ == lineno)
				return h->line;
		}
	}
	return NULL;
}

char *
cli_history_prev(void)
{
	struct cli *cli = this_cli;

	if (!cli || !cli->hist_mem)
		return NULL;

	if (!CIRCLEQ_EMPTY(&cli->hd_hist)) {
		struct cli_hist *hist;

		if (!cli->curr_hist) {
			cli->curr_hist = CIRCLEQ_LAST(&cli->hd_hist);
			return cli->curr_hist->line;
		}

		if (cli->curr_hist == CIRCLEQ_FIRST(&cli->hd_hist))
			return cli->curr_hist->line;

		hist = CIRCLEQ_LOOP_PREV(&cli->hd_hist, cli->curr_hist, next);
		cli->curr_hist = hist;

		return hist->line;
	}
	return NULL;
}

char *
cli_history_next(void)
{
	struct cli *cli = this_cli;

	if (!cli || !cli->hist_mem)
		return NULL;

	if (!CIRCLEQ_EMPTY(&cli->hd_hist)) {
		struct cli_hist *hist;

		if (!cli->curr_hist)
			return NULL;

		if (cli->curr_hist == CIRCLEQ_LAST(&cli->hd_hist))
			return (char *)(uintptr_t)"";

		hist = CIRCLEQ_LOOP_NEXT(&cli->hd_hist, cli->curr_hist, next);
		cli->curr_hist = hist;

		return hist->line;
	}
	return NULL;
}

void
cli_history_clear(void)
{
	struct cli *cli = this_cli;
	struct cli_hist *h;

	if (!cli || !cli->hist_mem)
		return;

	while (!CIRCLEQ_EMPTY(&cli->hd_hist)) {
		h = CIRCLEQ_FIRST(&cli->hd_hist);
		CIRCLEQ_REMOVE(&cli->hd_hist, h, next);
		cli_hist_free(h);
	}
}

void
cli_history_delete(void)
{
	struct cli *cli = this_cli;

	if (cli) {
		cli_history_clear();

		CIRCLEQ_INIT(&cli->hd_hist);

		cli->hist_mem = NULL;
		cli->nb_hist = 0;
	}
}

int
cli_set_history(uint32_t nb_hist)
{
	struct cli *cli = this_cli;
	size_t size;

	if (!cli)
		return -1;

	if (nb_hist == 0) {
		cli_history_delete();
		return 0;
	}

	if (cli->hist_mem && (nb_hist == cli->nb_hist))
		return 0;

	if (cli->hist_mem)
		cli_history_delete();

	cli->nb_hist = nb_hist;

	size = nb_hist * sizeof(struct cli_hist);

	cli->hist_mem = malloc(size);
	if (cli->hist_mem) {
		uint32_t i;
		struct cli_hist *hist = cli->hist_mem;

		memset(cli->hist_mem, '\0', size);

		/* Setup the history support is number of lines given */
		for (i = 0; i < nb_hist; i++, hist++)
			CIRCLEQ_INSERT_TAIL(&cli->free_hist, hist, next);

		return 0;
	}

	return -1;
}

void
cli_history_reset(void)
{
	this_cli->curr_hist = CIRCLEQ_FIRST(&this_cli->hd_hist);
}

void
cli_history_dump(void)
{
	struct cli *cli = this_cli;
	struct cli_hist *h;
	int i = 0;

	if (!cli || !cli->hist_mem || CIRCLEQ_EMPTY(&cli->hd_hist))
		return;

	CIRCLEQ_FOREACH(h, &cli->hd_hist, next) {
		cli_printf("%4d: %s\n", i++, h->line);
	}
}
