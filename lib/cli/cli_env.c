/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2026>, Intel Corporation.
 */
/* Created by: Keith Wiles @ intel */

/**
 * @file
 * CLI environment variable implementation.
 */
#include <stdlib.h>

#include "cli.h"
#include "cli_env.h"

static int
env_free(struct cli_env *env, struct env_node *n)
{
    if (!env || !n)
        return -1;

    TAILQ_REMOVE(&env->head, n, next);

    free((char *)(uintptr_t)n->var);
    free((char *)(uintptr_t)n->val);
    free(n);
    env->count--;

    return 0;
}

struct cli_env *
cli_env_create(void)
{
    struct cli_env *env;

    env = (struct cli_env *)calloc(1, sizeof(struct cli_env));
    if (!env)
        return NULL;
    memset(env, '\0', sizeof(struct cli_env));

    TAILQ_INIT(&env->head);

    return env;
}

void
cli_env_destroy(struct cli_env *env)
{
    struct env_node *n;

    if (!env)
        return;

    while (!TAILQ_EMPTY(&env->head)) {
        n = TAILQ_FIRST(&env->head);
        env_free(env, n);
    }
    free(env);
}

static struct env_node *
find_env(struct cli_env *env, const char *var)
{
    struct env_node *n;

    TAILQ_FOREACH (n, &env->head, next) {
        if (!strcmp(var, n->var))
            return n;
    }
    return NULL;
}

static struct env_node *
__env_set(struct cli_env *env, const char *var, const char *val)
{
    struct env_node *n;
    const char *safe_val;

    if (!env || !var)
        return NULL;

    safe_val = (val) ? val : "";

    n = find_env(env, var);
    if (n) {
        char *new_val = strdup(safe_val);

        if (!new_val)
            return NULL;

        free((char *)(uintptr_t)n->val);
        n->val = new_val;
        return n;
    }

    n = (struct env_node *)calloc(1, sizeof(struct env_node));
    if (!n)
        return NULL;

    n->var = strdup(var);
    n->val = strdup(safe_val);
    if (!n->var || !n->val) {
        free((char *)(uintptr_t)n->var);
        free((char *)(uintptr_t)n->val);
        free(n);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&env->head, n, next);
    env->count++;

    return n;
}

int
cli_env_set(struct cli_env *env, const char *var, const char *val)
{
    return (__env_set(env, var, val) == NULL) ? -1 : 0;
}

int
cli_env_string(struct cli_env *env, const char *var, cli_sfunc_t sfunc, const char *val)
{
    struct env_node *n;

    n = __env_set(env, var, val);
    if (!n)
        return -1;
    n->sfunc = sfunc;
    return 0;
}

const char *
cli_env_get(struct cli_env *env, const char *var)
{
    struct env_node *n;

    n = find_env(env, var);
    if (!n)
        return NULL;

    return (n->sfunc) ? n->sfunc(n->val) : n->val;
}

int
cli_env_del(struct cli_env *env, const char *var)
{
    return env_free(env, find_env(env, var));
}

/* strings to be substituted are of the form ${foo} or $(foo) */
void
cli_env_substitution(struct cli_env *env, char *line, int sz)
{
    char *p, *s, *e, *t, *tmp;
    const char *v;
    size_t remaining;

    if (!env || !line || sz <= 0)
        return;

    tmp = calloc(1, (size_t)sz + 1);
    if (!tmp)
        return;

    /* Determine the end of the string */
    e = line + sz;

    remaining = (size_t)sz;

    for (p = line, t = tmp; (p[0] != '\0') && (p < e); p++) {
        /* Look for the '$' then the open bracket */
        if (p[0] != '$')
            goto next;

        /* find opening bracket */
        if ((p[1] != '{') && (p[1] != '('))
            goto next;

        /* find closing bracket */
        s = strchr(p, (p[1] == '{') ? '}' : ')');
        if (!s)
            goto next;

        /* terminate the variable string */
        *s = '\0';

        v = cli_env_get(env, &p[2]);
        if (!v)
            v = "oops!";

        if (remaining > 1) {
            size_t vlen = strnlen(v, remaining - 1);
            memcpy(t, v, vlen);
            t += vlen;
            remaining -= vlen;
        }
        p = s; /* Point 'p' past the variable */
        continue;
    next:
        if (remaining <= 1)
            break;
        *t++ = *p;
        remaining--;
    }
    *t = '\0';

    snprintf(line, sz, "%s", tmp);

    free(tmp);
}

int
cli_env_get_all(struct cli_env *env, struct env_node **list, int max_size)
{
    struct env_node *node;
    int n = 0;

    if (!env)
        return 0;

    TAILQ_FOREACH (node, &env->head, next) {
        list[n++] = node;
        if (n == max_size)
            break;
    }

    return n;
}

void
cli_env_show(struct cli_env *env)
{
    struct env_node *node;

    TAILQ_FOREACH (node, &env->head, next) {
        if (node->sfunc)
            cli_printf("  \"%s\" = \"%s\"\n", node->var, node->sfunc(node->val));
        else
            cli_printf("  \"%s\" = \"%s\"\n", node->var, node->val);
    }
}
