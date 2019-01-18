/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */

#include <stdio.h>

#include <rte_version.h>
#include <rte_cycles.h>
#include <rte_timer.h>
#include <rte_devargs.h>
#include <rte_pci.h>
#include <rte_debug.h>
#include <rte_log.h>
#include <rte_string_fns.h>
#include <rte_strings.h>

#include "cli.h"
#include "cli_input.h"
#include "cli_cmds.h"
#include "cli_cmap.h"
#include "cli_map.h"
#include "cli_file.h"
#include "cli_help.h"

static int
__print_help(struct cli_node *node, char *search)
{
	struct cli_node *cmd;

	if (!node)
		node = get_cwd();
	else if (!is_directory(node))
		return -1;

	TAILQ_FOREACH(cmd, &node->items, next) {
		if (is_executable(cmd)) {
			if (search) {
				if (strcmp(cmd->name, search) == 0) {
					cli_printf("  %-16s %s\n", cmd->name,
						cmd->short_desc);
					return 1;
				}
			} else
				cli_printf("  %-16s %s\n", cmd->name,
					cmd->short_desc);
		}
	}
	return 0;
}

static int
chelp_cmd(int argc, char **argv)
{
	struct cli *cli = this_cli;
	struct cli_node *bin;
	char *search = NULL;
	int i, opt, all = 0;

	optind = 0;
	while ((opt = getopt(argc, argv, "a?")) != -1) {
		switch (opt) {
		case '?': cli_usage(); return 0;
		case 'a':   all = 1; break;
		default:
			break;
		}
	}
	if (optind < argc)
		search = argv[optind];

	cli_printf("*** CLI Help ***\n");
	cli_printf("  Use <command> -? to show usage for a command\n");
	cli_printf("  Use !<NN> to execute a history line\n");
	cli_printf("  Use @<host command> to execute a host binary\n");
	cli_printf("  Use Up/Down arrows to access history commands\n\n");
	cli_printf("  Use 'chelp -a' to list all commands\n");

	if (all == 0) {
		/* Look in the current directory first for a command */
		cli_printf("*** Current directory commands ***\n");

		return __print_help(NULL, search);
	}

	cli_printf("*** All executable commands in path ***\n");

	/* Did not find a command in local then look in the bin dirs */
	for (i = 0; i < CLI_MAX_BINS; i++) {
		bin = cli->bins[i];
		if (bin == NULL)
			continue;

		cli_printf("%s:\n", bin->name);

		if (__print_help(bin, search))
			return 0;
	}

	return 0;
}

static int
cd_cmd(int argc, char **argv)
{
	struct cli_node *node;

	if (argc > 1) {
		if (!strcmp(argv[1], "-?")) {
			cli_usage();
			return 0;
		}

		if (!cli_find_node(argv[1], &node)) {
			cli_printf("** Invalid directory: %s\n", argv[1]);
			return -1;
		}
		set_cwd(node);
	}

	return 0;
}

static int
pwd_cmd(int argc, char **argv)
{
	char *str = cli_cwd_path();

	if (argc > 1 && !strcmp(argv[1], "-?")) {
		cli_usage();
		return 0;
	}

	/* trim off the trailing '/' if needed */
	if (strlen(str) > 1)
		str[strlen(str) - 1] = '\0';

	cli_printf("%s\n", str);
	return 0;
}

static int
__list_long_dir(struct cli_node *node,
		uint32_t type __rte_unused, args_t *args)
{
	uint16_t flags = args->arg1.u16[3];
	uint16_t spc = args->arg2.u16[0];

	if (is_alias(node))
		cli_printf("  %*s%-16s %s : %s\n", spc, "",
			   node->name, cli_node_type(node), node->alias_str);
	else if (is_command(node))
		cli_printf("  %*s%-16s %s : %s\n", spc, "",
			   node->name, cli_node_type(node), node->short_desc);
	else
		cli_printf("  %*s%-16s %s\n", spc, "",
			   node->name, cli_node_type(node));

	if ((flags & CLI_RECURSE_FLAG) && is_directory(node)) {
		args->arg2.u16[0] += 2;
		cli_scan_directory(node, __list_long_dir, type, args);
		args->arg2.u16[0] = spc;
	}

	return 0;
}

