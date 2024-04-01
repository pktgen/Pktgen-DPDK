/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2023 Intel Corporation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <alloca.h>
#include <locale.h>
#include <pthread.h>

#include <pktperf.h>

#define BURST_COUNT_OPT "burst-count"
#define PKT_SIZE_OPT    "pkt-size"
#define TX_RATE_OPT     "tx-rate"
#define RX_TX_DESC_OPT  "descriptors"
#define MAPPING_OPT     "map"
#define TIMEOUT_OPT     "timeout"
#define PROMISCUOUS_OPT "no-promiscuous"
#define MBUF_COUNT_OPT  "mbuf-count"
#define FGEN_STRING_OPT "fgen"
#define FGEN_FILE_OPT   "fgen-file"
#define VERBOSE_OPT     "verbose"
#define HELP_OPT        "help"

// clang-format off
static const struct option lgopts[] = {
    {BURST_COUNT_OPT,	    1, 0, 'b'},
    {PKT_SIZE_OPT,	        1, 0, 's'},
    {TX_RATE_OPT,           1, 0, 'r'},
    {RX_TX_DESC_OPT,        1, 0, 'd'},
    {MAPPING_OPT,		    1, 0, 'm'},
	{TIMEOUT_OPT,		    1, 0, 'T'},
    {MBUF_COUNT_OPT,        1, 0, 'M'},
	{PROMISCUOUS_OPT,       0, 0, 'P'},
	{FGEN_STRING_OPT,       0, 0, 'f'},
	{FGEN_FILE_OPT,         0, 0, 'F'},
    {VERBOSE_OPT,           0, 0, 'v'},
	{HELP_OPT,			    0, 0, 'h'},
	{NULL,				    0, 0, 0}
};
// clang-format on

static const char *short_options = "t:b:s:r:d:m:T:M:F:f:Pvh";

/* display usage */
void
usage(const char *prgname)
{
    printf("%s [EAL options] -- [-b burst] [-s size] [-r rate] [-d rxd/txd] [-m map] [-T secs] "
           "[-P] [-M mbufs] [-v] [-h]\n"
           "\t-b|--burst-count <burst> Number of packets for Rx/Tx burst (default %d)\n"
           "\t-s|--pkt-size <size>     Packet size in bytes (default %'d) includes FCS bytes\n"
           "\t-r|--rate <rate>         Packet TX rate percentage 0=off (default %'d)\n"
           "\t-d|--descriptors <Rx/Tx> Number of RX/TX descriptors (default %'d/%'d)\n"
           "\t-m|--map <map>           Core to Port/queue mapping '[Rx-Cores:Tx-Cores].port'\n"
           "\t-T|--timeout <secs>      Timeout period in seconds (default %d second)\n"
           "\t-P|--no-promiscuous      Turn off promiscuous mode (default On)\n"
           "\t-M|--mbuf-count <count>  Number of mbufs to allocate (default %'d, max %'d)\n"
           "\t-f|--fgen <string>       FGEN string to load\n"
           "\t-F|--fgen-file <file>    FGEN file to load\n"
           "\t-v|--verbose             Verbose output\n"
           "\t-h|--help                Print this help\n",
           prgname, DEFAULT_BURST_COUNT, DEFAULT_PKT_SIZE, DEFAULT_TX_RATE, DEFAULT_RX_DESC,
           DEFAULT_TX_DESC, DEFAULT_TIMEOUT_PERIOD, DEFAULT_MBUF_COUNT, MAX_MBUF_COUNT);
}

