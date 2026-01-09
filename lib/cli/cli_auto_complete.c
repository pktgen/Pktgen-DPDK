/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2025>, Intel Corporation.
 */

#include <fnmatch.h>

#include <rte_timer.h>

#include <rte_string_fns.h>
#include <pg_strings.h>

#include "cli.h"
#include "cli_input.h"
#include "cli_auto_complete.h"

/*
 * gb_get_prev() has side-effects (it can move gb->point across the gap).
 * Auto-complete should never mutate cursor state just to inspect a character.
 */
static inline char
_gb_peek_prev(const struct gapbuf *gb)
{
    const char *point;

    if (!gb)
        return '\0';

    point = gb->point;
    if (point == gb->egap)
        point = gb->gap;

    if (point == gb->buf) {
        if (point == gb->gap)
            return '\0';
        return *point;
    }

    return *(point - 1);
}

static uint32_t
_ac_hash_line(const char *s, int arg_index, int at_new_token)
{
    /* 32-bit FNV-1a */
    uint32_t h = 2166136261u;

    if (s) {
        for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; p++) {
            h ^= (uint32_t)(*p);
            h *= 16777619u;
        }
    }

    h ^= (uint32_t)arg_index;
    h *= 16777619u;
    h ^= (uint32_t)at_new_token;
    h *= 16777619u;

    return h;
}

static int
_str_list_add_unique(char ***list, int *count, const char *s)
{
    if (!list || !count || !s || (*s == '\0'))
        return -1;

    for (int i = 0; i < *count; i++) {
        if (!strcmp((*list)[i], s))
            return 0;
    }

    char **new_list = realloc(*list, (size_t)(*count + 1) * sizeof(char *));
    if (!new_list)
        return -1;
    *list = new_list;

    (*list)[*count] = strdup(s);
    if (!(*list)[*count])
        return -1;

    (*count)++;
    return 0;
}

static void
_str_list_free(char **list, int count)
{
    if (!list)
        return;
    for (int i = 0; i < count; i++)
        free(list[i]);
    free(list);
}

static void
_print_strings(char **items, int item_cnt, const char *match)
{
    uint32_t mlen = 8, csize, ccnt, cnt = 0;
    uint32_t slen = (match) ? strlen(match) : 0;

    if (!items || item_cnt <= 0)
        return;

    for (int i = 0; i < item_cnt; i++)
        mlen = RTE_MAX(mlen, (uint32_t)strlen(items[i]));
    mlen++;

    csize = mlen;
    ccnt  = CLI_SCREEN_WIDTH / mlen;
    if (ccnt == 0)
        ccnt = 1;

    for (int i = 0; i < item_cnt; i++) {
        if (slen && strncmp(items[i], match, slen))
            continue;

        if (!cnt)
            cli_printf("\n");

        cli_printf("%-*s", csize, items[i]);
        if ((++cnt % ccnt) == 0)
            cli_printf("\n");
    }
    if (cnt % ccnt)
        cli_printf("\n");
}

static int
_is_placeholder(const char *tok)
{
    return tok && tok[0] == '%' && tok[1] != '\0';
}

static int
_is_choice_token(const char *tok)
{
    return tok && tok[0] == '%' && tok[1] == '|';
}

static int
_choice_token_contains(const char *choice_tok, const char *word)
{
    char tmp[CLI_MAX_PATH_LENGTH + 1];
    char *opts[CLI_MAX_ARGVS + 1];

    if (!choice_tok || !word || !_is_choice_token(choice_tok))
        return 0;

    snprintf(tmp, sizeof(tmp), "%s", choice_tok + 2);
    memset(opts, 0, sizeof(opts));
    int n = pg_strtok(tmp, "|", opts, CLI_MAX_ARGVS);
    for (int i = 0; i < n; i++) {
        if (opts[i] && !strcmp(opts[i], word))
            return 1;
    }
    return 0;
}

static const char *
_placeholder_hint(const char *tok)
{
    if (!tok || tok[0] != '%' || tok[1] == '\0')
        return NULL;

    /* Only provide hints for structured values to avoid noisy "<string>/<int>" suggestions. */
    switch (tok[1]) {
    case 'P':
        return "<portlist>";
    case 'C':
        return "<corelist>";
    case 'm':
        return "<mac>";
    case '4':
        return "<ipv4>";
    case '6':
        return "<ipv6>";
    case 'k':
        return "<kvargs>";
    case 'l':
        return "<list>";
    default:
        return NULL;
    }
}

