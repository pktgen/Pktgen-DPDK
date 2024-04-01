## PKTPERF example application to verify Rx/Tx performance with multiple cores

The `pktperf` example application is used to verify Rx/Tx performance with mutiple cores or queues. The application is configured from the command line arguments and does not have CLI to configure on the fly.

---
```console
**Copyright &copy; <2023>, Intel Corporation. All rights reserved.**

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

- Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

SPDX-License-Identifier: BSD-3-Clause

pktperf: Created 2023 by Keith Wiles @ Intel.com
```
---

### Overview

The implementation uses multiple Rx/Tx queues per port, which is required to achieve maximum performance for receiving and transmitting frames. If your NIC does not support multiple queues, but does support multiple `virtual functions` per physical port, then you should create multiple `virtual functions` network devices and configure them using the `-m` mapping parameters. A single core can only be attached to one network device or port, which means you can not have a single core processing more then one network device or port.

Having a single core processing more then one network device or port was almost never used and made the code more complicated. This is why the implementation was removed from the `pktperf` application.

### Command line arguments

The command line arguments contain the standard DPDK arguments with the pktperf parameters after the '--' option. Please look at the DPDK documentation for the EAL arguments. The pktperf parameter can be displayed using the -h or --help option after the '--' option.

```console
pktperf [EAL options] -- [-b burst] [-s size] [-r rate] [-d rxd/txd] [-m map] [-T secs] [-P] [-M mbufs] [-v] [-h]
	-b|--burst-count <burst> Number of packets for Rx/Tx burst (default 32)
	-s|--pkt-size <size>     Packet size in bytes (default 64) includes FCS bytes
	-r|--rate <rate>         Packet TX rate percentage 0=off (default 100)
	-d|--descriptors <Rx/Tx> Number of RX/TX descriptors (default 1,024/1,024)
	-m|--map <map>           Core to Port/queue mapping '[Rx-Cores:Tx-Cores].port'
	-T|--timeout <secs>      Timeout period in seconds (default 1 second)
	-P|--no-promiscuous      Turn off promiscuous mode (default On)
	-M|--mbuf-count <count>  Number of mbufs to allocate (default 8,192, max 131,072)
	-v|--verbose             Verbose output
	-h|--help                Print this help
```
### Command line example

```bash
sudo builddir/examples/pktperf/pktperf -l 1,2-9,14-21 -a 03:00.0 -a 82:00.0 -- -m "2-5:6-9.0" -m "14-17:18-21.1"
```

## CPU/Socket layout

```bash
======================================================================
Core and Socket Information (as reported by '/sys/devices/system/cpu')
======================================================================

cores =  [0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14]
sockets =  [0, 1]

        Socket 0        Socket 1
        --------        --------
Core 0  [0, 28]         [14, 42]
Core 1  [1, 29]         [15, 43]
Core 2  [2, 30]         [16, 44]
Core 3  [3, 31]         [17, 45]
Core 4  [4, 32]         [18, 46]
Core 5  [5, 33]         [19, 47]
Core 6  [6, 34]         [20, 48]
Core 8  [7, 35]         [21, 49]
Core 9  [8, 36]         [22, 50]
Core 10 [9, 37]         [23, 51]
Core 11 [10, 38]        [24, 52]
Core 12 [11, 39]        [25, 53]
Core 13 [12, 40]        [26, 54]
Core 14 [13, 41]        [27, 55]
```

The `-m` argument defines the core to port mapping `<RxCores:TxCores>.<port>` the `:` (colon) is used to specify the the Rx and Tx cores for the port mapping. Leaving off the ':' is equivalent to running Rx and Tx processing on the specified core(s). When present the left side denotes the core(s) to use for receive processing and the right side denotes the core(s) to use for transmit processing.

### Example console output

```bash
Port    : Rate Statistics per queue (-)
 0 >> Link up at 40 Gbps FDX Autoneg, WireSize 672 Bits, PPS 59,523,809, Cycles/Burst 5,120
  RxQs  :    9,512,816    8,870,944    9,235,916    9,272,516 Total:   36,892,192
  TxQs  :    6,182,144   12,364,288    6,182,144   12,364,288 Total:   37,092,864
  TxDrop:    8,779,360    2,596,544    8,779,072    2,596,192 Total:   22,751,168
  NoMBUF:            0            0            0            0 Total:            0
  TxTime:          351          384          372          330 Total:        1,437
  RxMissed:      248,108, ierr: 0, oerr: 0, RxNoMbuf: 0
 1 >> Link up at 100 Gbps FDX Autoneg, WireSize 672 Bits, PPS 148,809,523, Cycles/Burst 2,048
  RxQs  :   23,465,206   24,484,244   24,600,096   25,202,408 Total:   97,751,954
  TxQs  :   23,915,040   23,915,040   23,915,040   23,915,040 Total:   95,660,160
  TxDrop:   13,339,456   13,352,064   13,327,936   13,353,056 Total:   53,372,512
  NoMBUF:            0            0            0            0 Total:            0
  TxTime:          489          483          474          564 Total:        2,010
  RxMissed:            0, ierr: 0, oerr: 0, RxNoMbuf: 0

Burst: 32, MBUF Count: 12,864, PktSize:64, Rx/Tx 1,024/1,024, Rate 100%
```