static int
parse_cores(l2p_port_t *port, const char *cores, int mode)
{
    char *core_map = NULL;
    int num_cores  = 0, l, h, num_fields;
    char *fields[3];
    char name[64];

    core_map = alloca(MAX_ALLOCA_SIZE);
    if (!core_map)
        rte_exit(EXIT_FAILURE, "out of memory for core string\n");

    snprintf(core_map, MAX_ALLOCA_SIZE, "%s", cores);

    num_fields = rte_strsplit(core_map, strlen(core_map), fields, RTE_DIM(fields), '-');
    if (num_fields <= 0 || num_fields > 2)
        rte_exit(EXIT_FAILURE, "invalid core mapping '%s'\n", cores);
    DBG_PRINT("num_fields: %d from cores '%s'\n", num_fields, cores);

    if (num_fields == 1) {
        DBG_PRINT("single core specified: %s\n", fields[0]);
        l = h = strtol(fields[0], NULL, 10);
    } else if (num_fields == 2) {
        DBG_PRINT("two cores specified: %s - %s\n", fields[0], fields[1]);
        l = strtol(fields[0], NULL, 10);
        h = strtol(fields[1], NULL, 10);
    }

    DBG_PRINT("lcore: %d to %d\n", l, h);
    do {
        l2p_lport_t *lport;

        lport = info->lports[l];
        if (lport == NULL) {
            snprintf(name, sizeof(name), "lport-%u:%u", l, port->pid);
            lport = rte_zmalloc_socket(name, sizeof(l2p_lport_t), RTE_CACHE_LINE_SIZE,
                                       rte_eth_dev_socket_id(port->pid));
            if (!lport)
                rte_exit(EXIT_FAILURE, "Failed to allocate memory for lport info\n");
            lport->lid = l;

            info->lports[l] = lport;
        } else
            ERR_PRINT("lcore %u already in use\n", l);

        num_cores++;
        lport->port = port;
        lport->mode = mode;
        switch (mode) {
        case LCORE_MODE_RX:
            lport->rx_qid = port->num_rx_qids++;
            DBG_PRINT("lcore %u is in RX mode\n", l);
            break;
        case LCORE_MODE_TX:
            lport->tx_qid = port->num_tx_qids++;
            DBG_PRINT("lcore %u is in TX mode\n", l);
            break;
        case LCORE_MODE_BOTH:
            lport->rx_qid = port->num_rx_qids++;
            lport->tx_qid = port->num_tx_qids++;
            DBG_PRINT("lcore %u is in RX/TX mode\n", l);
            break;
        default:
            rte_exit(EXIT_FAILURE, "invalid port mode\n");
            break;
        }

        DBG_PRINT("lcore: %u port: %u qid: %u/%u name:'%s'\n", lport->lid, port->pid, lport->rx_qid,
                  lport->tx_qid, name);

        if (port->rx_mp == NULL) {
            printf("Creating Rx mbuf pool for lcore %3u, port %2u, MBUF Count %'u\n", l, port->pid,
                   info->mbuf_count);

            /* Create the Rx mbuf pool one per lcore/port */
            snprintf(name, sizeof(name), "rx-%u/%u", lport->lid, port->pid);
            port->rx_mp = rte_pktmbuf_pool_create(name, info->mbuf_count, MEMPOOL_CACHE_SIZE, 0,
                                                  RTE_MBUF_DEFAULT_BUF_SIZE,
                                                  rte_eth_dev_socket_id(port->pid));
            if (port->rx_mp == NULL)
                rte_exit(EXIT_FAILURE, "Can't initialize Rx mbuf pool for lcore/port/queues %s\n",
                         name);
        }
        if (port->tx_mp == NULL) {
            printf("Creating Tx mbuf pool for lcore %3u, port %2u, MBUF Count %'u\n", l, port->pid,
                   info->mbuf_count);

            /* Create the Tx mbuf pool pne per lcore/port/queue */
            snprintf(name, sizeof(name), "tx-%u/%u", lport->lid, port->pid);
            port->tx_mp = rte_pktmbuf_pool_create(name, info->mbuf_count, MEMPOOL_CACHE_SIZE, 0,
                                                  RTE_MBUF_DEFAULT_BUF_SIZE,
                                                  rte_eth_dev_socket_id(port->pid));
            if (port->tx_mp == NULL)
                rte_exit(EXIT_FAILURE, "Can't initialize Tx mbuf pool for lcore/port/queues %s\n",
                         name);
        }
    } while (l++ < h);

    DBG_PRINT("num_cores: %d\n", num_cores);
    return num_cores;
}

