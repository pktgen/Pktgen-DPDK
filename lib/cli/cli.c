/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */
/* Created by Keith Wiles @ intel.com */

#include <stdio.h>
#include <poll.h>
#include <string.h>

#include <rte_version.h>
#include <rte_timer.h>
#include <rte_log.h>
#include <rte_string_fns.h>
#include <rte_strings.h>
#include <rte_pause.h>

#include "cli.h"
#include "cli_input.h"

static int (*__dofile_lua)(void *, const char *);

RTE_DEFINE_PER_LCORE(struct cli *, cli);

void
cli_use_timers(void)
{
	if (this_cli && this_cli->flags & CLI_USE_TIMERS)
		rte_timer_manage();
}

int
cli_nodes_unlimited(void)
{
	if (!this_cli)
		return 0;
	return this_cli->flags & CLI_NODES_UNLIMITED;
}

/* Allocate a node from the CLI node pool */
static inline struct cli_node *
cli_alloc(void)
{
	struct cli *cli = this_cli;
	struct cli_node *node;

	node = (struct cli_node *)TAILQ_FIRST(&cli->free_nodes);
	if (node)
		TAILQ_REMOVE(&cli->free_nodes, node, next);
	else if (cli_nodes_unlimited()) {
		struct cli_node *orig = cli->node_mem;
		size_t size;

		size = (cli->nb_nodes + CLI_DEFAULT_NB_NODES) *
		       sizeof(struct cli_node);
		cli->node_mem = realloc(cli->node_mem, size);
		if (cli->node_mem == NULL) {
			cli->node_mem = orig;
			return NULL;
		}
		if (cli->node_mem == orig) {
			uint32_t i;

			node = &cli->node_mem[cli->nb_nodes];
			for (i = 0; i < CLI_DEFAULT_NB_NODES; i++, node++)
				TAILQ_INSERT_TAIL(&cli->free_nodes, node, next);
			cli->nb_nodes += CLI_DEFAULT_NB_NODES;
		} else
			return NULL;
	}

	return node;
}

/* Free a node back to the CLI mempool */
static inline void
cli_free(struct cli_node *node)
{
	TAILQ_INSERT_TAIL(&this_cli->free_nodes, node, next);
}

/* Add a directory to the executable path */
int
cli_add_bin(struct cli_node *dir)
{
	struct cli *cli = this_cli;
	int i;

	if (!dir || !is_directory(dir))
		return -1;

	for (i = 1; i < CLI_MAX_BINS; i++)
		if (cli->bins[i] == dir) {
			RTE_LOG(WARNING, EAL,
				"Adding duplicate bin directory (%s)\n",
				dir->name);
			return 0;
		}

	/* Skip first special entry for current working directory */
	for (i = 1; i < CLI_MAX_BINS; i++)
		if (cli->bins[i] == NULL) {
			cli->bins[i] = dir;
			return 0;
		}
	return -1;
}

/* Remove a directory from the executable path */
int
cli_del_bin(struct cli_node *dir)
{
	struct cli *cli = this_cli;
	int i;

	if (!dir || !is_directory(dir))
		return -1;

	for (i = 0; i < CLI_MAX_BINS; i++)
		if (cli->bins[i] == dir) {
			cli->bins[i] = NULL;

			/* compress the list of directories */
			if ((i + 1) < CLI_MAX_BINS) {
				memmove(&cli->bins[i], &cli->bins[i + 1],
				        (CLI_MAX_BINS - (i + 1)) *
				        sizeof(void *));
				cli->bins[CLI_MAX_BINS - 1] = NULL;
			}
			return 0;
		}
	return -1;
}

/* Add a directory to the executable path using the path string */
int
cli_add_bin_path(const char *path)
{
	struct cli_node *node;

	if (cli_find_node(path, &node)) {
		if (cli_add_bin(node))
			return -1;
	} else
		return -1;

	return 0;
}

