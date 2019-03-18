/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <sys/types.h>
#include <unistd.h>
#include <sys/queue.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_tailq.h>
#include <rte_atomic.h>

/**
 * @file
 * PLUGIN
 *
 * The plugin library provides the ability to add and remove modules written
 * in C or other languages in a .so format.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_MAX_INST		32

extern int libplugin_logtype;
#define PLUGIN_LOG(level, fmt, args...) \
	rte_log(RTE_LOG_ ## level, libplugin_logtype, "%s(%d): " fmt, \
		__func__, getpid(), ## args)

#define PLUGIN_MKVER(_M, _m, _p, _r) \
	.major = _M, .minor = _m, .patch = _p, .release = _r

#define PLUGIN_INFO(_n, _d, _f, _v) \
	struct plugin_info _n ## _plugin_info = { \
		.desc = _d, \
		.start = _n ## _start, \
		.stop = _n ## _stop, \
		.pfuncs = (void *)_f, \
		_v \
	}

struct plugin_info {
	const char *desc;		/**< Short plugin description */

	int (*start)(int inst, void *arg); /**< start function optional */
	int (*stop)(int inst);		/**< stop function optional */
	void *pfuncs;			/**< plugin defined functions/info */

	RTE_STD_C11
	union {
		uint32_t version;	/* 18.04.00-rc1 == 18040001 */
		struct {
			uint8_t major;	/**< Version of Plugin */
			uint8_t minor;
			uint8_t patch;
			uint8_t release;
		};
	};
};

struct plugin {
	TAILQ_ENTRY(plugin) next;	/**< Next plugin pointer */

	char *plugin;			/**< short name of plugin <name>.do */
	char *path;			/**< path to plugin */
	void *dl;			/**< dlopen handle pointer */
	rte_atomic32_t refcnt;		/**< reference count */
	uint32_t hash;			/**< Plugin ID value */
	int inst;			/**< Plugin instance ID value */
	struct plugin_info *info;	/**< Pointer to plugin info struct */
};

/**
 * Create a plugin instance by defining the name and path if needed.
 *
 * @param name
 *   The short name of the plugin minus the .so or .do, no path included.
 * @param path
 *   The path to the plugin if not in LD_LIBRARY_PATH environment variable.
 * @return
 *   the instance number or -1 if error
 */
int plugin_create(char *name, char *path);

/**
 * Destroy a given instance of a plugin
 *
 * @param inst
 *   The inst number for the plugin.
 * @return
 *    < 0 is error and 0 is success.
 */
int plugin_destroy(int inst);

/**
 * start the plugin
 *
 * @param inst
 *   The instance number
 * @param arg
 *   User defined value to the plugin on start up.
 * @return
 *   -1 on error or 0 if OK.
 */
int plugin_start(int inst, void *arg);

/**
 * stop the plugin
 *
 * @param inst
 *   The instance number
 * @return
 *   -1 on error or 0 if OK.
 */
int plugin_stop(int inst);

/**
 * Return the struct plugin pointer by instance index
 *
 * @param inst
 *    The instance index value
 * @return
 *    The struct rte+plugin pointer for the instance or NULL
 */
struct plugin *plugin_get(int inst);

/**
 * Locate the plugin information by name.
 *
 * @param name
 *   The short name of the plugin to be found.
 * @return
 *   instance number or -1 on error
 */
int plugin_find_by_name(const char *name);

/**
 * Locate the plugin information by ID value.
 *
 * @param id
 *   ID of the plugin.
 * @return
 *   The instance number or -1 on error
 */
int plugin_find_by_id(uint32_t id);

/**
 * Dump out the list of plugins in the system.
 *
 * @param f
 *   The file pointer used to output the information, if NULL use stderr.
 */
void plugin_dump(FILE *f);

/**
 * return the description for the given instance id value
 *
 * @param pin
 *   The plugin_info pointer
 * @return
 *   The description string or NULL if not found.
 */
static inline const char *
plugin_desc(struct plugin *pin)
{
	if (pin && pin->info)
		return pin->info->desc;

	return NULL;
}

/**
 * return the plugin hash ID for the given instance id value
 *
 * @param pin
 *   The plugin data structure pointer
 * @param id
 *   Varable to put the id value.
 * @return
 *   -1 on error or 0 on success
 */
static inline int
plugin_get_id(struct plugin *pin, uint32_t *id)
{
	if (!pin || !id)
		return -1;

	*id = pin->hash;

	return 0;
}

/**
 * Get the version word from the plugin information
 *
 * @param inst
 *    The instance index value
 * @return
 *    The 32bit unsigned version number of the plugin
 * @return
 *    -1 is error and 0 on success
 */
static inline uint32_t
plugin_get_version(int inst)
{
	struct plugin *pin = plugin_get(inst);

	if (!pin || !pin->info)
		return -1;

	return pin->info->version;
}

/**
 * Get the struct plugin_info structure suppled by the plugin.
 *
 * @param inst
 *    The instance index value
 * @return
 *   NULL on error or pointer to info structure.
 */
static inline struct plugin_info *
plugin_get_info(int inst)
{
	struct plugin *pin = plugin_get(inst);

	if (!pin)
		return NULL;
	return pin->info;
}

/**
 * Return a pointer to a global symbol in the plugin code.
 *
 * @param ping
 *   The instance of plugin structure
 * @param symbol_name
 *   The symbol string to search for in the plugin.
 * @param ret
 *   Place to put the symbol address if found.
 * @return
 *   -1 on error or 0 if success and ret contains the address.
 */
int plugin_get_symbol(struct plugin *pin, const char *symbol_name, char **ret);

#ifdef __cplusplus
}
#endif

#endif /* _PLUGIN_H_ */