static int
__list_dir(struct cli_node *node, uint32_t flag __rte_unused, args_t *args)

{
	char buf[CLI_NAME_LEN + 1];
	uint16_t cnt = args->arg1.u16[0];
	uint16_t mlen = args->arg1.u16[1];
	uint16_t col = args->arg1.u16[2];
	uint16_t flags = args->arg1.u16[3];

	if (!node)
		return -1;

	if (is_directory(node)) {
		char dbuf[CLI_NAME_LEN + 1];
		snprintf(dbuf, sizeof(dbuf), "[%s]", node->name);
		snprintf(buf, sizeof(buf), "%-*s", mlen, dbuf);
	} else
		snprintf(buf, sizeof(buf), "%-*s", mlen, node->name);

	cli_printf("%s", buf);
	if ((++cnt % col) == 0)
		cli_printf("\n");

	if ((flags & CLI_RECURSE_FLAG) && is_directory(node)) {
		cli_printf("\n");
		args->arg1.u16[0] = 0;
		cli_scan_directory(node, __list_dir, CLI_ALL_TYPE, args);
		args->arg1.u16[0] = cnt;
		cli_printf("\n");
	}

	args->arg1.u16[0] = cnt;
	return 0;
}

static int
ls_cmd(int argc, char **argv)
{
	struct cli_node *node = get_cwd();
	args_t args;
	uint32_t flags = 0;
	int opt;

	optind = 0;
	while ((opt = getopt(argc, argv, "?rl")) != -1) {
		switch (opt) {
		case '?':   cli_usage(); return 0;
		case 'r':   flags |= CLI_RECURSE_FLAG; break;
		case 'l':   flags |= CLI_LONG_LIST_FLAG; break;
		default:
			break;
		}
	}

	if (optind < argc)
		if (cli_find_node(argv[optind], &node) == 0) {
			cli_printf("Invalid directory (%s)!!\n", argv[optind]);
			return -1;
		}

	memset(&args, 0, sizeof(args));

	args.arg1.u16[0] = 0;
	args.arg1.u16[1] = 16;
	args.arg1.u16[2] = 80 / 16;
	args.arg1.u16[3] = flags;
	args.arg2.u16[0] = 0;

	if (flags & CLI_LONG_LIST_FLAG)
		cli_scan_directory(node, __list_long_dir, CLI_ALL_TYPE, &args);
	else
		cli_scan_directory(node, __list_dir, CLI_ALL_TYPE, &args);

	cli_printf("\n");
	return 0;
}

static int
scrn_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	cli_clear_screen();
	return 0;
}

static int
quit_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	cli_quit();
	return 0;
}

static int
hist_cmd(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "-?"))
		cli_usage();
	else
		cli_history_list();
	return 0;
}

static int
more_cmd(int argc, char **argv)
{
	struct cli_node *node;
	char *buf, c;
	int i, len, n, k, lines = 24;
	int opt;

	optind = 0;
	while ((opt = getopt(argc, argv, "?n:")) != -1) {
		switch (opt) {
		case '?':   cli_usage(); return 0;
		case 'n':   lines = atoi(optarg); break;
		default:
			break;
		}
	}

	if (optind >= argc)
		return 0;

	len = 256;
	buf = alloca(len + 1);
	if (!buf)
		return -1;

	for (i = optind; i < argc; i++) {
		k = 0;
		node = cli_file_open(argv[i], "r");
		if (!node) {
			cli_printf("** (%s) is not a file\n", argv[i]);
			continue;
		}
		do {
			n = cli_readline(node, buf, len);
			if (n > 0)
				cli_printf("%s", buf);	/* contains a newline */
			if (++k >= lines) {
				k = 0;
				c = cli_pause("More", NULL);
				if ((c == vt100_escape) ||
				    (c == 'q') || (c == 'Q'))
					break;
			}
		} while (n > 0);
		cli_file_close(node);
	}

	cli_printf("\n");

	return 0;
}