/* Helper routine to remove nodes to the CLI tree */
int
cli_remove_node(struct cli_node *node)
{
	struct cli_node *parent, *n;

	if (!node)
		return 0;

	parent = node->parent;
	if (!parent)	/* Can not remove '/' or root */
		return -1;

	switch (node->type) {
	case CLI_DIR_NODE:
		if (!TAILQ_EMPTY(&node->items))
			while (!TAILQ_EMPTY(&node->items)) {
				n = TAILQ_FIRST(&node->items);
				if (cli_remove_node(n))
					return -1;
			}
		break;
	case CLI_CMD_NODE:
	case CLI_FILE_NODE:
	case CLI_ALIAS_NODE:
	case CLI_STR_NODE:
		break;
	default:
		return -1;
	}

	if (is_directory(node))
		cli_del_bin(node);

	TAILQ_REMOVE(&parent->items, node, next);
	cli_free(node);

	return 0;
}

/* Helper routine to add nodes to the CLI tree */
static struct cli_node *
__add_node(const char *name, struct cli_node *parent,
	int type, cli_funcs_t func, const char *short_desc)
{
	struct cli_node *node;

	if (!name)
		return NULL;

	switch (type) {
	case CLI_DIR_NODE:
		if (parent && (strcmp(name, CLI_ROOT_NAME) == 0))
			return NULL;
		if (!parent && strcmp(name, CLI_ROOT_NAME))
			return NULL;
		if (func.cfunc)
			return NULL;
		break;
	case CLI_CMD_NODE:
		if (!parent || !func.cfunc)
			return NULL;
		break;
	case CLI_FILE_NODE:
		if (!parent || !func.ffunc)
			return NULL;
		break;
	case CLI_ALIAS_NODE:
		if (!parent || func.cfunc)
			return NULL;
		break;
	case CLI_STR_NODE:
		if (!func.sfunc && !short_desc)
			return NULL;
		break;
	default:
		return NULL;
	}

	node = cli_alloc();
	if (node == NULL) {
		cli_printf("%s: No nodes left\n", __func__);
		return NULL;
	}

	node->type = type;
	node->parent = parent;

	switch (type) {
	case CLI_CMD_NODE:
	case CLI_ALIAS_NODE:
		node->cfunc = func.cfunc;
		break;
	case CLI_FILE_NODE:
		node->ffunc = func.ffunc;
		break;
	case CLI_DIR_NODE:
	case CLI_STR_NODE:
		break;
	default:
		break;
	}
	node->short_desc = short_desc;
	snprintf(node->name, sizeof(node->name), "%s", name);
	node->name_sz = strlen(node->name);

	if (parent)
		TAILQ_INSERT_HEAD(&parent->items, node, next);

	return node;
}

/* Add a direcrtory to the CLI tree */
struct cli_node *
cli_add_dir(const char *name, struct cli_node *dir)
{
	struct cli *cli = this_cli;
	char *argv[CLI_MAX_ARGVS], *p;
	char path[CLI_MAX_PATH_LENGTH];
	int cnt, i;
	struct cli_node *n, *ret;
	cli_funcs_t funcs;

	RTE_ASSERT(cli != NULL);
	if (!name)
		return NULL;

	/* return the last node if directory path already exists */
	if (cli_find_node((char *)(uintptr_t)name, &ret))
		return ret;

	/* Set the function structure to NULL */
	funcs.cfunc = NULL;

	p = cli->scratch;
	if (!dir)	/* Passed in a NULL to start at root node */
		dir = cli->root.tqh_first;

	memset(path, '\0', sizeof(path));

	p = cli->scratch;

	/* Grab a local copy of the directory path */
	snprintf(p, CLI_MAX_SCRATCH_LENGTH, "%s", name);

	if (p[0] == '/') {	/* Start from root */
		dir = cli->root.tqh_first;
		p++;		/* Skip the / in the orignal path */
		path[0] = '/';	/* Add root to the path */
	}

	cnt = rte_strtok(p, "/", argv, CLI_MAX_ARGVS);

	n = NULL;
	for (i = 0; i < cnt; i++) {
		/* Append each directory part to the search path */
		strcat(path, argv[i]);

		if (cli_find_node(path, &ret)) {
			dir = ret;
			continue;
		}

		n = __add_node(argv[i], dir, CLI_DIR_NODE, funcs, NULL);
		if (n == NULL)
			break;
		dir = n;
	}
	return n;
}

