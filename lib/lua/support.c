/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <sys/queue.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>

#include <rte_config.h>
#include <rte_version.h>

#include <rte_log.h>
#include <rte_tailq.h>
#include <rte_common.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_timer.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include "lua_support.h"

static lua_State *globalL;

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua-shell"
#endif

#if !defined(LUA_INIT)
#define LUA_INIT		"LUA_INIT"
#endif

#define LUA_INITVERSION  \
	LUA_INIT "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

typedef luaL_Stream LStream;

#define tolstream(L)    ((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))
#define isclosed(p)     ((p)->closef == NULL)

static const char *progname = LUA_PROGNAME;

static newlib_t	newlibs[MAX_NEW_LIBS];
static int newlibs_idx = 0;

static char *
lua_readline(luaData_t *ld)
{
	*ld->buffer = '\0';
	return fgets(ld->buffer, LUA_BUFFER_SIZE, _get_stdin(ld));
}

static char *
rte_strtrim(char *str)
{
	if (!str || !*str)
		return str;

	/* trim white space characters at the front */
	while(isspace(*str))
		str++;

	/* Make sure the string is not empty */
	if (*str) {
		char *p = &str[strlen(str) - 1];

		/* trim trailing white space characters */
		while((p >= str) && isspace(*p))
			p--;

		p[1] = '\0';
	}
	return *str? str : NULL;
}

static int
traceback(lua_State *L)
{
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else if (!lua_isnoneornil(L, 1)) { /* is there an error object? */
		if (!luaL_callmeta(L, 1, "__tostring")) /* try its 'tostring' metamethod */
			lua_pushliteral(L, "(no error message)");
	}
	return 1;
}

static void
lstop(lua_State *L, lua_Debug *ar)
{
	(void) ar; /* unused arg. */
	lua_sethook(L, NULL, 0, 0);
	luaL_error(L, "interrupted!");
}

