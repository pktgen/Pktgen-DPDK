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
/* Created by: Keith Wiles @ intel */
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

	env = (struct cli_env *)malloc(sizeof(struct cli_env));
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

	TAILQ_FOREACH(n, &env->head, next) {
		if (!strcmp(var, n->var))
			return n;
	}
	return NULL;
}

static struct env_node *
__env_set(struct cli_env *env, const char *var, const char *val)
{
	struct env_node *n;

	if (!env || !var)
		return NULL;

	n = find_env(env, var);
	if (n) {
		free((char *)(uintptr_t)n->val);
		n->var = strdup(val);
		return n;
	}

	n = (struct env_node *)malloc(sizeof(struct env_node));
	if (!n)
		return NULL;

	n->var = strdup(var);
	n->val = strdup(val);

	TAILQ_INSERT_TAIL(&env->head, n, next);
	env->count++;

	return n;
}

int
cli_env_set(struct cli_env *env, const char *var, const char *val)
{
	return (__env_set(env, var, val) == NULL)? -1 : 0;
}

int
cli_env_string(struct cli_env *env, const char *var,
			   cli_sfunc_t sfunc, const char *val)
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

	return (n->sfunc)? n->sfunc(n->val) : n->val;
}

int
cli_env_del(struct cli_env *env, const char *var)
{
	return env_free(env, find_env(env, var));
}

void
cli_env_substitution(struct cli_env *env, char *line, int sz)
{
	char *p, *s, *e, *t, *tmp;
	const char *v;

	if (!env || !line || sz <= 0)
		return;

	tmp = alloca(sz + 1);
	if (!tmp)
		return;
	memset(tmp, '\0', sz + 1);

	e = line + sz;
	for (p = line, t = tmp; (p[0] != '\0') || (p < e); p++) {
		if ((p[0] == '$') && ((p[1] == '{') || (p[1] == '('))) {
			s = strchr(p, (p[1] == '{') ? '}' : ')');
			if (s) {
				*s++ = '\0';
				v = cli_env_get(env, &p[2]);
				if (v) {
					int n = strlen(v);
					memcpy(t, v, n);
					t += n;
					continue;
				}
				p = s;
			}
		}
		*t++ = *p;
	}
	*t = '\0';
	snprintf(line, sz, "%s", tmp);
}

int
cli_env_get_all(struct cli_env *env, struct env_node **list, int max_size)
{
	struct env_node *node;
	int n = 0;

	if (!env)
		return 0;

	TAILQ_FOREACH(node, &env->head, next) {
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

	TAILQ_FOREACH(node, &env->head, next) {
		if (node->sfunc)
			cli_printf("  \"%s\" = \"%s\"\n", node->var, node->sfunc(node->val));
		else
			cli_printf("  \"%s\" = \"%s\"\n", node->var, node->val);
	}
}