/* Add a command executable to the CLI tree */
struct cli_node *
cli_add_cmd(const char * name,
            struct cli_node *dir, cli_cfunc_t func,
            const char *short_desc)
{
	cli_funcs_t funcs;

	funcs.cfunc = func;
	return __add_node(name, dir, CLI_CMD_NODE, funcs, short_desc);
}

/* Add a command alias executable to the CLI tree */
struct cli_node *
cli_add_alias(const char *name,
              struct cli_node *dir,
              const char *line, const char *short_desc)
{
	struct cli_node *alias;
	cli_funcs_t funcs;

	funcs.cfunc = NULL;
	alias = __add_node(name, dir, CLI_ALIAS_NODE, funcs, short_desc);
	alias->alias_str = (const char *)strdup(line);

	return alias;
}

/* Add a file to the CLI tree */
struct cli_node *
cli_add_file(const char * name,
             struct cli_node *dir, cli_ffunc_t func,
             const char *short_desc)
{
	cli_funcs_t funcs;

	funcs.ffunc = func;
	return __add_node(name, dir, CLI_FILE_NODE, funcs, short_desc);
}

/* Add a string to the CLI tree */
int
cli_add_str(const char * name, cli_sfunc_t func, const char *str)
{
	return cli_env_string(this_cli->env, name, func, str);
}

/* Add a directory/commands/files/... to a directory */
int
cli_add_tree(struct cli_node *parent, struct cli_tree *tree)
{
	struct cli *cli = this_cli;
	struct cli_tree *t;
	struct cli_dir *d;
	struct cli_cmd *c;
	struct cli_file *f;
	struct cli_alias *a;
	struct cli_str *s;
	struct cli_node *n;

	if (!tree)
		return -1;

	if (!parent)
		parent = cli->root.tqh_first;

	for (t = tree; t->type != CLI_UNK_NODE; t++) {
		switch (t->type) {
		case CLI_DIR_NODE:
			d = &t->dir;

			if (!(n = cli_add_dir(d->name, parent))) {
				RTE_LOG(ERR, EAL,
				        "Add directory %s failed\n", d->name);
				return -1;
			}
			if (d->bin)
				cli_add_bin_path(d->name);

			parent = n;
			break;

		case CLI_CMD_NODE:
			c = &t->cmd;
			if (!cli_add_cmd(c->name, parent,
			                 c->cfunc, c->short_desc)) {
				RTE_LOG(ERR, EAL,
				        "Add command %s failed\n", c->name);
				return -1;
			}
			break;

		case CLI_FILE_NODE:
			f = &t->file;
			if (!cli_add_file(f->name, parent,
			                  f->ffunc, f->short_desc)) {
				RTE_LOG(ERR, EAL,
				        "Add file %s failed\n", f->name);
				return -1;
			}
			break;

		case CLI_ALIAS_NODE:
			a = &t->alias;
			if (!cli_add_alias(a->name, parent,
			                   a->alias_atr, a->short_desc)) {
				RTE_LOG(ERR, EAL,
				        "Add alias %s failed\n", a->name);
				return -1;
			}
			break;

		case CLI_STR_NODE:
			s = &t->str;
			if (cli_add_str(s->name, s->sfunc, s->string)) {
				RTE_LOG(ERR, EAL,
				        "Add string %s failed\n", s->name);
				return -1;
			}
			break;

		case CLI_UNK_NODE:
		default:
			RTE_LOG(ERR, EAL, "Unknown Node type %d\n", t->type);
			return 0;
		}
	}

	return 0;
}

