/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include "pktgen-main.h"

#include "pktgen.h"
#include "lpktgenlib.h"
#include "lua_shell.h"
#include "lua-socket.h"
#include "pktgen-cmds.h"
#include "pktgen-cpu.h"
#include "commands.h"
#include "pktgen-display.h"
#include "pktgen-log.h"

/* Defined in examples/pktgen/lib/lua/lua_shell.c */
extern void execute_lua_close(lua_State * L);


/**************************************************************************//**
*
* pktgen_l2p_dump - Dump the l2p table
*
* DESCRIPTION
* Dump the l2p table
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_l2p_dump(void)
{
	wr_raw_dump_l2p(pktgen.l2p);
}

#include <poll.h>

/**************************************************************************//**
*
* pktgen_interact - Main interaction routine for command line and display
*
* DESCRIPTION
* Process keyboard input and is called from the main command line loop.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_interact(struct cmdline *cl)
{
	char c;
	struct pollfd	fds;
	uint64_t		curr_tsc;
	uint64_t		next_poll;
	uint64_t		reload;

	fds.fd		= cl->s_in;
	fds.events	= POLLIN;
	fds.revents	= 0;

	c = -1;
	reload = (pktgen.hz/1000);
	next_poll = rte_rdtsc() + reload;

	for(;;) {
		rte_timer_manage();
		curr_tsc = rte_rdtsc();
		if ( unlikely(curr_tsc >= next_poll)  ) {
			next_poll = curr_tsc + reload;
			if ( poll(&fds, 1, 0) ) {
				if ( (fds.revents & (POLLERR | POLLNVAL | POLLHUP)) )
					break;
				if ( (fds.revents & POLLIN) ) {
					if (read(cl->s_in, &c, 1) < 0)
						break;
					if (cmdline_in(cl, &c, 1) < 0)
						break;
				}
			}
		}
	}
}

/**************************************************************************//**
*
* pktgen_get_lua - Get Lua state pointer.
*
* DESCRIPTION
* Get the Lua state pointer value.
*
* RETURNS: Lua pointer
*
* SEE ALSO:
*/

void * pktgen_get_lua(void)
{
	return pktgen.L;
}

