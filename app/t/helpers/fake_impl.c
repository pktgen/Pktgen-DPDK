#ifndef _FAKE_IMPL_H
#define _FAKE_IMPL_H

/* Somewhat functional implementations of various functions for test purposes.
 * These implementations are not fully functional, but they allow for example
 * for tests to run without having to initialize the complete runtime environ-
 * ment.
 * The #define's are set through helpers/makedeps, depending on which header
 * files appear in a STUB directive in the test source.
 */


#ifdef STUB_RTE_MALLOC_H

#include <stdlib.h>
#include <string.h>
void * rte_zmalloc (const char *type, size_t size, unsigned align)
{
	char *mem = malloc(size);
	memset(mem, 0, size);

	return mem;
}

#endif


#ifdef STUB_RTE_DEBUG_H

#include <stdio.h>
void __rte_panic(const char *funcname , const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	fprintf(stderr, "PANIC: %s(): ", funcname);
	fprintf(stderr, format, ap);
	va_end(ap);
}

#endif


#ifdef STUB_CMDLINE_H

#include <cmdline_parse.h>
struct cmdline_token_ops cmdline_token_string_ops;
struct cmdline_token_ops cmdline_token_portlist_ops;
struct cmdline_token_ops cmdline_token_num_ops;
struct cmdline_token_ops cmdline_token_etheraddr_ops;
struct cmdline_token_ops cmdline_token_ipaddr_ops;

#endif


#ifdef STUB_LUA_SOCKET_H

#include <lua.h>
void execute_lua_close(lua_State * L) {
	return;
}

#endif


#ifdef STUB_RTE_LCORE_H

#include <rte_lcore.h>
struct lcore_config lcore_config[RTE_MAX_LCORE];

#endif


#ifdef STUB_RTE_SCRN_H

#include <wr_scrn.h>
wr_scrn_t *scrn;

#endif


#endif  // _FAKE_IMPL_H