/* execute a command or alias node in the CLI tree */
int
cli_execute(void)
{
	struct cli *cli = this_cli;
	struct cli_node *node;
	int argc, ret, sz;
	struct gapbuf *gb = cli->gb;
	char *line, *p, *hist;

	RTE_ASSERT(cli != NULL);

	sz = gb_data_size(gb);
	sz = RTE_MAX(sz, CLI_MAX_PATH_LENGTH);

	line = alloca(sz + 1);
	if (!line)
		return -1;

	memset(line, '\0', sz + 1);

	/* gb_copy_to_buf() forces linebuf to be null terminated */
	gb_copy_to_buf(gb, line, sz);

	/* Trim the string of whitspace on front and back */
	p = rte_strtrim(line);
	if (!strlen(p))
		return 0;

	if (p[0] == '#')	/* Found a comment line starting with a '#' */
		return 0;
	else if (p[0] == '!') {	/* History command */
		hist = cli_history_line(atoi(&p[1]));
		if (!hist) {
			cli_printf("Unknown history line number %d\n", atoi(&p[1]));
			return 0;
		}
		/* History lines are already trimmed and ready to be executed */
		strcpy(line, hist);
#ifdef RTE_CLI_HOST_COMMANDS
	} else if (p[0] == '@') {	/* System execute a command */
		ret = cli_system(&p[1]);
		if (!ret)
			cli_history_add(p);
		return ret;
#endif
	} else
		cli_history_add(p);

	/* Process the line for environment variable substitution */
	cli_env_substitution(cli->env, p, sz - (p - line));

	argc = rte_strqtok(p, " \r\n", cli->argv, CLI_MAX_ARGVS);

	if (!argc)
		return 0;

	node = cli_find_cmd(cli->argv[0]);
	if (!node) {
		cli_printf("** command not found (%s)\n", cli->argv[0]);
		return -1;
	}

	ret = -1;
	switch (node->type) {
	case CLI_CMD_NODE:
		cli->exe_node = node;
		ret = node->cfunc(argc, cli->argv);
		cli->exe_node = NULL;
		break;

	case CLI_ALIAS_NODE:
		/* Delete the alias history line just added */
		cli_history_del();

		cli->scratch[0] = '\0';		/* Reset scratch to empty */

		/* If there is more data after command name save it */
		if (gb_data_size(gb) > node->name_sz)
			gb_copy_to_buf(cli->gb, cli->scratch, gb_data_size(gb));

		sz = strlen(cli->scratch);

		gb_reset_buf(gb);

		gb_str_insert(gb, (char *)(uintptr_t)node->alias_str,
		              strlen(node->alias_str));

		/* Add the extra line arguments */
		sz = sz - node->name_sz;
		if (sz > 0)
			gb_str_insert(gb, &cli->scratch[node->name_sz], sz);
		ret = cli_execute();
		break;

	case CLI_DIR_NODE:
		cli_printf("** (%s) is a directory\n", cli->argv[0]);
		break;

	case CLI_FILE_NODE:
		cli_printf("** (%s) is a file\n", cli->argv[0]);
		break;

	case CLI_STR_NODE:
		cli_printf("** (%s) is a string\n", cli->argv[0]);
		break;

	case CLI_UNK_NODE:
	default:
		cli_printf("** unknown type (%s)\n", cli->argv[0]);
		break;
	}
	cli_history_reset();
	return ret;
}

/* Main entry point into the CLI system to start accepting user input */
void
cli_start(const char *msg)
{
	char c;

	RTE_ASSERT(this_cli != NULL);

	cli_printf("\n** Version: %s, %s with%s timers\n", rte_version(),
	           (msg == NULL) ? "Command Line Interface" : msg,
	           (this_cli->flags & CLI_USE_TIMERS) ? "" : "out");

	this_cli->plen = this_cli->prompt(0);

	/* Must be set before command files are executed */
	this_cli->quit_flag = 0;

	cli_execute_cmdfiles();

	while(!this_cli->quit_flag) {
		if (cli_poll(&c))
			cli_input(&c, 1);
		rte_pause();
	}

	cli_printf("\n");
}

void
cli_start_with_timers(const char *msg)
{
	RTE_ASSERT(this_cli != NULL);

	this_cli->flags |= CLI_USE_TIMERS;

	cli_start(msg);
}

