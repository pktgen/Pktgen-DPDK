/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <dlfcn.h>

#include <rte_config.h>
#include <rte_compat.h>
#include <rte_rwlock.h>

#ifdef RTE_ARCH_X86
#include <rte_hash_crc.h>
#define hash_func       rte_hash_crc
#else
#include <rte_jhash.h>
#define hash_func       rte_jhash
#endif

#include "plugin.h"

static rte_rwlock_t _plugin_rwlock = RTE_RWLOCK_INITIALIZER;
rte_rwlock_t *plugin_rwlock = &_plugin_rwlock;
#define RTE_PLUGIN_TAILQ_RWLOCK	plugin_rwlock

static TAILQ_HEAD(plugin_list, plugin) plugin_head = TAILQ_HEAD_INITIALIZER(plugin_head);

static struct plugin *plugin_inst[PLUGIN_MAX_INST];

static const char *plugin_fmts[] = {
	"%s",
	"%s.so",
	"lib%s",
	"lib%s.so",
	"librte_%s.so",
	"librte_pmd_%s.so",
	NULL
};

struct plugin *
plugin_get(int inst)
{
	if (inst < 0 || inst >= PLUGIN_MAX_INST)
		return NULL;
	return plugin_inst[inst];	/* This could be NULL as well */
}

static int
plugin_add_inst(struct plugin *pin)
{
	int inst;

	rte_rwlock_write_lock(RTE_PLUGIN_TAILQ_RWLOCK);

	for(inst = 0; inst < PLUGIN_MAX_INST; inst++) {
		if (plugin_inst[inst] == NULL) {
			plugin_inst[inst] = pin;

			rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
			return inst;
		}
	}

	rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
	return -1;
}

static int
plugin_del_inst(int inst)
{
	struct plugin *pin = plugin_get(inst);

	if (!pin) {
		PLUGIN_LOG(DEBUG, "Plugin pointer is NULL or info is NULL\n");
		return -1;
	}

	/* remove the plugin from the global list */
	rte_rwlock_write_lock(RTE_PLUGIN_TAILQ_RWLOCK);

	plugin_inst[inst] = NULL;

	if (rte_atomic32_dec_and_test(&pin->refcnt)) {
		/* remove instance when refcnt becomes zero */
		TAILQ_REMOVE(&plugin_head, pin, next);

		dlclose(pin->dl);

		free(pin->plugin);
		free(pin->path);
		free(pin);
	}

	rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
	return 0;
}

static int
plugin_find_inst(struct plugin *pin)
{
	int inst;

	rte_rwlock_write_lock(RTE_PLUGIN_TAILQ_RWLOCK);

	for(inst = 0; inst < PLUGIN_MAX_INST; inst++) {
		if (plugin_inst[inst] == pin) {
			rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
			return inst;
		}
	}

	rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
	return -1;
}

/**
 * Routine to search for and open a plugin.
 *
 * @param pin
 *   The plugin structure pointer.
 * @return
 *   0 if plugin found and loaded or -1 on not found.
 */
static int
plugin_open(struct plugin *pin)
{
	char file[1024];
	int i;

	for(i = 0; plugin_fmts[i]; i++) {
		char fn[64];

		snprintf(fn, sizeof(fn), plugin_fmts[i], pin->plugin);

		if (pin->path)
			snprintf(file, sizeof(file), "%s/%s", pin->path, fn);
		else
			snprintf(file, sizeof(file), "%s", fn);

		printf("Trying to load this file (%s)\n", file);
		pin->dl = dlopen(file, RTLD_NOW | RTLD_LOCAL);
		if (pin->dl)
			return 0;
		printf("dlopen: %s\n", dlerror());
	}
	return -1;
}

int
plugin_get_symbol(struct plugin *pin, const char *sym, char **ret)
{
	char *val;
	void *v;

	if (!pin || !ret)
		return -1;

	*ret = NULL;
	(void)dlerror();		/* Clear errors */

	PLUGIN_LOG(INFO, "dlsym %p for %s:%d\n",
		pin->dl, pin->plugin, pin->inst);
	v = dlsym(pin->dl, sym);	/* could return NULL */

	val = dlerror();
	if (val) {
		PLUGIN_LOG(ERR, "failed to get symbol (%s)\n", val);
		return -1;
	}

	*ret = v;

	return 0;
}

