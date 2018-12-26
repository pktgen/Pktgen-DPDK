/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */

#include "cli.h"
#include "cli_input.h"
#include "cli_scrn.h"
#include "cli_auto_complete.h"

static inline void
key_up_arr(void)
{
	struct gapbuf *gb = this_cli->gb;
	char *line;

	line = cli_history_prev();
	if (line) {
		gb_reset_buf(gb);
		gb_set_point(gb, gb_str_insert(gb, line, 0));
		cli_set_flag(DISPLAY_LINE | CLEAR_LINE);
	}
}

static inline void
key_down_arr(void)
{
	struct gapbuf *gb = this_cli->gb;
	char *line;

	line = cli_history_next();
	if (line) {
		gb_reset_buf(gb);
		gb_set_point(gb, gb_str_insert(gb, line, 0));
		cli_set_flag(DISPLAY_LINE | CLEAR_LINE);
	}
}

static inline void
key_right_arr(void)
{
	struct gapbuf *gb = this_cli->gb;

	if (!gb_eof(gb)) {
		scrn_write(vt100_right_arr, 0);
		gb_move_right(gb);
	}
}

static inline void
key_left_arr(void)
{
	struct gapbuf *gb = this_cli->gb;

	if (!gb_point_at_start(gb)) {
		scrn_write(vt100_left_arr, 0);
		gb_move_left(gb);
	}
}

static inline void
key_backspace(void)
{
	struct gapbuf *gb = this_cli->gb;

	if (!gb_point_at_start(gb)) {
		cli_cursor_left();

		gb_move_left(gb);

		gb_del(gb, 1);

		cli_set_flag(DELETE_CHAR);
	}
}

static inline void
key_return(void)
{
	cli_write("\n", 1);

	cli_execute();

	/* Init buffer must be after execute of command */
	gb_reset_buf(this_cli->gb);

	/* If not quit command then print prompt */
	if (!this_cli->quit_flag)
		this_cli->flags |= DISPLAY_PROMPT;
	this_cli->curr_hist = NULL;
}

static inline void
key_ctrl_a(void)
{
	cli_printf(vt100_multi_left, gb_left_data_size(this_cli->gb));
	gb_set_point(this_cli->gb, 0);
}

static inline void
key_ctrl_e(void)
{
	cli_printf(vt100_multi_right, gb_right_data_size(this_cli->gb));
	gb_set_point(this_cli->gb, -1);
}

static inline void
key_ctrl_k(void)
{
	struct cli *cli = this_cli;

	cli_clear_to_eol();
	gb_move_gap_to_point(cli->gb);
	free(cli->kill);
	if (gb_right_data_size(cli->gb))
		cli->kill = strndup(gb_end_of_gap(cli->gb),
		                    gb_right_data_size(cli->gb) + 1);
	gb_del(cli->gb, gb_right_data_size(cli->gb));
}

static inline void
key_ctrl_y(void)
{
	struct cli *cli = this_cli;

	/* Yank and put are Not supported yet */
	if (cli->kill) {
		gb_str_insert(cli->gb, cli->kill, strlen(cli->kill));
		cli_clear_line(-1);
		cli_redisplay_line();
	}
}

static inline void
key_ctrl_c(void)
{
	gb_reset_buf(this_cli->gb);
	cli_clear_line(-1);
	this_cli->plen = this_cli->prompt(0);
	this_cli->curr_hist = NULL;
}

static inline void
key_ctrl_f(void)
{
	key_right_arr();
}

static inline void
key_ctrl_b(void)
{
	key_left_arr();
}

// This is the 'super key' which has had many usages in the past. I changed it to
// but the back space key as it seems windows or mobaxterm sends this code to delete key.
static inline void
key_suppr(void)
{
	struct gapbuf *gb = this_cli->gb;

	if (!gb_point_at_start(gb)) {
		cli_cursor_left();

		gb_move_left(gb);

		gb_del(gb, 1);

		cli_set_flag(DELETE_CHAR);
	}
}

static inline void
key_tab(void)
{
	cli_auto_complete();
}

static inline void
key_ctrl_d(void)
{
	gb_dump(this_cli->gb, NULL);
}

static inline void
key_ctrl_l(void)
{
	cli_clear_screen();
	cli_clear_line(-1);
	cli_redisplay_line();
}

static inline void
key_return2(void)
{
	key_return();
}

static inline void
key_meta_backspace(void)
{
}

/* meta+b or command+b or window+b or super+b */
static inline void
key_word_left(void)
{
	do {
		key_left_arr();
		if (gb_get_prev(this_cli->gb) == ' ')
			break;
	} while (!gb_point_at_start(this_cli->gb));
}

static inline void
key_word_right(void)
{
	while (!gb_point_at_end(this_cli->gb)) {
		key_right_arr();
		if (gb_get(this_cli->gb) == ' ')
			break;
	}
}

static inline void
key_ctrl_w(void)
{
	key_meta_backspace();
}