/**************************************************************************//**
*
* pktgen_usage - Display the help for the command line.
*
* DESCRIPTION
* Display the help message for the command line.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
pktgen_usage(const char *prgname)
{
    printf("Usage: %s [EAL options] -- [-h] [-P] [-G] [-T] [-f cmd_file] [-l log_file] [-s P:PCAP_file] [-m <string>]\n"
           "  -s P:file    PCAP packet stream file, 'P' is the port number\n"
           "  -f filename  Command file (.pkt) to execute or a Lua script (.lua) file\n"
           "  -l filename  Write log to filename\n"
           "  -P           Enable PROMISCUOUS mode on all ports\n"
    	   "  -g address   Optional IP address and port number default is (localhost:0x5606)\n"
    	   "               If -g is used that enable socket support as a server application\n"
    	   "  -G           Enable socket support using default server values localhost:0x5606 \n"
    	   "  -N           Enable NUMA support\n"
           "  -T           Enable the color output\n"
           "  -m <string>  matrix for mapping ports to logical cores\n"
    	   "      BNF: (or kind of BNF)\n"
    	   "      <matrix-string>   := \"\"\" <lcore-port> { \",\" <lcore-port>} \"\"\"\n"
    	   "      <lcore-port>      := <lcore-list> \".\" <port-list>\n"
    	   "      <lcore-list>      := \"[\" <rx-list> \":\" <tx-list> \"]\"\n"
    	   "      <port-list>       := \"[\" <rx-list> \":\" <tx-list>\"]\"\n"
    	   "      <rx-list>         := <num> { \"/\" (<num> | <list>) }\n"
    	   "      <tx-list>         := <num> { \"/\" (<num> | <list>) }\n"
    	   "      <list>            := <num> { \"/\" (<range> | <list>) }\n"
    	   "      <range>           := <num> \"-\" <num> { \"/\" <range> }\n"
    	   "      <num>             := <digit>+\n"
    	   "      <digit>           := 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9\n"
    	   "      1.0, 2.1, 3.2                 - core 1 handles port 0 rx/tx,\n"
    	   "                                      core 2 handles port 1 rx/tx\n"
    	   "                                      core 3 handles port 2 rx/tx\n"
    	   "      1.[0-2], 2.3, ...             - core 1 handle ports 0,1,2 rx/tx,\n"
    	   "                                      core 2 handle port 3 rx/tx\n"
    	   "      [0-1].0, [2/4-5].1, ...       - cores 0-1 handle port 0 rx/tx,\n"
    	   "                                      cores 2,4,5 handle port 1 rx/tx\n"
    	   "      [1:2].0, [4:6].1, ...         - core 1 handles port 0 rx,\n"
    	   "                                      core 2 handles port 0 tx,\n"
    	   "      [1:2].[0-1], [4:6].[2/3], ... - core 1 handles port 0 & 1 rx,\n"
    	   "                                      core 2 handles port  0 & 1 tx\n"
    	   "      [1:2-3].0, [4:5-6].1, ...     - core 1 handles port 0 rx, cores 2,3 handle port 0 tx\n"
    	   "                                      core 4 handles port 1 rx & core 5,6 handles port 1 tx\n"
    	   "      [1-2:3].0, [4-5:6].1, ...     - core 1,2 handles port 0 rx, core 3 handles port 0 tx\n"
    	   "                                      core 4,5 handles port 1 rx & core 6 handles port 1 tx\n"
    	   "      [1-2:3-5].0, [4-5:6/8].1, ... - core 1,2 handles port 0 rx, core 3,4,5 handles port 0 tx\n"
    	   "                                      core 4,5 handles port 1 rx & core 6,8 handles port 1 tx\n"
    	   "      [1:2].[0:0-7], [3:4].[1:0-7], - core 1 handles port 0 rx, core 2 handles ports 0-7 tx\n"
    	   "                                      core 3 handles port 1 rx & core 4 handles port 0-7 tx\n"
    	   "      BTW: you can use \"{}\" instead of \"[]\" as it does not matter to the syntax.\n"
           "  -h           Display the help information\n",
           prgname);
}

/**************************************************************************//**
*
* pktgen_parse_args - Main parsing routine for the command line.
*
* DESCRIPTION
* Main parsing routine for the command line.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int
pktgen_parse_args(int argc, char **argv)
{
    int opt, ret, port;
    char **argvopt;
    int option_index;
    char *prgname = argv[0], * p;
    static struct option lgopts[] = {
        {NULL, 0, 0, 0}
    };

    argvopt = argv;

	pktgen.hostname		= (char *)strdupf(pktgen.hostname, "localhost");
	pktgen.socket_port	= 0x5606;

    pktgen.argc = argc;
    for (opt = 0; opt < argc; opt++)
    	pktgen.argv[opt] = strdup(argv[opt]);

    while ((opt = getopt_long(argc, argvopt, "p:m:f:l:s:g:hPNGT",
                  lgopts, &option_index)) != EOF) {
        switch (opt) {
		case 'p':
			// Port mask not used anymore
			break;
        case 'f':			// Command file or Lua script.
            pktgen.cmd_filename = strdup(optarg);
            break;

        case 'l':           // Log file
            pktgen_log_set_file(optarg);
            break;

        case 'm':			// Matrix for port mapping.
            if ( wr_parse_matrix(pktgen.l2p, optarg) == -1 ) {
                pktgen_log_error("invalid matrix string (%s)", optarg);
                pktgen_usage(prgname);
                return -1;
            }
            break;

        case 's':           // Read a PCAP packet capture file (stream)
        	port = strtol(optarg, NULL, 10);
        	p = strchr(optarg, ':');
            if( (p == NULL) || (pktgen.info[port].pcap = wr_pcap_open(++p, port)) == NULL ) {
                pktgen_log_error("Invalid PCAP filename (%s) must include port number as P:filename", optarg);
                pktgen_usage(prgname);
                return -1;
            }
            break;

        case 'P':			// Enable promiscuous mode on the ports
            pktgen.flags    |= PROMISCUOUS_ON_FLAG;
            break;

        case 'N':			// Enable NUMA support.
        	pktgen.flags	|= NUMA_SUPPORT_FLAG;
        	break;

        case 'G':
        	pktgen.flags	|= (ENABLE_GUI_FLAG | IS_SERVER_FLAG);
        	break;

        case 'g':			// Define the port number and IP address used for the socket connection.
        	pktgen.flags	|= (ENABLE_GUI_FLAG | IS_SERVER_FLAG);

        	p = strchr(optarg, ':');
        	if ( p == NULL )		// No : symbol means pktgen is a server application.
				pktgen.hostname	= (char *)strdupf(pktgen.hostname, optarg);
        	else {
        		char	c = *p;

        		*p = '\0';
    			if ( p != optarg )
					pktgen.hostname = (char *)strdupf(pktgen.hostname, optarg);

	    		pktgen.socket_port = strtol(++p, NULL, 0);
				pktgen_log_info(">>> Socket GUI support %s%c0x%x", pktgen.hostname, c, pktgen.socket_port);
        	}
        	break;

		case 'T':
			pktgen.flags	|= ENABLE_THEME_FLAG;
			break;

        case 'h':			// print out the help message
            pktgen_usage(prgname);
            return -1;

        case 0:				// Long options
        default:
            pktgen_usage(prgname);
            return -1;
        }
    }

    // Setup the program name
    if (optind >= 0)
        argv[optind-1] = prgname;

    ret = optind-1;
    optind = 0;				/* reset getopt lib */
    return ret;
}