int
plugin_create(char *plugin, char *path)
{
	struct plugin *pin;
	struct plugin_info *info;
	int inst, ret;
	char sym[64];

	/* Make sure we have not already loaded this plugin */
	inst = plugin_find_by_name(plugin);
	if (inst >= 0) {
		pin = plugin_get(inst);

		/* Did we find a new instance of the same plugin */
		if (pin && !strcmp(path, pin->path)) {
			info = pin->info;

			PLUGIN_LOG(DEBUG, "Plugin: '%s' - %s, version: %d.%02d.%02d-rc%d\n",
				pin->plugin, info->desc,
				info->major, info->minor, info->patch,
				info->release);
			return inst;
		}
		/* not the same plugin */
	}

	/* Create the plugin structure and begin loading plugin */
	pin = malloc(sizeof(struct plugin));
	if (!pin)
		return -1;
	memset(pin, 0, sizeof(struct plugin));
	pin->inst = -1;

	inst = plugin_add_inst(pin);
	if (inst < 0) {
		PLUGIN_LOG(ERR, "plugin_add_inst() failed for %s\n", plugin);
		free(pin);
		return -1;
	}

	pin->inst = inst;

	/* Save these away for later comparsions */
	pin->plugin = strdup(plugin);
	pin->path = strdup(path);

	if (plugin_open(pin)) {
		PLUGIN_LOG(ERR, "dlopen() unable to load %s\n", pin->plugin);
		PLUGIN_LOG(ERR, "dlerror: %s\n", dlerror());
		goto err_exit;
	}

	/* Look for the plugin information structure */
	snprintf(sym, sizeof(sym), "%s_plugin_info", pin->plugin);
	ret = plugin_get_symbol(pin, sym, (char **)&info);
	if (ret < 0) {
		PLUGIN_LOG(ERR, "Invalid plugin %s not found\n", sym);
		goto err_exit;
	}
	pin->info = info;

	PLUGIN_LOG(DEBUG, "Plugin: '%s' - %s, version: %d.%02d.%02d-rc%d\n",
		pin->plugin, info->desc,
		info->major, info->minor, info->patch, info->release);

	/* Create the CRC 32bit value using plugin name, path and info data */
	pin->hash = hash_func(pin->plugin, strlen(pin->plugin), pin->hash);
	pin->hash = hash_func(pin->path, strlen(pin->path), pin->hash);
	pin->hash = hash_func(pin->info, sizeof(struct plugin_info), pin->hash);

	/* add the plugin to the global list */
	rte_rwlock_write_lock(RTE_PLUGIN_TAILQ_RWLOCK);
	TAILQ_INSERT_TAIL(&plugin_head, pin, next);
	rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);

	return inst;

err_exit:
	PLUGIN_LOG(ERR, "Failed to create plugin %s\n", plugin);
	plugin_del_inst(inst);
	return -1;
}

int
plugin_destroy(int inst)
{
	if (inst < 0)
		return -1;
	return plugin_del_inst(inst);
}

int
plugin_start(int inst, void *arg)
{
	struct plugin *pin = plugin_get(inst);

	if (!pin) {
		PLUGIN_LOG(DEBUG, "Plugin pointer is NULL\n");
		return -1;
	}

	/* if we have a entry point then call to unload plugin */
	if (pin->info->start(inst, arg)) {
		PLUGIN_LOG(DEBUG, "Start of %s failed\n", pin->plugin);
		return -1;
	}

	return 0;
}

int
plugin_stop(int inst)
{
	struct plugin *pin = plugin_get(inst);

	if (!pin) {
		PLUGIN_LOG(DEBUG, "Plugin pointer is NULL or info is NULL\n");
		return -1;
	}

	/* if we have a entry point then call to unload plugin */
	if (pin->info->stop(inst)) {
		PLUGIN_LOG(DEBUG, "Start of %s failed\n", pin->plugin);
		return -1;
	}

	return 0;
}

int
plugin_find_by_name(const char *name)
{
	struct plugin *pin;

	rte_rwlock_write_lock(RTE_PLUGIN_TAILQ_RWLOCK);

	TAILQ_FOREACH(pin, &plugin_head, next) {
		if (!strcmp(name, pin->plugin)) {
			rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
			return pin->inst;
		}
	}

	rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);

	return -1;
}

int
plugin_find_by_id(uint32_t h)
{
	struct plugin *pin;

	rte_rwlock_write_lock(RTE_PLUGIN_TAILQ_RWLOCK);

	TAILQ_FOREACH(pin, &plugin_head, next) {
		if (h == pin->hash) {
			int inst = plugin_find_inst(pin);
			rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
			return inst;
		}
	}

	rte_rwlock_write_unlock(RTE_PLUGIN_TAILQ_RWLOCK);

	return -1;
}

static void
_plugin_dump(FILE * f, struct plugin *pin)
{
	if (!f)
		f = stderr;

	if (!pin) {
		fprintf(f, "*** Plugin pointer is NULL\n");
		return;
	}

	fprintf(f, "%08x %-16s %6d %s\n", pin->hash, pin->plugin,
		rte_atomic32_read(&pin->refcnt),
		(pin->info)? pin->info->desc : "---");
}

void
plugin_dump(FILE *f)
{
	struct plugin *pin = NULL;
	int cnt = 0;

	rte_rwlock_read_lock(RTE_PLUGIN_TAILQ_RWLOCK);

	fprintf(f, "**** Plugins ****\n");
	fprintf(f, "%-8s %-16s %-6s %s\n", "ID", "Name", "refcnt", "Description");
	TAILQ_FOREACH(pin, &plugin_head, next) {
		_plugin_dump(f, pin);
		cnt++;
	}
	fprintf(f, "Total plugins found: %d\n", cnt);

	rte_rwlock_read_unlock(RTE_PLUGIN_TAILQ_RWLOCK);
}

int libplugin_logtype;

RTE_INIT(libplugin_init_log)
{
	libplugin_logtype = rte_log_register("librte.plugin");
	if (libplugin_logtype >= 0)
		rte_log_set_level(libplugin_logtype, RTE_LOG_INFO);
}