static void
laction(int i)
{
	signal(i, SIG_DFL ); /* if another SIGINT happens before lstop,
				terminate process (default action) */
	if (globalL)
		lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int
_k(lua_State *L __rte_unused, int status, lua_KContext ctx __rte_unused)
{
	(void)traceback;

	DBG("Entry\n");
	signal(SIGINT, SIG_DFL );
	globalL = NULL;

	DBG("Exit status %d\n", status);
	return status;
}

static int
docall(lua_State *L, int narg, int nres)
{
	int status;

	DBG("Entry narg %d, nres %d\n", narg, nres);
	globalL = L;				/* to be available to 'laction' */
	signal(SIGINT, laction);

	status = _k(L, lua_pcallk(L, narg, nres, 0, 0, _k), 0);

	DBG("Exit status %d\n", status);
	return status;
}

static int
server_startup(luaData_t *ld)
{
	char *err_msg = NULL;
	struct sockaddr_in ipaddr;
	struct hostent *pHost;
	int linger = 1;

	pthread_detach(pthread_self());

	err_msg = "gethostbyname failed";
	if( (pHost = gethostbyname(ld->hostname)) == NULL )
		goto error_exit;

	err_msg = "Socket create failed";
	if ( (ld->server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		goto error_exit;

	memset(&ipaddr, 0, sizeof(ipaddr));
	ipaddr.sin_family = AF_INET;
	ipaddr.sin_port = htons(ld->socket_port);
	ipaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	err_msg = "Setsockopt failed";
	if (setsockopt(ld->server_socket, SOL_SOCKET, SO_REUSEADDR, &linger,
	                sizeof(linger)) == -1)
		goto error_exit;

	err_msg = "Bind failed";
	if (bind(ld->server_socket, (struct sockaddr *) &ipaddr,
	                sizeof(ipaddr)) < 0)
		goto error_exit;

	err_msg = "Listen failed";
	if (listen(ld->server_socket, 5) < 0)
		goto error_exit;

	return 0;

error_exit:
	if (ld->server_socket != -1)
		close(ld->server_socket);
	ld->server_socket = -1;
	if (err_msg) {
		perror(err_msg);
		fflush(stdout);
	}
	return -1;
}

int
lua_newlib_add(newlib_t n)
{
	if ( newlibs_idx >= MAX_NEW_LIBS )
		return -1;

	newlibs[newlibs_idx++] = n;

	return 0;
}

void
lua_newlibs_init(luaData_t *ld)
{
	int i;

	for(i = 0; i < newlibs_idx; i++)
		newlibs[i](ld);
}

static void
_socket_open(luaData_t *ld)
{
	if (ld) {
		ld->in  = fdopen(ld->client_socket, "r");
		ld->out = fdopen(ld->client_socket, "w");
		ld->err = fdopen(ld->client_socket, "w");
	}
}

static void
_socket_close(luaData_t *ld)
{
	if (ld) {
		fclose(ld->in);
		fclose(ld->out);
		fclose(ld->err);
	}
}

void *
_get_stdout(luaData_t *ld)
{
	if (!ld || !ld->out)
		return stdout;

	return ld->out;
}

void *
_get_stdin(luaData_t *ld)
{
	if (!ld || !ld->in)
		return stdin;

	return ld->in;
}

void *
_get_stderr(luaData_t *ld)
{
	if (!ld || !ld->err)
		return stderr;
	return ld->err;
}

void
_set_stdfiles(luaData_t *ld)
{
	lua_set_stdfiles(ld);
	signal(SIGPIPE, SIG_IGN);
}

void
_reset_stdfiles(luaData_t *ld)
{
	signal(SIGPIPE, SIG_DFL);
	lua_reset_stdfiles(ld);
}

static char *
pushline(luaData_t *ld)
{
	lua_State *L = ld->L;
	char *b;

	DBG("Entry Stack top %d\n", lua_gettop(L));
	do {
		b = lua_readline(ld);
		if (!b)
			break;
		fprintf(stderr, ">>> %s", b);		/* String contains newline */
		b = rte_strtrim(b);

		/* skip blank lines or comments */
		if (!b || (b[0] == '-' && b[1] == '-'))
			continue;
		break;
	} while(1);
	DBG("Exit Stack top %d\n", lua_gettop(L));
	return b;
}

static int
loadline(luaData_t *ld)
{
	lua_State *L = ld->L;
	int status = 0;
	int firstline = 1;

	DBG("Entry Stack top %d\n", lua_gettop(L));
	do {
		char *line;
		size_t l;

		line = pushline(ld);
		if (!line)
			break;

		/* first line starts with `=' then change to 'return ' */
		if (firstline && line[0] == '=')
			snprintf(ld->buffer, LUA_BUFFER_SIZE, "return %s", line + 1);
		firstline = 0;
		l = strlen(line);

		DBG("Before luaL_loadbuffer Stack top %d len %lu\n", lua_gettop(L), l);
		status = luaL_loadbuffer(L, line, l, "=stdin");
		DBG("After luaL_loadbuffer Stack top %d status %d\n", lua_gettop(L), status);
		if (status != LUA_OK)
			traceback(L);
		break;
	} while(1);
	DBG("Exit Stack top %d\n", lua_gettop(L));
	return status;
}

static void
dotty(luaData_t *ld)
{
	lua_State *L = ld->L;
	int status;
	const char *oldprogname = progname;
	progname = NULL;

	DBG("Entry\n");
	while ((status = loadline(ld)) != -1) {
		if (status)
			break;
		DBG("Before lua_pcall Stack top %d\n", lua_gettop(L));
		status = lua_pcall(L, 0, LUA_MULTRET, 0);
		DBG("After lua_pcall Stack top %d status %d\n", lua_gettop(L), status);
		if (status) {
			DBG("%s\n", lua_tostring(L, -1));
			break;
		}
	}
	lua_writeline();

	progname = oldprogname;
	DBG("Exit %s\n", progname);
}

static void
handle_server_requests(luaData_t *ld)
{
	struct sockaddr_in ipaddr;
	socklen_t	len;

	DBG("ld %p\n", ld);
	ld->client_socket = -1;

	do {
		len = sizeof(struct sockaddr_in);
		DBG("Wait accept\n");
		if ( (ld->client_socket = accept(ld->server_socket,
		                                 (struct sockaddr *)&ipaddr, &len)) < 0) {
			perror("accept failed");
			break;
		}

		DBG("Accept found fd %d\n", ld->client_socket);
		if (ld->client_socket > 0) {
			DBG("Socket Open\n");
			_socket_open(ld);
			dotty(ld);
			_socket_close(ld);
			DBG("Socket Closed\n");

			close(ld->client_socket);
			ld->client_socket = -1;
		}
	} while(1);

	if (ld->server_socket > 0) {
		close(ld->server_socket);
		ld->server_socket = -1;
	}
}

static void *
lua_server(void *arg)
{

	if (server_startup((luaData_t *)arg))
		fprintf(stderr, "server_startup() failed!\n");

	handle_server_requests((luaData_t *)arg);

	return NULL;
}

int
lua_start_socket(luaData_t *ld, pthread_t *pthread, char *hostname, int port)
{
	int r;

	ld->client_socket = -1;
	ld->server_socket = -1;
	ld->socket_port = port;
	ld->hostname = strdup( (hostname) ? hostname : "localhost" );

	/* Split assert and function because using NDEBUG define will remove function */
	r = pthread_create(pthread, NULL, lua_server, ld);
	if (r)
		return -1;

	rte_thread_setname(*pthread, "pktgen-socket");

	return 0;
}

luaData_t *
lua_create_instance(void)
{
	luaData_t *ld;

	ld = (luaData_t *)malloc(sizeof(luaData_t));
	if (ld) {
		memset(ld, 0, sizeof(luaData_t));

		ld->client_socket = -1;
		ld->server_socket = -1;

		ld->buffer = malloc(LUA_BUFFER_SIZE);
		if (!ld->buffer) {
			free(ld);
			return NULL;
		}
		memset(ld->buffer, 0, LUA_BUFFER_SIZE);

		ld->L = luaL_newstate();
		if (!ld->L) {
			free(ld);
			return NULL;
		}

		lua_newlibs_init(ld);

		// Make sure we display the copyright string for Lua.
		lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
		lua_writeline();
	}

	return ld;
}

static void
l_message(lua_State *L, const char *pname, const char *msg)
{
	(void)L;
	if (pname)
		lua_writestringerror("%s: ", pname);
	lua_writestringerror("%s\n", msg);
}

static int
report(lua_State *L, int status)
{
	if (status != LUA_OK && !lua_isnil(L, -1)) {
		const char *msg = lua_tostring(L, -1);
		if (msg == NULL )
			msg = "(error object is not a string)";
		l_message(L, progname, msg);
		lua_pop(L, 1);
		/* force a complete garbage collection in case of errors */
		lua_gc(L, LUA_GCCOLLECT, 0);
	}
	return status;
}

/* the next function is called unprotected, so it must avoid errors */
static void
finalreport(lua_State *L, int status)
{
	if (status != LUA_OK) {
		const char *msg = (lua_type(L, -1) == LUA_TSTRING) ?
		                  lua_tostring(L, -1) : NULL;
		if (msg == NULL )
			msg = "(error object is not a string)";
		l_message(L, progname, msg);
		lua_pop(L, 1);
	}
}

int
lua_dofile(luaData_t *ld, const char *name)
{
	int status;

	DBG("Entry (%s)\n", name);
	status = luaL_loadfile(ld->L, name);

	DBG("Here 0 status %d\n", status);
	if (status == LUA_OK)
		status = docall(ld->L, 0, 0);

	DBG("Call report with status %d\n", status);
	return report(ld->L, status);
}

int
lua_dostring(luaData_t *ld, const char *s, const char *name)
{
	int status;

	DBG("s (%s), name (%s)\n", s, name);
	status = luaL_loadbuffer(ld->L, s, strlen(s), name);

	DBG("Here 0 status %d\n", status);
	if (status == LUA_OK)
		status = docall(ld->L, 0, 0);

	DBG("Call report with status %d\n", status);
	return report(ld->L, status);
}

int
lua_dolibrary(lua_State *L, const char *name)
{
	int status;

	DBG("Entry %s\n", name);

	lua_getglobal(L, "require");
	lua_pushstring(L, name);

	DBG("Here 0\n");
	status = docall(L, 1, 1); /* call 'require(name)' */
	if (status == LUA_OK)
		lua_setglobal(L, name); /* global[name] = require return */

	DBG("Call report with status %d\n", status);
	return report(L, status);
}

static int
handle_luainit(luaData_t *ld)
{
	const char *name;
	const char *init;

	name = "=" LUA_INITVERSION;
	init = getenv(&name[1]);
	DBG("init %s, name %s\n", init, name);
	if (!init) {
		name = "=" LUA_INIT;
		init = getenv(&name[1]); /* try alternative name */
	}

	if (!init)
		return LUA_OK;

	DBG("%s\n", init);
	if (init[0] == '@')
		return lua_dofile(ld, init + 1);
	else
		return lua_dostring(ld, init, name);
}

static int
pmain(lua_State *L)
{
	luaData_t *ld;

	DBG("Entry\n");
	if (!lua_isuserdata(L, 1)) {
		lua_putstring("*** Not Light User data\n");
		return -1;
	}
	ld = lua_touserdata(L, 1);

	DBG("Here 0 ld %p\n", ld);
	/* open standard libraries */
	luaL_checkversion(L);

	DBG("Here 1\n");
	if (handle_luainit(ld) != LUA_OK)
		return 0; /* error running LUA_INIT */

	_set_stdfiles(ld);

	dotty(ld);

	_reset_stdfiles(ld);

	lua_pushboolean(L, 1); /* signal no errors */

	return 1;
}

int
lua_shell(luaData_t *ld)
{
	int status;
	int result;
	lua_State *L = ld->L;

	DBG("Here 0 ls %p\n", ld);
	(void)finalreport;

	/* call 'pmain' in protected mode */
	lua_pushcfunction(L, &pmain);
	lua_pushlightuserdata(L, ld);

	DBG("Here 1 about to make docall()\n");
	status = docall(L, 1, 1);

	DBG("Here 2 status %d\n", status);
	result = lua_toboolean(L, -1);	/* get result */

	DBG("Here 3 result %d\n", result);
	return report(L, status);
}

int
execute_lua_string(luaData_t *ld, char *buffer )
{
	lua_State *L = ld->L;

	if (!L)
		return -1;

	buffer = rte_strtrim(buffer);
	if (!buffer)
		return -1;

	if ( luaL_dostring(L, buffer) != 0 ) {
		DBG("%s\n", lua_tostring(L,-1));
		return -1;
	}

	return 0;
}

void
execute_lua_close(luaData_t *ld)
{
	if ( ld->L )
		lua_close(ld->L);
}

void
lua_set_stdfiles(luaData_t *ld)
{
	lua_State *L = ld->L;

	luaL_getmetatable(L, LUA_FILEHANDLE);

	if (lua_isnil(L, -1)) {
		DBG("luaL_getmetatable() returned NIL\n");
		return;
	}

	/* create (and set) default files */
	create_stdfile(ld, ld->in, IO_INPUT, "stdin");
	create_stdfile(ld, ld->out, IO_OUTPUT, "stdout");
	create_stdfile(ld, ld->err, NULL, "stderr");
}

void
lua_reset_stdfiles(luaData_t *ld)
{
	lua_State *L = ld->L;

	luaL_getmetatable(L, LUA_FILEHANDLE);

	if (lua_isnil(L, -1))
		return;

	/* create (and set) default files */
	create_stdfile(ld, stdin, IO_INPUT, "stdin");
	create_stdfile(ld, stdout, IO_OUTPUT, "stdout");
	create_stdfile(ld, stderr, NULL, "stderr");
}

/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
static int
io_noclose (lua_State *L)
{
	LStream *p = tolstream(L);
	p->closef = &io_noclose;  /* keep file opened */
	lua_pushnil(L);
	lua_pushliteral(L, "cannot close standard file");
	return 2;
}

static LStream *
newprefile (lua_State *L)
{
	LStream *p = (LStream *)lua_newuserdata(L, sizeof(LStream));
	p->closef = NULL;  /* mark file handle as 'closed' */
	luaL_setmetatable(L, LUA_FILEHANDLE);
	return p;
}

void
create_stdfile (luaData_t *ld, FILE *f, const char *k, const char *fname)
{
	lua_State *L = ld->L;
	LStream *p = newprefile(L);

	p->f = f;
	p->closef = &io_noclose;
	if (k != NULL) {
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
	}
	lua_setfield(L, -2, fname);  /* add file to module */
}
