/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <execinfo.h>
#include <signal.h>
#include <locale.h>

#include <lua_config.h>
#ifdef LUA_ENABLED
#include <lua_socket.h>
#endif
#include <pg_delay.h>

#include "pktgen-main.h"

#include "pktgen.h"
#ifdef LUA_ENABLED
#include "lpktgenlib.h"
#include "lauxlib.h"
#endif
#include "pktgen-cmds.h"
#include "pktgen-cpu.h"
#include "pktgen-display.h"
#include "pktgen-log.h"
#include "cli-functions.h"

#ifdef LUA_ENABLED
/**
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

void *
pktgen_get_lua(void)
{
    return pktgen.ld->L;
}
#endif

/**
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
    printf("Usage: %s [EAL options] -- [-h] [-v] [-P] [-G] [-g host:port] [-T] [-f cmd_file] [-l "
           "log_file] [-s P:filepath] [-m <string>]\n"
#ifdef LUA_ENABLED
           "  -f filename   Command file (.pkt) to execute or a Lua script (.lua) file\n"
#else
           "  -f filename   Command file (.pkt) to execute\n"
#endif
           "  -l filename   Write log to filename\n"
           "  -s P:filepath PCAP packet stream file, 'P' is the port number\n"
           "  -P            Enable PROMISCUOUS mode on all ports\n"
           "  -g address    Optional IP address and port number default is (localhost:0x5606)\n"
           "                If -g is used that enable socket support as a server application\n"
           "  -G            Enable socket support using default server values localhost:0x5606 \n"
           "  -N            Enable NUMA support\n"
           "  -T            Enable the color output\n"
           "  -v            Verbose output\n"
           "  -j            Enable jumbo frames of 9600 bytes\n"
           "  -c            Enable clock_gettime\n"
           "  --txd=N       set the number of descriptors in Tx rings to N \n"
           "  --rxd=N       set the number of descriptors in Rx rings to N \n"
           "  -m <string>   matrix for mapping ports to logical cores\n"
           "      BNF: (or kind of BNF)\n"
           "      <matrix-string>   := \"\"\" <lcore-port> { \",\" <lcore-port>} \"\"\"\n"
           "      <lcore-port>      := <lcore-list> \".\" <port>\n"
           "      <lcore-list>      := \"[\" <rx-list> \":\" <tx-list> \"]\"\n"
           "      <port>            := \"<num>\"\n"
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
           "      [1:2-3].0, [4:5-6].1, ...     - core 1 handles port 0 rx, cores 2,3 handle port "
           "0 tx\n"
           "                                      core 4 handles port 1 rx & core 5,6 handles port "
           "1 tx\n"
           "      [1-2:3].0, [4-5:6].1, ...     - core 1,2 handles port 0 rx, core 3 handles port "
           "0 tx\n"
           "                                      core 4,5 handles port 1 rx & core 6 handles port "
           "1 tx\n"
           "      [1-2:3-5].0, [4-5:6/8].1, ... - core 1,2 handles port 0 rx, core 3,4,5 handles "
           "port 0 tx\n"
           "                                      core 4,5 handles port 1 rx & core 6,8 handles "
           "port 1 tx\n"
           "      BTW: you can use \"{}\" instead of \"[]\" as it does not matter to the syntax.\n"
           "  -h           Display the help information\n",
           prgname);
}

/**
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
    int opt, ret;
    char **argvopt;
    int option_index;
    char *prgname   = argv[0], *p;
    char buff[1024] = {0};
    // clang-format off
    static struct option lgopts[] = {
        {"txd", required_argument, 0, 't'},
        {"rxd", required_argument, 0, 'r'},
        {NULL, 0, 0, 0}
    };
    // clang-format on
    uint16_t pid;

    argvopt = argv;

    pktgen.hostname    = (char *)strdupf(pktgen.hostname, "localhost");
    pktgen.socket_port = 0x5606;

    pktgen.argc = argc;
    for (opt = 0; opt < argc; opt++)
        pktgen.argv[opt] = strdup(argv[opt]);

    pktgen.mbuf_dataroom = RTE_MBUF_DEFAULT_DATAROOM;
    pktgen.mbuf_headroom = RTE_PKTMBUF_HEADROOM;
    pktgen.mbuf_buf_size = pktgen.mbuf_dataroom + pktgen.mbuf_headroom;

    pktgen.verbose = 0;
    while ((opt = getopt_long(argc, argvopt, "p:m:f:l:s:g:hPNGTvjtrc", lgopts, &option_index)) !=
           EOF) {
        switch (opt) {
        case 't':
            pktgen.nb_txd = atoi(optarg);
            pktgen_log_info(">>> Tx Descriptor set to %d", pktgen.nb_txd);
            break;

        case 'r':
            pktgen.nb_rxd = atoi(optarg);
            pktgen_log_info(">>> Rx Descriptor set to %d", pktgen.nb_rxd);
            break;

        case 'j':
            pktgen.flags |= JUMBO_PKTS_FLAG;
            pktgen.mbuf_dataroom = PG_JUMBO_DATAROOM_SIZE;
            pktgen.mbuf_buf_size = pktgen.mbuf_dataroom + pktgen.mbuf_headroom;

            pktgen_log_info("**** Jumbo Frames of %'d enabled.", RTE_ETHER_MAX_JUMBO_FRAME_LEN);
            break;

        case 'p':
            /* Port mask not used anymore */
            break;

        case 'f': /* Command file or Lua script. */
            cli_add_cmdfile(optarg);
            break;

        case 'l': /* Log file */
            pktgen_log_set_file(optarg);
            break;

        case 'm': /* Matrix for port mapping. */
            if (l2p_parse_mapping_add(optarg)) {
                pktgen_log_error("too many mapping entries");
                pktgen_usage(prgname);
                return -1;
            }
            break;

        case 's': /* Read a PCAP packet capture file (stream) */
            snprintf(buff, sizeof(buff), "%s", optarg);
            p = strchr(buff, ':');
            if (p == NULL)
                goto pcap_err;
            *p++ = '\0';

            pid = (uint16_t)strtol(buff, NULL, 10);

            if (pktgen_pcap_add(p, pid) < 0)
                goto pcap_err;
            break;
        case 'P': /* Enable promiscuous mode on the ports */
            pktgen.flags |= PROMISCUOUS_ON_FLAG;
            break;

        case 'N': /* Enable NUMA support. */
            pktgen.flags |= NUMA_SUPPORT_FLAG;
            break;

        case 'G':
            pktgen.flags |= IS_SERVER_FLAG;
            break;

        case 'g': /* Define the port number and IP address used for the socket connection. */
            pktgen.flags |= IS_SERVER_FLAG;

            p = strchr(optarg, ':');
            if (p == NULL) /* No : symbol means pktgen is a server application. */
                pktgen.hostname = (char *)strdupf(pktgen.hostname, optarg);
            else {
                char c = *p;

                *p = '\0';
                if (p != optarg)
                    pktgen.hostname = (char *)strdupf(pktgen.hostname, optarg);

                pktgen.socket_port = strtol(++p, NULL, 0);
                pktgen_log_info(">>> Socket support %s%c0x%x", pktgen.hostname, c,
                                pktgen.socket_port);
            }
            break;

        case 'T':
            pktgen.flags |= ENABLE_THEME_FLAG;
            break;
        case 'c':
            enable_clock_gettime(ENABLE_STATE);
            break;
        case 'v':
            pktgen.verbose = 1;
            break;

        case 'h': /* print out the help message */
            pktgen_usage(prgname);
            return -1;

        default:
            pktgen_usage(prgname);
            return -1;
        }
    }

    /* Setup the program name */
    if (optind >= 0)
        argv[optind - 1] = prgname;

    ret    = optind - 1;
    optind = 1; /* reset getopt lib */

    if (l2p_parse_mappings() < 0)
        pktgen_log_error("error or too many mapping entries");
    if (pktgen_pcap_open() < 0)
        pktgen_log_error("error opening PCAP files");

    return ret;