static int
_is_hint_candidate(const char *s)
{
    size_t len;

    if (!s)
        return 0;

    len = strlen(s);
    return (len >= 3 && s[0] == '<' && s[len - 1] == '>');
}

static int
_env_collect_var_candidates(const char *prefix, char ***out_list, int *out_cnt)
{
    struct cli_env *env = this_cli ? this_cli->env : NULL;
    struct env_node **list;
    int max;
    int n;

    if (!env || !out_list || !out_cnt)
        return 0;

    max = cli_env_count(env);
    if (max <= 0)
        return 0;

    list = calloc((size_t)max, sizeof(*list));
    if (!list)
        return 0;

    n = cli_env_get_all(env, list, max);
    for (int i = 0; i < n; i++) {
        const char *name;

        if (!list[i])
            continue;
        name = list[i]->var;
        if (!name || name[0] == '\0')
            continue;
        if (prefix && *prefix && strncmp(name, prefix, strlen(prefix)))
            continue;
        if (_str_list_add_unique(out_list, out_cnt, name)) {
            free(list);
            return -1;
        }
    }

    free(list);
    return *out_cnt;
}

static int
_portlist_collect_candidates(const char *prefix, char ***out_list, int *out_cnt)
{
    /* Minimal, but useful: the CLI help documents 'all' as a valid portlist. */
    if (prefix && *prefix && strncmp("all", prefix, strlen(prefix)))
        return *out_cnt;

    if (_str_list_add_unique(out_list, out_cnt, "all"))
        return -1;

    return *out_cnt;
}

static int
_map_collect_placeholder_candidates(const char *cmd, char **mtoks, int mtokc, int arg_index,
                                    int argc, char **argv, const char *placeholder,
                                    const char *prefix, char ***out_list, int *out_cnt)
{
    (void)mtokc;

    if (!cmd || !mtoks || !argv || !out_list || !out_cnt)
        return 0;

    /* env get|set|del <VAR> : complete existing environment variable names */
    if (!strcmp(cmd, "env") && arg_index == 2 && argc >= 2 && argv[1] &&
        (!strcmp(argv[1], "get") || !strcmp(argv[1], "set") || !strcmp(argv[1], "del")))
        return _env_collect_var_candidates(prefix, out_list, out_cnt);

    /* seq/sequence <seq#> ... : show a helpful hint for the required sequence number */
    if ((!strcmp(cmd, "seq") || !strcmp(cmd, "sequence")) && arg_index == 1 && placeholder &&
        !strcmp(placeholder, "%d")) {
        if (!prefix || !*prefix) {
            if (_str_list_add_unique(out_list, out_cnt, "<seq#>"))
                return -1;
        }
        return *out_cnt;
    }

    /* %P (portlist) : suggest common values like 'all' */
    if (placeholder && placeholder[0] == '%' && placeholder[1] == 'P')
        return _portlist_collect_candidates(prefix, out_list, out_cnt);

    return 0;
}