/* Helper for building log strings.
 * The macro takes an existing string, a printf-like format string and optional
 * arguments. It formats the string and appends it to the existing string, while
 * avoiding possible buffer overruns.
 */
#define strncatf(dest, fmt, ...) do {					\
		char _buff[1024];					\
		snprintf(_buff, sizeof(_buff), fmt, ## __VA_ARGS__);	\
		strncat(dest, _buff, sizeof(dest) - strlen(dest) - 1);	\
} while (0)

static __inline__ uint8_t
sct(struct cmap *cm, uint8_t s, uint8_t c, uint8_t t) {
	lc_info_t   *lc = cm->linfo;
	uint8_t i;

	for (i = 0; i < cm->num_cores; i++, lc++)
		if (lc->sid == s && lc->cid == c && lc->tid == t)
			return lc->lid;

	return 0;
}

static int
core_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	struct cmap *c;
	int i;

	c = cmap_create();

	cli_printf("CPU : %s", c->model);
	cli_printf("      %d lcores, %u socket%s, %u core%s per socket and "
		   "%u thread%s per core\n",
		   c->num_cores,
		   c->sid_cnt, c->sid_cnt > 1 ? "s" : "",
		   c->cid_cnt, c->cid_cnt > 1 ? "s" : "",
		   c->tid_cnt, c->tid_cnt > 1 ? "s" : "");

	cli_printf("Socket     : ");
	for (i = 0; i < c->sid_cnt; i++)
		cli_printf("%4d      ", i);
	cli_printf("\n");

	for (i = 0; i < c->cid_cnt; i++) {
		cli_printf("  Core %3d : [%2d,%2d]   ", i,
			   sct(c, 0, i, 0), sct(c, 0, i, 1));
		if (c->sid_cnt > 1)
			cli_printf("[%2d,%2d]   ",
				   sct(c, 1, i, 0), sct(c, 1, i, 1));
		if (c->sid_cnt > 2)
			cli_printf("[%2d,%2d]   ",
				   sct(c, 2, i, 0), sct(c, 2, i, 1));
		if (c->sid_cnt > 3)
			cli_printf("[%2d,%2d]   ",
				   sct(c, 3, i, 0), sct(c, 3, i, 1));
		cli_printf("\n");
	}

	cmap_free(c);

	return 0;
}

static int
huge_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	if (system("cat /proc/meminfo | grep -i huge"))
		return -1;
	return 0;
}

#ifdef CLI_DEBUG_CMDS
static int
sizes_cmd(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "-?")) {
		cli_usage();
		return 0;
	}

	cli_printf("  sizeof(struct cli)      %zu\n", sizeof(struct cli));
	cli_printf("  sizeof(struct cli_node) %zu\n", sizeof(struct cli_node));
	cli_printf("  sizeof(args_t)          %zu\n", sizeof(args_t));
	cli_printf("  Total number of Nodes   %d\n", this_cli->nb_nodes);
	cli_printf("  Number History lines    %d\n", this_cli->nb_hist);
	cli_printf("  CLI_DEFAULT_NB_NODES    %d\n", CLI_DEFAULT_NB_NODES);
	cli_printf("  CLI_DEFAULT_HIST_LINES  %d\n", CLI_DEFAULT_HIST_LINES);
	cli_printf("  CLI_MAX_SCRATCH_LENGTH  %d\n", CLI_MAX_SCRATCH_LENGTH);
	cli_printf("  CLI_MAX_PATH_LENGTH     %d\n", CLI_MAX_PATH_LENGTH);
	cli_printf("  CLI_NAME_LEN            %d\n", CLI_NAME_LEN);
	cli_printf("  CLI_MAX_ARGVS           %d\n", CLI_MAX_ARGVS);
	cli_printf("  CLI_MAX_BINS            %d\n", CLI_MAX_BINS);

	return 0;
}
#endif