/* Create a CLI root node for the tree */
struct cli_node *
cli_create_root(const char *dirname)
{
	struct cli_node *root;
	cli_funcs_t funcs;

	funcs.cfunc = NULL;

	/* Create and add the root directory */
	root = __add_node(dirname, NULL, CLI_DIR_NODE, funcs, NULL);
	if (!root)
		return NULL;

	TAILQ_INSERT_HEAD(&this_cli->root, root, next);

	/* point at the root directory for current working directory */
	set_cwd(root);

	return root;
}

/* Default CLI prompt routine */
static int
__default_prompt(int cont)
{
	char *str = cli_cwd_path();
	int len = 0;

	if (strlen(str) > 1)	/* trim the trailing '/' from string */
		str[strlen(str) - 1] = '\0';

	scrn_color(SCRN_GREEN, SCRN_NO_CHANGE, SCRN_OFF);
	len += cli_printf("%s:", (cont) ? " >> " : "DPDK-cli");
	scrn_color(SCRN_CYAN, SCRN_NO_CHANGE, SCRN_OFF);
	len += cli_printf("%s", str);
	scrn_color(SCRN_DEFAULT_FG, SCRN_DEFAULT_BG, SCRN_OFF);
	len += cli_printf("> ");

	return len;
}

/* Main entry point to create a CLI system */
int
cli_init(int nb_entries, uint32_t nb_hist)
{
	int i;
	size_t size;
	struct cli *cli;
	struct cli_node *node;
	struct cli_node *root;

	cli = malloc(sizeof(struct cli));
	if (cli == NULL) {
		RTE_LOG(ERR, EAL, "Unable to allocate CLI structure\n");
		return -1;
	}
	memset(cli, '\0', sizeof(struct cli));

	this_cli = cli;

	cli->prompt = __default_prompt;

	if (nb_entries == 0)
		nb_entries = CLI_DEFAULT_NB_NODES;
	else if (nb_entries == -1) {
		cli->flags |= CLI_NODES_UNLIMITED;
		nb_entries = CLI_DEFAULT_NB_NODES;
	}
	cli->nb_nodes = nb_entries;

	if (nb_hist == (uint32_t)CLI_DEFAULT_HISTORY)
		nb_hist = CLI_DEFAULT_HIST_LINES;

	size = (nb_entries * sizeof(struct cli_node));
	cli->node_mem = malloc(size);
	if (cli->node_mem == NULL) {
		RTE_LOG(ERR, EAL, "Unable to allocate CLI node structures\n");
		goto error_exit;
	}
	memset(cli->node_mem, '\0', size);

	cli->nb_hist = nb_hist;

	TAILQ_INIT(&cli->root);		/* Init the directory list */
	TAILQ_INIT(&cli->free_nodes);	/* List of free nodes */
	TAILQ_INIT(&cli->help_nodes);	/* List of help nodes */

	CIRCLEQ_INIT(&cli->free_hist);	/* List of free hist nodes */
	CIRCLEQ_INIT(&cli->hd_hist);	/* Init the history for list head */

	if (scrn_create_with_defaults(SCRN_THEME_ON))
		goto error_exit;

	cli->vt = vt100_setup();
	if (!cli->vt)
		goto error_exit;

	cli->scratch = calloc(CLI_MAX_SCRATCH_LENGTH + 1, 1);
	if (!cli->scratch)
		goto error_exit;

	cli->argv = calloc(CLI_MAX_ARGVS, sizeof(void *));
	if (!cli->argv)
		goto error_exit;

	/* Create the pool for the number of nodes */
	node = cli->node_mem;
	for (i = 0; i < nb_entries; i++, node++)
		TAILQ_INSERT_TAIL(&cli->free_nodes, node, next);

	root = cli_create_root(CLI_ROOT_NAME);
	if (!root) {
		RTE_LOG(ERR, EAL, "Unable to create root directory\n");
		goto error_exit;
	}

	/* Set current working directory to root*/
	set_cwd(root);

	if (cli_set_history(nb_hist)) {
		RTE_LOG(ERR, EAL, "Unable to create history\n");
		goto error_exit;
	}

	/* create and initialize the gap buffer structures */
	cli->gb = gb_create();
	if (!cli->gb) {
		RTE_LOG(ERR, EAL, "Unable to create Gap Buffer\n");
		goto error_exit;
	}

	/* Startup the environment system */
	cli->env = cli_env_create();
	if (!cli->env)
		goto error_exit;

	return 0;

error_exit:
	cli_destroy();
	return -1;
}