static int
_map_collect_candidates(struct cli_map *maps, int argc, char **argv, int arg_index,
                        const char *prefix, char ***out_list, int *out_cnt)
{
    char fmt_copy[CLI_MAX_PATH_LENGTH + 1];
    char *mtoks[CLI_MAX_ARGVS + 1];

    if (!maps || argc <= 0 || !argv || !argv[0] || arg_index < 1)
        return 0;

    for (int mi = 0; maps[mi].fmt != NULL; mi++) {
        memset(mtoks, 0, sizeof(mtoks));
        snprintf(fmt_copy, sizeof(fmt_copy), "%s", maps[mi].fmt);

        int mtokc = pg_strtok(fmt_copy, " ", mtoks, CLI_MAX_ARGVS);
        if (mtokc <= arg_index)
            continue;

        /* map must be for this command */
        if (!mtoks[0])
            continue;
        if (_is_choice_token(mtoks[0])) {
            if (!_choice_token_contains(mtoks[0], argv[0]))
                continue;
        } else if (strcmp(mtoks[0], argv[0])) {
            continue;
        }

        /* must match already-typed tokens before the arg we are completing */
        int ok = 1;
        for (int i = 0; i < arg_index && i < argc && i < mtokc; i++) {
            if (_is_placeholder(mtoks[i]))
                continue;
            if (!argv[i] || strcmp(mtoks[i], argv[i])) {
                ok = 0;
                break;
            }
        }
        if (!ok)
            continue;

        const char *cand_tok = mtoks[arg_index];
        if (!cand_tok || cand_tok[0] == '\0')
            continue;

        /* Placeholder-aware completion (e.g., env var names for env get/set/del). */
        if (_is_placeholder(cand_tok) && !_is_choice_token(cand_tok)) {
            if (_map_collect_placeholder_candidates(argv[0], mtoks, mtokc, arg_index, argc, argv,
                                                    cand_tok, prefix, out_list, out_cnt) < 0)
                return -1;

            /* If no better candidates exist, provide a human hint like "<portlist>". */
            if (!prefix || !*prefix) {
                const char *hint = _placeholder_hint(cand_tok);
                if (hint && _str_list_add_unique(out_list, out_cnt, hint))
                    return -1;
            }
            continue;
        }

        if (_is_choice_token(cand_tok)) {
            /* cand_tok is like "%|a|b|c" */
            char opt_copy[CLI_MAX_PATH_LENGTH + 1];
            char *opts[CLI_MAX_ARGVS + 1];
            memset(opts, 0, sizeof(opts));
            snprintf(opt_copy, sizeof(opt_copy), "%s", cand_tok + 2);
            int n = pg_strtok(opt_copy, "|", opts, CLI_MAX_ARGVS);
            for (int oi = 0; oi < n; oi++) {
                if (!opts[oi])
                    continue;
                if (prefix && *prefix && strncmp(opts[oi], prefix, strlen(prefix)))
                    continue;
                if (_str_list_add_unique(out_list, out_cnt, opts[oi]))
                    return -1;
            }
        } else {
            if (prefix && *prefix && strncmp(cand_tok, prefix, strlen(prefix)))
                continue;
            if (_str_list_add_unique(out_list, out_cnt, cand_tok))
                return -1;
        }
    }

    return *out_cnt;
}

static int
_map_next_is_user_value(struct cli_map *maps, int argc, char **argv, int arg_index)
{
    char fmt_copy[CLI_MAX_PATH_LENGTH + 1];
    char *mtoks[CLI_MAX_ARGVS + 1];

    if (!maps || argc <= 0 || !argv || !argv[0] || arg_index < 1)
        return 0;

    for (int mi = 0; maps[mi].fmt != NULL; mi++) {
        memset(mtoks, 0, sizeof(mtoks));
        snprintf(fmt_copy, sizeof(fmt_copy), "%s", maps[mi].fmt);
        int mtokc = pg_strtok(fmt_copy, " ", mtoks, CLI_MAX_ARGVS);
        if (mtokc <= arg_index)
            continue;

        if (!mtoks[0])
            continue;
        if (_is_choice_token(mtoks[0])) {
            if (!_choice_token_contains(mtoks[0], argv[0]))
                continue;
        } else if (strcmp(mtoks[0], argv[0])) {
            continue;
        }

        int ok = 1;
        for (int i = 0; i < arg_index && i < argc && i < mtokc; i++) {
            if (_is_placeholder(mtoks[i]))
                continue;
            if (!argv[i] || strcmp(mtoks[i], argv[i])) {
                ok = 0;
                break;
            }
        }
        if (!ok)
            continue;

        const char *tok = mtoks[arg_index];
        if (!tok || tok[0] == '\0')
            continue;

        if (_is_placeholder(tok) && !_is_choice_token(tok))
            return 1;
    }

    return 0;
}

