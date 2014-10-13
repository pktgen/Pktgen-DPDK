/*
 ** $Id: lua.c,v 1.205 2012/05/23 15:37:09 roberto Exp $
 ** Lua stand-alone interpreter
 ** See Copyright Notice in lua.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define lua_c

#include "lua.h"
#include "lstate.h"

#include "lauxlib.h"
#include "lualib.h"
#include "lua-socket.h"
#include "lua_shell.h"

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua-shell"
#endif

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		1024
#endif

#if !defined(LUA_INIT)
#define LUA_INIT		"LUA_INIT"
#endif

#define LUA_INITVERSION  \
	LUA_INIT "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

/*
 ** lua_readline defines how to show a prompt and then read a line from
 ** the standard input.
 ** lua_saveline defines how to "save" a read line in a "history".
 ** lua_freeline defines how to free a line read by lua_readline.
 */
#if defined(LUA_USE_READLINE)

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#define lua_readline(L,b)	((void)L, ((b)=readline(NULL)) != NULL)
#define lua_saveline(L,idx) \
        if (lua_rawlen(L,idx) > 0)  /* non-empty line? */ \
          add_history(lua_tostring(L, idx));  /* add it to history */
#define lua_freeline(L,b)	((void)L, free(b))

#elif !defined(lua_readline)

#define lua_readline(L,b) \
        ((void)L, fgets(b, LUA_MAXINPUT, _get_stdin(L)) != NULL)  /* get line */
#define lua_saveline(L,idx)	{ (void)L; (void)idx; }
#define lua_freeline(L,b)	{ (void)L; (void)b; }

#endif

static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;

static void lstop(lua_State *L, lua_Debug *ar) {
	(void) ar; /* unused arg. */
	lua_sethook(L, NULL, 0, 0);
	luaL_error(L, "interrupted!");
}

static void laction(int i) {
	signal(i, SIG_DFL ); /* if another SIGINT happens before lstop,
	 terminate process (default action) */
	lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static void l_message(lua_State * L, const char *pname, const char *msg) {
	if (pname)
		luai_writestringerror(L, "%s: ", pname);
	luai_writestringerror(L, "%s\n", msg);
}

static int report(lua_State *L, int status) {
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
static void finalreport(lua_State *L, int status) {
	if (status != LUA_OK) {
		const char *msg =
				(lua_type(L, -1) == LUA_TSTRING) ? lua_tostring(L, -1) : NULL;
		if (msg == NULL )
			msg = "(error object is not a string)";
		l_message(L, progname, msg);
		lua_pop(L, 1);
	}
}

static int traceback(lua_State *L) {
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else if (!lua_isnoneornil(L, 1)) { /* is there an error object? */
		if (!luaL_callmeta(L, 1, "__tostring")) /* try its 'tostring' metamethod */
			lua_pushliteral(L, "(no error message)");
	}
	return 1;
}

static int docall(lua_State *L, int narg, int nres) {
	int status;
	int base = lua_gettop(L) - narg; /* function index */

	lua_pushcfunction(L, traceback);
	/* push traceback function */
	lua_insert(L, base); /* put it under chunk and args */
	globalL = L; /* to be available to 'laction' */
	signal(SIGINT, laction);
	status = lua_pcall(L, narg, nres, base);
	signal(SIGINT, SIG_DFL );
	globalL = NULL;
	lua_remove(L, base); /* remove traceback function */

	return status;
}

static int dofile(lua_State *L, const char *name) {
	int status = luaL_loadfile(L, name);
	if (status == LUA_OK)
		status = docall(L, 0, 0);
	return report(L, status);
}

static int dostring(lua_State *L, const char *s, const char *name) {
	int status = luaL_loadbuffer(L, s, strlen(s), name);
	if (status == LUA_OK)
		status = docall(L, 0, 0);
	return report(L, status);
}

int dolibrary(lua_State *L, const char *name) {
	int status;
	lua_getglobal(L, "require");
	lua_pushstring(L, name);
	status = docall(L, 1, 1); /* call 'require(name)' */
	if (status == LUA_OK)
		lua_setglobal(L, name); /* global[name] = require return */
	return report(L, status);
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)

static int incomplete(lua_State *L, int status) {
	if (status == LUA_ERRSYNTAX) {
		size_t lmsg;
		const char *msg = lua_tolstring(L, -1, &lmsg);
		if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0) {
			lua_pop(L, 1);
			return 1;
		}
	}
	return 0; /* else... */
}

static int pushline(lua_State *L, int firstline) {
	char buffer[LUA_MAXINPUT];
	char *b = buffer;
	size_t l;

	if (lua_readline(L, b) == 0)
		return 0; 								/* no input */

	l = strlen(b);
	if (l > 0 && b[l - 1] == '\n')				/* line ends with newline? */
		b[l - 1] = '\0';						/* remove it */
	if (firstline && b[0] == '=')							/* first line starts with `=' ? */
		lua_pushfstring(L, "return %s", b + 1); /* change it to `return' */
	else
		lua_pushstring(L, b);
	lua_freeline(L, b);

	return 1;
}

static int loadline(lua_State *L) {
	int status;

	lua_settop(L, 0);
	if (!pushline(L, 1))
		return -1; /* no input */

	for (;;) { /* repeat until gets a complete line */
		size_t l;
		const char *line = lua_tolstring(L, 1, &l);

		status = luaL_loadbuffer(L, line, l, "=stdin");
		if (!incomplete(L, status))
			break; 					/* cannot try to add lines? */

		if (!pushline(L, 0)) 		/* no more input? */
			return -1;

		lua_pushliteral(L, "\n");	/* add a new line... */
		lua_insert(L, -2);			/* ...between the two lines */
		lua_concat(L, 3);			/* join them */
	}
	lua_saveline(L, 1);
	lua_remove(L, 1);				/* remove line */
	return status;
}

static void dotty(lua_State *L) {
	int status;
	const char *oldprogname = progname;
	progname = NULL;

	while ((status = loadline(L)) != -1) {
		if (status == LUA_OK)
			status = docall(L, 0, LUA_MULTRET);
		report(L, status);
		if (status == LUA_OK && lua_gettop(L) > 0) { /* any result to print? */
			luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
			lua_getglobal(L, "print");
			lua_insert(L, 1);
			if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != LUA_OK)
				l_message(L, progname,
						lua_pushfstring(L,
								"error calling " LUA_QL("print") " (%s)",
								lua_tostring(L, -1)));
		}
	}
	lua_settop(L, 0); /* clear stack */
	luai_writeline(L);

	progname = oldprogname;
}