/**************************************************************************//**
*
* main - Main routine to setup pktgen.
*
* DESCRIPTION
* Main routine to setup pktgen.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

int
main(int argc, char **argv)
{
    uint32_t i;
    int32_t	ret;

	wr_scrn_setw(1);			// Reset the window size

    // call before the rte_eal_init()
    (void)rte_set_application_usage_hook(pktgen_usage);

    memset(&pktgen, 0, sizeof(pktgen));

    pktgen.flags            = PRINT_LABELS_FLAG;
    pktgen.ident            = 0x1234;
    pktgen.nb_rxd           = DEFAULT_RX_DESC;
    pktgen.nb_txd           = DEFAULT_TX_DESC;
    pktgen.nb_ports_per_page= DEFAULT_PORTS_PER_PAGE;

    wr_print_copyright(PKTGEN_APP_NAME, PKTGEN_CREATED_BY);

    if ( (pktgen.l2p = wr_l2p_create()) == NULL )
		pktgen_log_panic("Unable to create l2p");

    pktgen.portdesc_cnt = wr_get_portdesc(pktgen.portlist, pktgen.portdesc, RTE_MAX_ETHPORTS, 0);

    /* Initialize the screen and logging */
    pktgen_init_log();
	pktgen_cpu_init();

    /* initialize EAL */
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        return -1;
    argc -= ret;
    argv += ret;

    pktgen.hz = rte_get_timer_hz();		// Get the starting HZ value.

    rte_delay_ms(100);      // Wait a bit for things to settle.

    /* parse application arguments (after the EAL ones) */
    ret = pktgen_parse_args(argc, argv);
    if (ret < 0)
        return -1;

    pktgen_init_screen((pktgen.flags & ENABLE_THEME_FLAG) ? THEME_ON : THEME_OFF);

    lua_newlib_add(_lua_openlib);

    // Open the Lua script handler.
    if ( (pktgen.L = lua_create_instance()) == NULL ) {
		pktgen_log_error("Failed to open Lua pktgen support library");
    	return -1;
	}

    pktgen_log_info(">>> Packet Burst %d, RX Desc %d, TX Desc %d, mbufs/port %d, mbuf cache %d",
    		DEFAULT_PKT_BURST, DEFAULT_RX_DESC,	DEFAULT_TX_DESC, MAX_MBUFS_PER_PORT, MBUF_CACHE_SIZE);

    // Configure and initialize the ports
    pktgen_config_ports();

    pktgen_log_info("");
    pktgen_log_info("=== Display processing on lcore %d", rte_lcore_id());

    /* launch per-lcore init on every lcore except master and master + 1 lcores */
    for (i = 0; i < RTE_MAX_LCORE; i++ ) {
    	if ( (i == rte_get_master_lcore()) || !rte_lcore_is_enabled(i) )
    		continue;
        ret = rte_eal_remote_launch(pktgen_launch_one_lcore, NULL, i);
        if ( ret != 0 )
            pktgen_log_error("Failed to start lcore %d, return %d", i, ret);
    }
    rte_delay_ms(1000);				// Wait for the lcores to start up.

	// Disable printing log messages of level info and below to screen,
    // erase the screen and start updating the screen again.
    pktgen_log_set_screen_level(LOG_LEVEL_WARNING);
	wr_scrn_erase(pktgen.scrn->nrows);

	wr_logo(3, 16, PKTGEN_APP_NAME);
	wr_splash_screen(3, 16, PKTGEN_APP_NAME, PKTGEN_CREATED_BY);

    wr_scrn_resume();

    pktgen_redisplay(1);

    rte_timer_setup();

    if ( pktgen.flags & ENABLE_GUI_FLAG ) {
        if ( !wr_scrn_is_paused() ) {
            wr_scrn_pause();
            wr_scrn_cls();
            wr_scrn_setw(1);
            wr_scrn_pos(pktgen.scrn->nrows, 1);
        }

		lua_init_socket(pktgen.L, &pktgen.thread, pktgen.hostname, pktgen.socket_port);
    }

    pktgen_cmdline_start();

    execute_lua_close(pktgen.L);
	pktgen_stop_running();

    wr_scrn_pause();

	wr_scrn_setw(1);
	wr_scrn_printf(100, 1, "\n");      // Put the cursor on the last row and do a newline.

    // Wait for all of the cores to stop running and exit.
    rte_eal_mp_wait_lcore();

    return 0;
}

/***********************************************************************//**
*
* pktgen_stop_running - Stop pktgen to exit in a clean way
*
* DESCRIPTION
* Stop all of the logical core threads to stop pktgen cleanly.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_stop_running(void)
{
	uint8_t		lid;

	for(lid = 0; lid < RTE_MAX_LCORE; lid++)
		wr_stop_lcore(pktgen.l2p, lid);
}