int
cli_create(void)
{
	return cli_init(CLI_DEFAULT_NODES, CLI_DEFAULT_HIST_LINES);
}

int
cli_create_with_defaults(void)
{
	if (cli_init(CLI_DEFAULT_NODES, CLI_DEFAULT_HIST_LINES) == 0)
		return cli_setup_with_defaults();
	return -1;
}

/* Cleanup the CLI allocation of memory */
void
cli_destroy(void)
{
	struct cli *cli = this_cli;

	if (!cli)
		return;

	gb_destroy(cli->gb);
	vt100_free(cli->vt);
	scrn_destroy();
	cli_history_delete();

	free(cli->scratch);
	free(cli->kill);
	free(cli->argv);
	free(cli->hist_mem);
	free(cli->node_mem);
	free(cli);

	this_cli = NULL;
}

int
cli_setup(cli_prompt_t prompt, cli_tree_t default_func)
{
	if (!this_cli)
		return -1;

	/* Set the user or default prompt routine */
	this_cli->prompt = (prompt == NULL) ? __default_prompt : prompt;

	/* when null call our default tree setup routine */
	if (default_func == NULL)
		default_func = cli_default_tree_init;

	/* now call the user supplied func or ours if default_func was NULL */
	return default_func();
}

/* Helper routine around the cli_create() routine */
int
cli_setup_with_defaults(void)
{
	return cli_setup(NULL, NULL);
}

/* Helper routine around the cli_create() routine */
int
cli_setup_with_tree(cli_tree_t tree)
{
	return cli_setup(NULL, tree);
}

/* Add a new prompt routine to the CLI system */
cli_prompt_t
cli_set_prompt(cli_prompt_t prompt)
{
	struct cli *cli = this_cli;
	cli_prompt_t old;

	old = cli->prompt;	/* Save old prompt function */
	cli->prompt = prompt;	/* Install new prompt function */

	if (cli->prompt == NULL)/* Set to default function if NULL */
		cli->prompt = __default_prompt;

	return old;
}

/**
 * set the callout function pointer to execute a lua file.
 */
void
cli_set_lua_callback( int(*func)(void *, const char *))
{
	__dofile_lua = func;
}

/**
 * Load and execute a command file or Lua script file.
 *
 */
int
cli_execute_cmdfile(const char *filename)
{
	if (filename == NULL)
		return 0;

	gb_reset_buf(this_cli->gb);

	if (strstr(filename, ".lua") || strstr(filename, ".LUA") ) {
		if (!this_cli->user_state) {
			cli_printf(">>> User State for CLI not set for Lua\n");
			return -1;
		}
		if (__dofile_lua) {
			/* Execute the Lua script file. */
			if (__dofile_lua(this_cli->user_state, filename) != 0)
				return -1;
		} else
			cli_printf(">>> Lua is not enabled in configuration!\n");
	} else {
		FILE    *fd;
		char buff[256];

		fd = fopen(filename, "r");
		if (fd == NULL)
			return -1;

		/* Read and feed the lines to the cmdline parser. */
		while (fgets(buff, sizeof(buff), fd))
			cli_input(buff, strlen(buff));

		fclose(fd);
	}
	return 0;
}

int
cli_execute_cmdfiles(void)
{
	int i, cnt;

	cnt = this_cli->cmd_files.idx;

	for (i = 0; i < cnt; i++) {
		const char *path;
		if ((path = this_cli->cmd_files.filename[i]) == NULL)
			continue;

		if (cli_execute_cmdfile(path))
			return -1;

		free((char *)(uintptr_t)path);
		this_cli->cmd_files.filename[i] = NULL;
	}
	this_cli->cmd_files.idx = 0;
	return 0;
}