static inline void
key_ctrl_p(void)
{
	key_up_arr();
}

static inline void
key_ctrl_n(void)
{
	key_down_arr();
}

static inline void
key_meta_d(void)
{
}

static inline void
key_ctrl_x(void)
{
	this_cli->quit_flag = 1;
}

static inline void
key_invalid(void)
{
}

/* Order must be maintained see cli_vt100.h */
static struct vt100_cmds vt100_cmd_list[] = {
	{ "Invalid",        key_invalid },
	{ vt100_up_arr,     key_up_arr },	/* Move cursor up one line */
	{ vt100_down_arr,   key_down_arr },	/* Move cursor down on line */
	{ vt100_right_arr,  key_right_arr },	/* Move cursor right */
	{ vt100_left_arr,   key_left_arr },	/* Move cursor left */
	{ "\177",           key_backspace },	/* Cursor Left + delete */
	{ "\n",             key_return },	/* Execute command */
	{ "\001",           key_ctrl_a },	/* Same as key_left_arr */
	{ "\005",           key_ctrl_e },	/* Same as key_right_arr */
	{ "\013",           key_ctrl_k },	/* Kill to end of line */
	{ "\031",           key_ctrl_y },	/* Put the kill buffer not working */
	{ "\003",           key_ctrl_c },	/* Reset line and start over */
	{ "\006",           key_ctrl_f },	/* Same as key_right_arr */
	{ "\002",           key_ctrl_b },	/* Same as key_left_arr */
	{ vt100_suppr,      key_suppr },	/* delete 1 char from the left */
	{ vt100_tab,        key_tab },		/* Auto complete */
	{ "\004",           key_ctrl_d },	/* Debug output if enabled */
	{ "\014",           key_ctrl_l },	/* redraw screen */
	{ "\r",             key_return2 },	/* Same as key_return */
	{ "\033\177",       key_meta_backspace },/* Delete word left */
	{ vt100_word_left,  key_word_left },	/* Word left */
	{ vt100_word_right, key_word_right },	/* Word right */
	{ "\027",           key_ctrl_w },	/* Same as key_meta_backspace */
	{ "\020",           key_ctrl_p },	/* Same as key_up_arr */
	{ "\016",           key_ctrl_n },	/* Same as key_down_arr */
	{ "\033\144",       key_meta_d },	/* Delete word right */
	{ "\030",           key_ctrl_x },	/* Terminate application */
	{ NULL, NULL }
};

void
vt100_do_cmd(int idx)
{
	if (idx < VT100_MAX_KEYS)
		vt100_cmd_list[idx].func();
}

struct vt100_cmds *
vt100_get_cmds(void)
{
	return vt100_cmd_list;
}

static int
vt100_find_cmd(char *buf, unsigned int size)
{
	struct vt100_cmds *cmd;
	size_t cmdlen;
	int i;

	for (i = 0, cmd = vt100_get_cmds(); cmd->str; cmd++, i++) {
		cmdlen = strnlen(cmd->str, VT100_BUF_SIZE);
		if ((size == cmdlen) && !strncmp(buf, cmd->str, cmdlen))
			return i;
	}

	return VT100_DONE;
}

int
vt100_parse_input(struct cli_vt100 *vt, uint8_t c)
{
	uint32_t size;

	RTE_ASSERT(vt != NULL);

	if ((vt->bufpos == VT100_INITIALIZE) ||
	    (vt->bufpos >= VT100_BUF_SIZE)) {
		vt->state = VT100_INIT;
		vt->bufpos = 0;
	}

	vt->buf[vt->bufpos++] = c;
	size = vt->bufpos;

	switch (vt->state) {
	case VT100_INIT:
		if (c == vt100_escape)
			vt->state = VT100_ESCAPE;
		else {
			vt->bufpos = VT100_INITIALIZE;
			return vt100_find_cmd(vt->buf, size);
		}
		break;

	case VT100_ESCAPE:
		if (c == vt100_open_square)
			vt->state = VT100_ESCAPE_CSI;
		else if (c >= '0' && c <= vt100_del) {
			vt->bufpos = VT100_INITIALIZE;
			return vt100_find_cmd(vt->buf, size);
		}
		break;

	case VT100_ESCAPE_CSI:
		if (c >= '@' && c <= '~') {
			vt->bufpos = VT100_INITIALIZE;
			return vt100_find_cmd(vt->buf, size);
		}
		break;

	default:
		vt->bufpos = VT100_INITIALIZE;
		break;
	}

	return VT100_CONTINUE;
}

struct cli_vt100 *
vt100_setup(void)
{
	struct cli_vt100 *vt;

	vt = calloc(1, sizeof(struct cli_vt100));
	if (!vt)
		return NULL;

	vt->bufpos = -1;

	return vt;
}

void
vt100_free(struct cli_vt100 *vt)
{
	if (vt)
		free(vt);
}