static int
_map_current_is_user_value(struct cli_map *maps, int argc, char **argv, int arg_index)
{
    char fmt_copy[CLI_MAX_PATH_LENGTH + 1];
    char *mtoks[CLI_MAX_ARGVS + 1];

    if (!maps || argc <= 0 || !argv || !argv[0] || arg_index < 1)
        return 0;

    for (int mi = 0; maps[mi].fmt != NULL; mi++) {
        memset(mtoks, 0, sizeof(mtoks));
        snprintf(fmt_copy, sizeof(fmt_copy), "%s", maps[mi].fmt);
        int mtokc = pg_strtok(fmt_copy, " ", mtoks, CLI_MAX_ARGVS);
        if (mtokc <= arg_index)
            continue;

        if (!mtoks[0])
            continue;
        if (_is_choice_token(mtoks[0])) {
            if (!_choice_token_contains(mtoks[0], argv[0]))
                continue;
        } else if (strcmp(mtoks[0], argv[0])) {
            continue;
        }

        int ok = 1;
        for (int i = 0; i < arg_index && i < argc && i < mtokc; i++) {
            if (_is_placeholder(mtoks[i]))
                continue;
            if (!argv[i] || strcmp(mtoks[i], argv[i])) {
                ok = 0;
                break;
            }
        }
        if (!ok)
            continue;

        const char *tok = mtoks[arg_index];
        if (!tok || tok[0] == '\0')
            continue;

        if (_is_placeholder(tok) && !_is_choice_token(tok))
            return 1;
    }

    return 0;
}

static int
_map_has_tokens_after(struct cli_map *maps, int argc, char **argv, int arg_index)
{
    char fmt_copy[CLI_MAX_PATH_LENGTH + 1];
    char *mtoks[CLI_MAX_ARGVS + 1];

    if (!maps || argc <= 0 || !argv || !argv[0] || arg_index < 1)
        return 0;

    for (int mi = 0; maps[mi].fmt != NULL; mi++) {
        memset(mtoks, 0, sizeof(mtoks));
        snprintf(fmt_copy, sizeof(fmt_copy), "%s", maps[mi].fmt);
        int mtokc = pg_strtok(fmt_copy, " ", mtoks, CLI_MAX_ARGVS);

        if (!mtoks[0])
            continue;
        if (_is_choice_token(mtoks[0])) {
            if (!_choice_token_contains(mtoks[0], argv[0]))
                continue;
        } else if (strcmp(mtoks[0], argv[0])) {
            continue;
        }

        int ok = 1;
        for (int i = 0; i < arg_index && i < argc && i < mtokc; i++) {
            if (_is_placeholder(mtoks[i]))
                continue;
            if (!argv[i] || strcmp(mtoks[i], argv[i])) {
                ok = 0;
                break;
            }
        }
        if (!ok)
            continue;

        if (mtokc > (arg_index + 1))
            return 1;
    }

    return 0;
}

static uint32_t
_column_count(struct cli_node **nodes, uint32_t node_cnt, uint32_t *len)
{
    uint32_t i, mlen = 8, cs;

    if (!nodes || !len)
        return CLI_SCREEN_WIDTH / mlen;

    /* Calculate the column size */
    for (i = 0; i < node_cnt; i++)
        mlen = RTE_MAX(mlen, strlen(nodes[i]->name));
    mlen++; /* Make sure we have at least a space between */

    *len = mlen;
    cs   = CLI_SCREEN_WIDTH / mlen;

    return cs;
}

static int
_print_nodes(struct cli_node **nodes, uint32_t node_cnt, uint32_t dir_only, char *match,
             struct cli_node **ret)
{
    struct cli_node *n;
    uint32_t i, cnt = 0, ccnt, found = 0, slen, csize;

    if (!node_cnt || !nodes)
        return 0;

    ccnt = _column_count(nodes, node_cnt, &csize);

    slen = (match) ? strlen(match) : 0;

    /* display the node names */
    for (i = 0; i < node_cnt; i++) {
        n = nodes[i];

        if (dir_only && !is_directory(n))
            continue;

        if (slen && strncmp(n->name, match, slen))
            continue;

        if (!cnt)
            cli_printf("\n");

        cli_printf("%-*s", csize, n->name);
        if ((++cnt % ccnt) == 0)
            cli_printf("\n");

        /* Found a possible match */
        if (ret)
            *ret = n;
        found++;
    }

    /* if not nodes found cnt will be zero and no CR */
    if (cnt % ccnt)
        cli_printf("\n");

    return found;
}

static int
qsort_compare(const void *p1, const void *p2)
{
    const struct cli_node *n1, *n2;

    n1 = *(const struct cli_node *const *)p1;
    n2 = *(const struct cli_node *const *)p2;

    return strcmp(n1->name, n2->name);
}