static void
parse_mapping(const char *map)
{
    char *fields[3], *lcores[3];
    char *mapping = NULL;
    int num_fields, num_cores, num_lcores;
    uint16_t pid;

    if (!map || strlen(map) == 0)
        rte_exit(EXIT_FAILURE, "no mapping specified or string empty\n");

    mapping = alloca(MAX_ALLOCA_SIZE);
    if (!mapping)
        rte_exit(EXIT_FAILURE, "unable to allocate map string\n");
    snprintf(mapping, MAX_ALLOCA_SIZE, "%s", map);
    DBG_PRINT("Mapping: '%s'\n", map);

    /* parse map into a lcore list and port number */
    num_fields = rte_strsplit(mapping, strlen(mapping), fields, RTE_DIM(fields), '.');
    if (num_fields != 2)
        rte_exit(EXIT_FAILURE, "Invalid mapping format '%s'\n", map);
    DBG_PRINT("Mapping: fields(%u) lcore '%s', port '%s'\n", num_fields, fields[0], fields[1]);

    pid = strtol(fields[1], NULL, 10);
    if (pid >= RTE_MAX_ETHPORTS)
        rte_exit(EXIT_FAILURE, "Invalid port number '%s'\n", fields[1]);
    DBG_PRINT("Mapping: Port %u\n", pid);

    info->ports[pid].pid = pid;

    num_lcores = rte_strsplit(fields[0], strlen(mapping), lcores, RTE_DIM(lcores), ':');
    if (num_lcores == 1) {
        num_cores = parse_cores(&info->ports[pid], lcores[0], LCORE_MODE_BOTH);
        if (num_cores <= 0)
            rte_exit(EXIT_FAILURE, "Invalid mapping format '%s'\n", map);
        DBG_PRINT("num_cores for both Rx/Tx: %d\n", num_cores);
    } else {
        num_cores = parse_cores(&info->ports[pid], lcores[0], LCORE_MODE_RX);
        if (num_cores <= 0)
            rte_exit(EXIT_FAILURE, "Invalid mapping format '%s'\n", map);
        DBG_PRINT("num_cores for RX: %d\n", num_cores);

        num_cores = parse_cores(&info->ports[pid], lcores[1], LCORE_MODE_TX);
        if (num_cores <= 0)
            rte_exit(EXIT_FAILURE, "Invalid mapping format '%s'\n", map);
        DBG_PRINT("num_cores for TX: %d\n", num_cores);
    }
}