static int
path_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	int i;
	char *str;

	cli_printf("  Path = .:");
	for (i = 1; i < CLI_MAX_BINS; i++) {
		if (this_cli->bins[i] == NULL)
			continue;
		str = cli_path_string(this_cli->bins[i], NULL);

		/* trim off the trailing '/' if needed */
		if (strlen(str) > 1)
			str[strlen(str) - 1] = '\0';

		cli_printf("%s:", str);
	}
	cli_printf("\n");

	return 0;
}

static const char *copyright =
"   BSD LICENSE\n"
"\n"
"   Copyright(c) 2010-2017 Intel Corporation. All rights reserved.\n"
"\n"
"   Redistribution and use in source and binary forms, with or without\n"
"   modification, are permitted provided that the following conditions\n"
"   are met:\n"
"\n"
"     * Redistributions of source code must retain the above copyright\n"
"       notice, this list of conditions and the following disclaimer.\n"
"     * Redistributions in binary form must reproduce the above copyright\n"
"       notice, this list of conditions and the following disclaimer in\n"
"       the documentation and/or other materials provided with the\n"
"       distribution.\n"
"     * Neither the name of Intel Corporation nor the names of its\n"
"       contributors may be used to endorse or promote products derived\n"
"       from this software without specific prior written permission.\n"
"\n"
"   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
"   \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
"   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
"   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
"   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
"   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n"
"   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
"   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
"   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
"   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
"   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
"\n"
"   SPDX-License-Identifier: BSD-3-Clause\n";

static int
copyright_file(struct cli_node *node, char *buff, int len, uint32_t flags)
{

	if (is_file_open(flags)) {
		node->file_data = (char *)(uintptr_t)copyright;
		node->file_size = strlen(copyright);
		node->fflags = CLI_DATA_RDONLY;
		if (is_file_eq(flags, (CLI_FILE_APPEND | CLI_FILE_WR)))
			node->foffset = node->file_size;
		return 0;
	}
	return cli_file_handler(node, buff, len, flags);
}

static int
version_file(struct cli_node *node, char *buff, int len, uint32_t flags)
{
	const char *data = rte_version();

	if (is_file_open(flags)) {
		node->file_data = (char *)(uintptr_t)data;
		node->file_size = strlen(data);
		node->fflags = CLI_DATA_RDONLY;
		if (is_file_eq(flags, (CLI_FILE_APPEND | CLI_FILE_WR)))
			node->foffset = node->file_size;
		return 0;
	}
	return cli_file_handler(node, buff, len, flags);
}

static int
sleep_cmd(int argc __rte_unused, char **argv)
{
	uint32_t cnt = (atoi(argv[1]) * 4);

	if (rte_get_timer_hz() == 0) {
		cli_printf("rte_get_timer_hz() returned zero\n");
		return 0;
	}

	while (cnt--) {
		cli_use_timers();
		rte_delay_us_sleep(250 * 1000);
	}
	return 0;
}

static int
delay_cmd(int argc __rte_unused, char **argv)
{
	int ms = atoi(argv[1]);
	int cnt = (ms / 1000) * 4;

	while (cnt--) {
		cli_use_timers();
		rte_delay_us_sleep(250 * 1000);
		ms -= 250;
	}
	if (ms > 0)
		rte_delay_us_sleep(ms * 1000);
	return 0;
}

static int
mkdir_cmd(int argc, char **argv)
{
	if (argc != 2) {
		cli_printf("Must have at least one path/driectory\n");
		return -1;
	}

	if (!cli_add_dir(argv[1], get_cwd()))
		return -1;
	return 0;
}

static int
rm_cmd(int argc, char **argv)
{
	struct cli_node *node;

	if (argc != 2) {
		cli_printf("usage: rm [dir|file|command]\n");
		return -1;
	}

	if (!cli_find_node(argv[1], &node)) {
		cli_printf("Unable to find: %s\n", argv[1]);
		return -1;
	}

	return cli_remove_node(node);
}

static char *
ver_cmd(const char *val __rte_unused)
{
	return (char *)(uintptr_t)rte_version();
}

static struct cli_map cli_env_map[] = {
	{ 10, "env" },
	{ 20, "env get %s" },
	{ 30, "env set %s %s" },
	{ 40, "env del %s" },
	{ -1, NULL }
};