static int
complete_args(int argc, char **argv, uint32_t types)
{
    struct cli_node **nodes = NULL, *node = NULL;
    struct gapbuf *gb;
    char *match;
    uint32_t node_cnt, found = 0, dir_only = 0, slen;

    if (argc)
        match = argv[argc - 1];
    else
        match = NULL;

    gb = this_cli->gb;

    if (match) {
        uint32_t stype;
        uint32_t slashes;
        char *p;

        /* Count the number of slashes in the path */
        slashes = pg_strcnt(match, '/');

        if (slashes) {
            /* full path to command given */
            if (cli_find_node(match, &node))
                if (is_executable(node))
                    return 0;

            /* if not found get last directory in path */
            node = cli_last_dir_in_path(match);

            if ((slashes == 1) && (match && (match[0] == '/'))) {
                match++;
                dir_only++;
            }
        }

        stype = CLI_ALL_TYPE; /* search for all nodes */
        if (argc > 1)
            stype = CLI_OTHER_TYPE; /* search for non-exe nodes */

        node_cnt = cli_node_list_with_type(node, stype, (void **)&nodes);
        p        = strrchr(match, '/');
        if (p)
            match = ++p;
    } else
        node_cnt = cli_node_list_with_type(NULL, types, (void **)&nodes);

    if (node_cnt) {
        struct cli_node *mnode = NULL;

        if (node_cnt > 1)
            qsort(nodes, node_cnt, sizeof(void *), qsort_compare);

        found = _print_nodes(nodes, node_cnt, dir_only, match, &mnode);

        /*
         * _match is a pointer to the last matched node
         * _found is a flag to determine if pointer is valid
         */
        if (mnode && (found == 1)) { /* Found a possible match */
            struct cli_node *node = (struct cli_node *)mnode;
            char *s;
            int nlen;

            s = strrchr(match, '/');
            if (s)
                match = ++s;

            slen = strlen(match);
            nlen = (strlen(node->name) - slen);

            if (nlen > 0) /* Add the rest of the matching command */
                gb_str_insert(gb, &node->name[slen], nlen);

            if (is_directory(node))
                gb_str_insert(gb, (char *)(uintptr_t)"/", 1);
            else
                gb_str_insert(gb, (char *)(uintptr_t)" ", 1);
        }
    }
    cli_node_list_free(nodes);

    return found;
}