/* Parse the argument given in the command line of the application */
int
parse_args(int argc, char **argv)
{
    int opt, ret;
    char **argvopt;
    int option_index;
    char *prgname = basename(argv[0]);
    char rxtx_desc[64];
    char *descs[3];

    argvopt = argv;

    info = (txpkts_info_t *)calloc(1, sizeof(txpkts_info_t));
    if (!info)
        rte_exit(EXIT_FAILURE, "Unable to allocate memory for txpkts_info_t structure\n");

    for (int i = 0; i < RTE_MAX_ETHPORTS; i++) {
        info->ports[i].pid = RTE_MAX_ETHPORTS + 1;
        pthread_spin_init(&info->ports[i].tx_lock, PTHREAD_PROCESS_PRIVATE);
    }

    if ((info->num_ports = rte_eth_dev_count_avail()) == 0)
        rte_exit(EXIT_FAILURE, "No Ethernet ports found - bye\n");

    info->promiscuous_on = DEFAULT_PROMISCUOUS_MODE;
    info->timeout_secs   = DEFAULT_TIMEOUT_PERIOD;
    info->burst_count    = DEFAULT_BURST_COUNT;
    info->nb_rxd         = DEFAULT_RX_DESC;
    info->nb_txd         = DEFAULT_TX_DESC;
    info->mbuf_count     = DEFAULT_MBUF_COUNT;
    info->pkt_size       = DEFAULT_PKT_SIZE;
    info->tx_rate        = DEFAULT_TX_RATE;
    info->force_quit     = false;
    info->verbose        = false;

    while ((opt = getopt_long(argc, argvopt, short_options, lgopts, &option_index)) != EOF) {
        switch (opt) {
        case 'b': /* RX/TX burst option */
            info->burst_count = strtoul(optarg, NULL, 10);
            if (info->burst_count <= 0)
                info->burst_count = DEFAULT_BURST_COUNT;
            else if (info->burst_count > MAX_BURST_COUNT)
                info->burst_count = MAX_BURST_COUNT;
            DBG_PRINT("RX/TX burst count: %'u\n", info->burst_count);
            break;

        case 's': /* Packet size option */
            info->pkt_size = strtoul(optarg, NULL, 10);
            if (info->pkt_size <= 0)
                info->pkt_size = DEFAULT_PKT_SIZE;
            else if (info->pkt_size > MAX_PKT_SIZE)
                info->pkt_size = MAX_PKT_SIZE;
            DBG_PRINT("Packet size: %'u\n", info->pkt_size);
            break;

        case 'r': /* Tx Rate option */
            info->tx_rate = strtoul(optarg, NULL, 10);
            if (info->tx_rate > MAX_TX_RATE)
                info->tx_rate = DEFAULT_TX_RATE;
            DBG_PRINT("Packet Tx rate: %'u\n", info->tx_rate);
            break;

        case 'd': /* Number of Rx/Tx descriptors */
            snprintf(rxtx_desc, sizeof(rxtx_desc), "%s", optarg);
            if (rte_strsplit(rxtx_desc, strlen(rxtx_desc), descs, RTE_DIM(descs), '/') != 2)
                rte_exit(EXIT_FAILURE, "Invalid Rx/Tx descriptors '%s'\n", optarg);
            info->nb_rxd = strtoul(descs[0], NULL, 10);
            if (info->nb_rxd < MIN_RX_DESC || info->nb_rxd > MAX_RX_DESC)
                info->nb_rxd = DEFAULT_RX_DESC;
            info->nb_txd = strtoul(descs[1], NULL, 10);
            if (info->nb_txd < MIN_TX_DESC || info->nb_txd > MAX_TX_DESC)
                info->nb_txd = DEFAULT_TX_DESC;
            DBG_PRINT("Rx/Tx Ring Size: %'u/%'u\n", info->nb_rxd, info->nb_txd);
            break;

        case 'm': /* Mapping option */
            DBG_PRINT("Mapping: '%s'\n", optarg);

            /* place mapping strings into a list for processing later */
            info->mappings[info->num_mappings++] = strdup(optarg);
            break;

        case 'T': /* Timeout option */
            info->timeout_secs = strtol(optarg, NULL, 0);
            if (info->timeout_secs <= 0)
                info->timeout_secs = DEFAULT_TIMEOUT_PERIOD;
            else if (info->timeout_secs > MAX_TIMEOUT_PERIOD) {
                ERR_PRINT("invalid timeout value 1 <= %'u <= %'d (default %d)\n",
                          info->timeout_secs, MAX_TIMEOUT_PERIOD, DEFAULT_TIMEOUT_PERIOD);
                usage(prgname);
                return -1;
            }
            DBG_PRINT("Timeout period: %'u\n", info->timeout_secs);
            break;

        case 'M': /* MBUF Count */
            info->mbuf_count = strtol(optarg, NULL, 0);
            if (info->mbuf_count < DEFAULT_MBUF_COUNT)
                info->mbuf_count = DEFAULT_MBUF_COUNT;
            else if (info->mbuf_count > MAX_MBUF_COUNT) {
                ERR_PRINT("invalid MBUF Count value 1 <= %'u <= %'d (default %'d)\n",
                          info->mbuf_count, MAX_MBUF_COUNT, DEFAULT_MBUF_COUNT);
                usage(prgname);
                return -1;
            }
            DBG_PRINT("Timeout period: %'u\n", info->timeout_secs);
            break;

        case 'P': /* Promiscuous option */
            info->promiscuous_on = 0;
            DBG_PRINT("Promiscuous mode: off\n");
            break;

        case 'f': /* FGEN string */
            if (fgen_load_strings(info->fgen, (const char **)&optarg, 1) < 0) {
                ERR_PRINT("Unable to load FGEN string '%s'\n", optarg);
                usage(prgname);
                return -1;
            }
            break;

        case 'F': /* FGEN file */
            if (fgen_load_file(info->fgen, optarg) < 0) {
                ERR_PRINT("Unable to load FGEN file '%s'\n", optarg);
                usage(prgname);
                return -1;
            }
            break;

        case 'v': /* Verbose option */
            info->verbose = true;
            break;

        case 'h': /* Help option */
            usage(prgname);
            return 0;

        default:
            usage(prgname);
            return -1;
        }
    }

    if (optind >= 0)
        argv[optind - 1] = prgname;

    ret    = optind - 1;
    optind = 1; /* reset getopt lib */

    printf("num_ports: %'u, nb_rxd %'u, nb_txd %'u, burst %'u, lcores %'u\n", info->num_ports,
           info->nb_rxd, info->nb_txd, info->burst_count, rte_lcore_count());
    info->mbuf_count += info->num_ports * (info->nb_rxd + info->nb_txd + info->burst_count +
                                           (rte_lcore_count() * MEMPOOL_CACHE_SIZE));
    info->mbuf_count = RTE_MAX(info->mbuf_count, DEFAULT_MBUF_COUNT);

    DBG_PRINT("TX packet application started, Burst size %'u, Packet size %'u, Rate %u%%\n",
              info->burst_count, info->pkt_size, info->tx_rate);

    if (info->num_mappings == 0)
        rte_exit(EXIT_FAILURE, "No port mappings specified, use '-m' option\n");

    for (int i = 0; i < info->num_mappings; i++)
        parse_mapping(info->mappings[i]);

    return ret;
}