static const char *cli_env_help[] = {
	"env                       - Display current evironment variables",
	"env get <string>          - Get the requested variable",
	"env set <string> <string> - Set the given variable to string",
	"env del <string>          - Delete the given variable",
	NULL
};

static int
env_cmd(int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(cli_env_map, argc, argv);
	if (!m)
		cli_cmd_error("Environment command error:", "Env", argc, argv);
	switch (m->index) {
	case 10: cli_env_show(this_cli->env); break;
	case 20:
		cli_printf("  \"%s\" = \"%s\"\n", argv[2],
			cli_env_get(this_cli->env, argv[2]));
		break;
	case 30: cli_env_set(this_cli->env, argv[2], argv[3]); break;
	case 40: cli_env_del(this_cli->env, argv[2]); break;
	default:
		cli_help_show_group("env");
		return -1;
	}
	return 0;
}

static int
script_cmd(int argc, char **argv)
{
	int i;

	if (argc <= 1)
		return -1;

	for(i = 1; i < argc; i++)
		if (cli_execute_cmdfile(argv[1]))
			return -1;
	return 0;
}

static int
echo_cmd(int argc, char **argv)
{
	int i;

	for(i = 1; i < argc; i++)
		cli_printf("%s ",argv[i]);
	cli_printf("\n");
	return 0;
}

static int
version_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	cli_printf("Version: %s\n", rte_version());
	return 0;
}

static struct cli_tree cli_default_tree[] = {
c_file("copyright", copyright_file, "DPDK copyright information"),
c_file("dpdk-version",   version_file,   "DPDK version"),
c_bin("/sbin"),

c_cmd("delay",      delay_cmd,  "delay a number of milliseconds"),
c_cmd("sleep",      sleep_cmd,  "delay a number of seconds"),
c_cmd("chelp",      chelp_cmd,  "CLI help - display information for DPDK"),
c_cmd("mkdir",      mkdir_cmd,  "create a directory"),
c_cmd("rm",         rm_cmd,     "remove a file or directory"),
c_cmd("ls",         ls_cmd,     "ls [-lr] <dir> # list current directory"),
c_cmd("cd",         cd_cmd,     "cd <dir> # change working directory"),
c_cmd("pwd",        pwd_cmd,    "pwd # display current working directory"),
c_cmd("screen.clear", scrn_cmd, "screen.clear # clear the screen"),
c_cmd("quit",       quit_cmd,   "quit # quit the application"),
c_cmd("history",    hist_cmd,   "history # display the current history"),
c_cmd("more",       more_cmd,   "more <file> # display a file content"),
#ifdef CLI_DEBUG_CMDS
c_cmd("sizes",      sizes_cmd,  "sizes # display some internal sizes"),
#endif
c_cmd("cmap",       core_cmd,   "cmap # display the core mapping"),
c_cmd("hugepages",  huge_cmd,   "hugepages # display hugepage info"),
c_cmd("path",       path_cmd,   "display the execution path for commands"),
c_cmd("env",        env_cmd,    "Show/del/get/set environment variables"),
c_cmd("script",     script_cmd,	"load and process cli command files"),
c_cmd("echo",       echo_cmd,   "simple echo a string to the screen"),
c_cmd("version",    version_cmd,"Display version information"),

/* The following are environment variables */
c_str("SHELL",      NULL,       "CLI shell"),
c_str("DPDK_VER",   ver_cmd,	""),
c_end()
};

int
cli_default_tree_init(void)
{
	int ret = 0;

	if (this_cli->flags & CLI_DEFAULT_TREE)
		return ret;

	this_cli->flags |= CLI_DEFAULT_TREE;

	/* Add the list of commands/dirs in cli_cmds.c file */
	if ((ret = cli_add_tree(NULL, cli_default_tree)) == 0)
		cli_help_add("Env", cli_env_map, cli_env_help);

	if (ret) {
		RTE_LOG(ERR, EAL, "Unable to add commands or directoies\n");
		this_cli->flags &= ~CLI_DEFAULT_TREE;
	}

	return ret;
}