void
cli_auto_complete(void)
{
    char *argv[CLI_MAX_ARGVS + 1];
    char *line = NULL;
    int argc, size, ret;
    int at_new_token;
    int arg_index;
    struct cli_map *maps = NULL;
    char **cands         = NULL;
    int cand_cnt         = 0;
    uint64_t now_tsc;
    uint32_t line_hash;
    int force_usage = 0;

    memset(argv, '\0', sizeof(argv));

    size = gb_data_size(this_cli->gb);
    if (!size)
        return;

    size = RTE_MIN(size, (int)(CLI_MAX_SCRATCH_LENGTH - 1));

    line = calloc(1, (size_t)size + 1);
    if (!line)
        return;

    gb_copy_to_buf(this_cli->gb, line, size);

    argc = pg_strqtok(line, " \r\n", argv, CLI_MAX_ARGVS);

    /* Be defensive: some tokenizers may yield empty/NULL trailing tokens. */
    while (argc > 0 && (!argv[argc - 1] || argv[argc - 1][0] == '\0'))
        argc--;

    at_new_token = 0;
    if (!gb_point_at_start(this_cli->gb)) {
        char prev    = _gb_peek_prev(this_cli->gb);
        at_new_token = (prev == ' ' || prev == '\t' || prev == '\n' || prev == '\r');
    }

    /* Determine which argv slot we are completing */
    if (argc == 0)
        arg_index = 0;
    else
        arg_index = at_new_token ? argc : (argc - 1);

    /* Double-tab detection: same line state within a short time window */
    now_tsc   = rte_get_timer_cycles();
    line_hash = _ac_hash_line(line, arg_index, at_new_token);
    {
        const uint64_t hz            = rte_get_timer_hz();
        const uint64_t window_cycles = (hz) ? (hz * 4 / 10) : 0; /* ~400ms */

        if (window_cycles && this_cli->ac_last_hash == line_hash &&
            (now_tsc - this_cli->ac_last_tsc) <= window_cycles)
            force_usage = 1;

        this_cli->ac_last_hash = line_hash;
        this_cli->ac_last_tsc  = now_tsc;
    }

    if (argc == 0) {
        ret = complete_args(argc, argv, CLI_ALL_TYPE);

        if (ret)
            cli_redisplay_line();
        free(line);
        return;
    }

    /* If the first token is a known command with a map, try map-driven completion */
    maps = (argc > 0 && argv[0]) ? cli_get_cmd_map(argv[0]) : NULL;
    if (maps && arg_index >= 1) {
        const char *prefix = NULL;
        if (!at_new_token && arg_index < argc && argv[arg_index])
            prefix = argv[arg_index];

        ret = _map_collect_candidates(maps, argc, argv, arg_index, prefix, &cands, &cand_cnt);
        if (ret < 0) {
            _str_list_free(cands, cand_cnt);
            free(line);
            return;
        }

        /*
         * If we are sitting on a user-value token with no suggestions (e.g. "set 0<Tab>"),
         * treat Tab as end-of-token: insert a space and complete the next map token.
         */
        if (cand_cnt == 0 && !at_new_token && prefix && *prefix &&
            _map_current_is_user_value(maps, argc, argv, arg_index) &&
            _map_has_tokens_after(maps, argc, argv, arg_index)) {
            gb_str_insert(this_cli->gb, (char *)(uintptr_t)" ", 1);
            cli_redisplay_line();

            _str_list_free(cands, cand_cnt);
            cands    = NULL;
            cand_cnt = 0;

            at_new_token = 1;
            arg_index    = argc;
            prefix       = NULL;

            ret = _map_collect_candidates(maps, argc, argv, arg_index, prefix, &cands, &cand_cnt);
            if (ret < 0) {
                _str_list_free(cands, cand_cnt);
                free(line);
                return;
            }
        }

        if (cand_cnt == 1) {
            const char *ins = cands[0];
            size_t plen     = (prefix) ? strlen(prefix) : 0;

            if (_is_hint_candidate(ins)) {
                _print_strings(cands, cand_cnt, prefix);
                cli_redisplay_line();
            } else if (plen <= strlen(ins)) {
                gb_str_insert(this_cli->gb, (char *)(uintptr_t)&ins[plen], strlen(ins) - plen);
                gb_str_insert(this_cli->gb, (char *)(uintptr_t)" ", 1);
                cli_redisplay_line();
            }
        } else if (cand_cnt > 1) {
            _print_strings(cands, cand_cnt, prefix);
            cli_redisplay_line();
        } else if (at_new_token) {
            /* If next token is user input, don't spam usage; otherwise show usage safely. */
            if (!_map_next_is_user_value(maps, argc, argv, arg_index)) {
                cli_printf("\n");
                cli_maps_show(maps, argc, argv);
                cli_redisplay_line();
            } else if (force_usage) {
                cli_printf("\n");
                cli_maps_show(maps, argc, argv);
                cli_redisplay_line();
            }
        }

        _str_list_free(cands, cand_cnt);
        free(line);
        return;
    }

    /* no space before cursor maybe a command completion request */
    if (gb_point_at_start(this_cli->gb) || _gb_peek_prev(this_cli->gb) != ' ') {
        if (argc == 1) /* Only one word then look for a command */
            ret = complete_args(argc, argv, CLI_ALL_TYPE);
        else /* If more then one word then look for file/dir */
            ret = complete_args(argc, argv, CLI_FILE_NODE | CLI_DIR_NODE);

        /* if we get an error then redisplay the line */
        if (ret)
            cli_redisplay_line();
    } else {
        char *save = calloc(1, (size_t)size + 1);

        if (!save) {
            free(line);
            return;
        }

        /* Call function to print out help text, plus save a copy */
        gb_copy_to_buf(this_cli->gb, save, size);

        /* Add the -? to the command */
        gb_str_insert(this_cli->gb, (char *)(uintptr_t)"-?", 2);

        cli_execute();

        /* reset the input buffer to remove -? */
        gb_reset_buf(this_cli->gb);

        /* insert the saved string back to the input buffer */
        gb_str_insert(this_cli->gb, save, size);

        cli_redisplay_line();

        free(save);
    }

    free(line);
}