static int handle_luainit(lua_State *L) {
	const char *name = "=" LUA_INITVERSION;
	const char *init = getenv(name + 1);
	if (init == NULL ) {
		name = "=" LUA_INIT;
		init = getenv(name + 1); /* try alternative name */
	}
	if (init == NULL )
		return LUA_OK;
	else if (init[0] == '@')
		return dofile(L, init + 1);
	else
		return dostring(L, init, name);
}

static newlib_t	newlibs[MAX_NEW_LIBS];
static int newlibs_idx = 0;

int lua_newlib_add(newlib_t n) {
	if ( newlibs_idx >= MAX_NEW_LIBS )
		return -1;
	newlibs[newlibs_idx++] = n;
	return 0;
}

void lua_newlibs_init(lua_State * L) {
	int		i;

	for(i = 0; i < newlibs_idx; i++)
		newlibs[i](L);

}

static int pmain(lua_State *L) {

	/* open standard libraries */
	luaL_checkversion(L);

	lua_newlibs_init(L);

	if (handle_luainit(L) != LUA_OK)
		return 0; /* error running LUA_INIT */

	_set_stdfiles(L, luaL_getprivate(L));

	dotty(L);

	_reset_stdfiles(L);

	lua_pushboolean(L, 1); /* signal no errors */

	return 1;
}

int lua_shell(void * pServer) {
	int status, result;
	lua_State *L;

	L = luaL_newstate(); /* create state */
	if (L == NULL ) {
		l_message(NULL, "Lua-Shell", "cannot create state: not enough memory");
		return EXIT_FAILURE;
	}
	luaL_setprivate(L, pServer);

	/* call 'pmain' in protected mode */
	lua_pushcfunction(L, &pmain);
	status = lua_pcall(L, 0, 1, 0);
	result = lua_toboolean(L, -1);	/* get result */
	finalreport(L, status);
	lua_close(L);

	return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**************************************************************************//**
*
* execute_lua_string - Execute a Lua script buffer.
*
* DESCRIPTION
* Execute a Lua script buffer or chunk of Lua p-code.
*
* RETURNS: o if OK and -1 if error.
*
* SEE ALSO:
*/

int
execute_lua_string( lua_State * L, char * buffer )
{
    if ( (buffer == NULL) || (L == NULL) )
        return -1;

	if ( luaL_dostring(L, buffer) != 0 ) {
		fprintf(stderr,"%s\n", lua_tostring(L,-1));
		return -1;
	}

	return 0;
}

/**************************************************************************//**
*
* execute_lua_close - Close the Lua instance and free the resources.
*
* DESCRIPTION
* Close the Lua instance and free the resources.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
execute_lua_close(lua_State * L)
{
	if ( L )
		lua_close(L);
}