pcap_err:
    pktgen_log_error("Invalid PCAP filename (%s) must include port number as P:filename", optarg);
    pktgen_usage(prgname);
    return -1;
}

#define MAX_BACKTRACE 32

static void
sig_handler(int v __rte_unused)
{
    void *array[MAX_BACKTRACE];
    size_t size;
    char **strings;
    size_t i;

    scrn_setw(1);              /* Reset the window size, from possible crash run. */
    scrn_printf(100, 1, "\n"); /* Move the cursor to the bottom of the screen again */

    printf("\n======");

    if (v == SIGSEGV)
        printf(" Pktgen got a Segment Fault\n");
    else if (v == SIGHUP)
        printf(" Pktgen received a SIGHUP\n");
    else if (v == SIGINT)
        printf(" Pktgen received a SIGINT\n");
    else if (v == SIGPIPE) {
        printf(" Pktgen received a SIGPIPE\n");
        return;
    } else
        printf(" Pktgen received signal %d\n", v);

    printf("\n");

    size    = backtrace(array, MAX_BACKTRACE);
    strings = backtrace_symbols(array, size);

    printf("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
        printf("%s\n", strings[i]);

    free(strings);

    cli_destroy();
    scrn_destroy();

    exit(-1);
}

#ifdef LUA_ENABLED
static int
pktgen_lua_dofile(void *ld, const char *filename)
{
    int ret;

    ret = lua_dofile((luaData_t *)ld, filename);

    return ret;
}
#endif

/**
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
    int32_t ret;
    struct sigaction sa;
    sigset_t set;

    setlocale(LC_ALL, "");

    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);

    /* Block SIGWINCH for all threads,
     * because we only want it to be
     * handled by the main thread */
    sigemptyset(&set);
    sigaddset(&set, SIGWINCH);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    scrn_setw(1);     /* Reset the window size, from possible crash run. */
    scrn_pos(100, 1); /* Move the cursor to the bottom of the screen again */

    print_copyright(PKTGEN_VERSION, PKTGEN_VER_CREATED_BY);
    fflush(stdout);

    /* call before the rte_eal_init() */
    (void)rte_set_application_usage_hook(pktgen_usage);

    memset(&pktgen, 0, sizeof(pktgen));

    pktgen.flags             = PRINT_LABELS_FLAG;
    pktgen.ident             = 0x1234;
    pktgen.nb_rxd            = DEFAULT_RX_DESC;
    pktgen.nb_txd            = DEFAULT_TX_DESC;
    pktgen.nb_ports_per_page = DEFAULT_PORTS_PER_PAGE;

    l2p_create();

    pktgen.portdesc_cnt = get_portdesc(pktgen.portdesc, RTE_MAX_ETHPORTS, 0);

    /* Initialize the screen and logging */
    pktgen_init_log();
    pktgen_cpu_init();

    /* initialize EAL */
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        return -1;

    argc -= ret;
    argv += ret;

    if (pktgen_cli_create()) {
        cli_destroy();
        scrn_destroy();
        return -1;
    }

#ifdef LUA_ENABLED
    lua_newlib_add(pktgen_lua_openlib, 0);

    /* Open the Lua script handler. */
    if ((pktgen.ld = lua_create_instance()) == NULL) {
        pktgen_log_error("Failed to open Lua pktgen support library");
        return -1;
    }
    cli_set_lua_callback(pktgen_lua_dofile);
    cli_set_user_state(pktgen.ld);
#endif

    /* parse application arguments (after the EAL ones) */
    ret = pktgen_parse_args(argc, argv);
    if (ret < 0) {
        cli_destroy();
        scrn_destroy();
        return -1;
    }

    if (l2p_get_pid_by_lcore(rte_get_main_lcore()) < RTE_MAX_ETHPORTS) {
        cli_printf("*** Error can not use initial lcore %d for port handling\n",
                   rte_get_main_lcore());
        cli_printf("    The initial lcore is %d\n", rte_get_main_lcore());
        cli_destroy();
        scrn_destroy();
        exit(-1);
    }

    pktgen.hz = pktgen_get_timer_hz(); /* Get the starting HZ value. */

    scrn_create_with_defaults(pktgen.flags & ENABLE_THEME_FLAG);

    rte_delay_us_sleep(100 * 1000); /* Wait a bit for things to settle. */

    if (pktgen.verbose)
        pktgen_log_info(
            ">>> Packet Max Burst %d/%d, RX Desc %d, TX Desc %d, mbufs/port %d, mbuf cache %d",
            MAX_PKT_RX_BURST, MAX_PKT_TX_BURST, pktgen.nb_rxd, pktgen.nb_txd,
            MAX_MBUFS_PER_PORT(pktgen.nb_rxd, pktgen.nb_txd), MBUF_CACHE_SIZE);

    /* Configure and initialize the ports */
    pktgen_config_ports();

    if (pktgen.verbose) {
        pktgen_log_info("");
        pktgen_log_info("=== Display processing on lcore %d", rte_lcore_id());
    }

    if (workq_create())
        rte_exit(EXIT_FAILURE, "Cannot create work queues\n");

    /* launch per-lcore init on every lcore except initial and initial + 1 lcores */
    ret = rte_eal_mp_remote_launch(pktgen_launch_one_lcore, NULL, SKIP_MAIN);
    if (ret != 0)
        pktgen_log_error("Failed to start lcores, return %d", ret);

    for (uint16_t i = 0; i < 4; i++)
        printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    fflush(stdout);

    /* Disable printing log messages of level info and below to screen, */
    /* erase the screen and start updating the screen again. */
    pktgen_log_set_screen_level(LOG_LEVEL_WARNING);
    scrn_erase(this_scrn->nrows);

    scrn_resume();

    pktgen_clear_display();

    pktgen_timer_setup();

#ifdef LUA_ENABLED
    if (pktgen.flags & IS_SERVER_FLAG) {
        pktgen.ld_sock = lua_create_instance();
        if (pktgen.ld_sock == NULL) {
            pktgen_log_error("Failed to open Lua socket server support library");
            return -1;
        }

        if (lua_start_socket(pktgen.ld_sock, &pktgen.thread, pktgen.hostname, pktgen.socket_port) <
            0) {
            pktgen_log_error("Failed to start Lua socket server thread");
            return -1;
        }
    }
#endif

    /* Unblock SIGWINCH so main thread
     * can handle screen resizes */
    sigemptyset(&set);
    sigaddset(&set, SIGWINCH);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    /* execute the command files if present */
    scrn_pause();
    cli_execute_cmdfiles();
    scrn_resume();
    pktgen_clear_display();

    cli_start(NULL); /* Start accepting input from user */

    scrn_pause();
    scrn_setw(1); /* Reset the window size, from possible crash run. */
    /* Move the cursor to the bottom of the screen again */
    scrn_printf(this_scrn->nrows + 1, 1, "\n");

    pktgen_stop_running();

    /* Wait for all of the cores to stop running and exit. */
    rte_eal_mp_wait_lcore();

    int i = 0;
    RTE_ETH_FOREACH_DEV(i)
    {
        rte_eth_dev_stop(i);
        rte_delay_us_sleep(100 * 1000);
    }

    cli_destroy();
    scrn_destroy();
    workq_destroy();
    return 0;
}

/**
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
void
pktgen_stop_running(void)
{
#ifdef LUA_ENABLED
    lua_execute_close(pktgen.ld);
#endif

    pktgen.timer_running = 0;
    pktgen.force_quit    = 1;
}
