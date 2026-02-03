# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## Build Commands

The project uses **Meson/Ninja**. A `builddir/` is typically already initialised.

```bash
# Incremental build (most common)
ninja -C builddir

# Full configure + build from scratch
meson setup builddir
meson compile -C builddir

# Makefile wrapper (delegates to tools/pktgen-build.sh)
make build          # release build
make rebuild        # clean + release build
make debug          # debug build (no optimisation)
make debugopt       # debug with -O2
make buildlua       # release + Lua scripting enabled
make clean

# Build options (pass to meson setup)
meson setup builddir -Denable_lua=true
meson setup builddir -Denable_docs=true
meson setup builddir -Donly_docs=true      # docs only, skip app/lib

# Code formatting
ninja -C builddir clang-format-check       # check only (CI does this)
ninja -C builddir clang-format             # auto-fix in place

# Documentation
make docs                                  # Sphinx + Doxygen

# Tests
ninja -C builddir test                     # meson test runner
# Integration tests are Lua scripts in test/ that connect to a running
# pktgen instance over its TCP socket (port 22022).
```

Build is strict: **warning_level=3** and **werror=true**. Any new warning is a build failure.

---

## Pre-commit Hook Setup

```bash
ln -sf ../../.githooks/pre-commit .git/hooks/pre-commit
chmod +x .githooks/pre-commit
```

This runs `clang-format -i` on staged C files and `markdownlint` on staged Markdown.

---

## Commit Message Convention

From `CONTRIBUTING.md`:

```
<short summary in imperative mood>

<context / motivation>
<what changed>
<any impact / migration notes>
```

Example prefix pattern observed in recent commits: `refactor:`, `fix:`, `stats:`, etc.

---

## High-Level Architecture

Pktgen is a DPDK-based wire-rate packet generator. The binary is `builddir/app/pktgen`.

### Module layout

```
lib/
  hmap/      Hash-map (type-safe, 64-bit unified storage)
  common/    Shared utilities: checksums, IP parsing, strings, CPU info, port config
  utils/     Portlist parsing, heap, inet_pton, JSON (parson)
  vec/       Dynamic-array (vector) container
  plugin/    dlopen-based plugin loader (up to 32 plugins, start/stop lifecycle)
  cli/       Full interactive shell: node tree, gap-buffer editor, TAB completion,
             history, help, env vars, map-driven argument parsing
  lua/       Lua 5.3/5.4 scripting bindings (conditional on -Denable_lua=true)

app/
  pktgen-main.c      Entry point: arg parsing, EAL init, port setup, CLI loop start
  pktgen.c           Core TX loop logic, packet rate calculation, timer callbacks
  pktgen.h           Global pktgen_t struct, page/flag enums, per-port iteration macros
  pktgen-port-cfg.h  port_info_t (per-port state), SEND_* flag bits, pkt_seq_t layout
  pktgen-cmds.c      ~112 KB command implementation dispatcher
  cli-functions.c    ~74 KB CLI map tables + command handler glue (registers maps)
  l2p.c / l2p.h      Lcore-to-Port mapping matrix, mempool ownership
  pktgen-{ipv4,ipv6,tcp,udp,arp,vlan,gre,gtpu,ether}.c   Protocol header builders
  pktgen-range.c     Incrementing-field packet generation
  pktgen-seq.c       16-slot packet sequence
  pktgen-random.c    Per-field random bitfield masks
  pktgen-pcap.c      PCAP file load/replay
  pktgen-capture.c   Packet capture to memory
  pktgen-latency.c   Latency sampling & histograms
  pktgen-stats.c     Per-port / per-queue stats and rate calculation
  pktgen-display.c   Terminal UI page rendering (10 display pages)
  pktgen-log.c       Ring-buffer message log
```

### Build dependency order (lib subdirectories)

```
hmap → common → utils → vec → plugin → cli → lua
```

All lib modules compile to static libraries and are linked into the single `pktgen` executable.

### Data-flow summary

1. **Initialisation** (`pktgen-main.c`): parse `-m` matrix string → populate `l2p_t` (which lcores own which ports/queues) → allocate per-queue mempools (`rx_mp`, `tx_mp`, `sp_mp`) → build default packet templates via `pktgen_packet_ctor()` → launch one worker per lcore.

2. **TX loop** (`pktgen.c:pktgen_launch_one_lcore`): each worker polls its assigned port/queue. The active sending mode is determined by the port's `port_flags` atomic (e.g. `SEND_SINGLE_PKTS`, `SEND_RANGE_PKTS`, `SEND_SEQ_PKTS`, `SEND_PCAP_PKTS`). Rate limiting uses cycle-based inter-burst timing (`tx_cycles`).

3. **RX path**: received packets are optionally processed (`PROCESS_INPUT_PKTS` — handles ARP/ICMP echo), captured (`CAPTURE_PKTS`), or used for latency measurement (`SAMPLING_LATENCIES`).

4. **CLI / command path**: the interactive shell runs on the main lcore. Commands are dispatched via `cli_map` pattern matching in `cli-functions.c` → actual logic in `pktgen-cmds.c`. Changing a port setting typically sets a flag (`SETUP_TRANSMIT_PKTS`) so the worker rebuilds its packet template on the next iteration.

5. **Display refresh**: a periodic timer (`UPDATE_DISPLAY_TICK_RATE`) triggers `pktgen_page_display()` which re-renders the current screen page using stats collected via `rte_eth_stats_get` / extended stats.

### Key global state

- `pktgen_t pktgen` — single global (defined in `pktgen.c`, declared `extern` in `pktgen.h`). Holds port count, display state, CPU info, capture buffers, Lua handles.
- `l2p_t` — lcore↔port mapping. Accessed via `l2p_get_port_pinfo(pid)` and the per-lcore `l2p_get_lport(lid)`.
- `port_info_t` — per-port struct (embedded inside `l2p_port_t.pinfo`). Contains packet templates (`seq_pkt[NUM_TOTAL_PKTS]`), range config, stats, latency, random-field masks, and the 64-bit `port_flags` atomic.
- `this_cli` — `RTE_PER_LCORE` pointer to `struct cli`. The CLI tree, gap buffer, history, and the `cmd_maps` registry all live here.

### Port-flag model

`port_flags` is a 64-bit atomic on each `port_info_t`. Bits are grouped:

| Group | Bits | Semantics |
|---|---|---|
| Non-exclusive RX/misc | 0–9 | ARP, ICMP echo, capture, latency sampling |
| Exclusive TX mode | 12–15 | Exactly one of SINGLE/PCAP/RANGE/SEQ must be set |
| Exclusive pkt-type | 16–23 | VLAN, MPLS, GRE, VxLAN, random, latency |
| Control | 28–31 | SETUP_TRANSMIT_PKTS, SENDING_PACKETS, SEND_FOREVER |

The macros `EXCLUSIVE_MODES` and `EXCLUSIVE_PKT_MODES` define the mutual-exclusion masks.

### CLI auto-completion architecture

TAB completion has two layers:

1. **Tree completion** — walks `cli_node` children of the current directory, matches prefixes of command/dir/file names.
2. **Map completion** — once the first token (command name) is typed, `cli_get_cmd_map(argv[0])` returns the registered `cli_map` table. The completer scans all map entries whose fixed tokens match what has been typed so far, then offers candidates or placeholder hints (e.g. `<portlist>`, `<ipv4-addr>`) for the current argument position.

Map registration happens automatically: `cli_help_add(group, map, help)` calls `cli_register_cmd_maps(map)` which extracts every unique first-token and stores `{token → map}` in the growable `cmd_maps` array on `this_cli`. Do not call `cli_register_cmd_map()` manually after `cli_help_add()` — it would be a redundant no-op.

Map format tokens: `%d` (32-bit), `%D` (64-bit), `%u`/`%U` (unsigned), `%h`/`%H` (hex), `%b` (8-bit), `%n` (number), `%s` (string), `%c` (comma-list), `%m` (MAC), `%4` (IPv4), `%6` (IPv6), `%P` (portlist), `%C` (corelist), `%k` (kvargs), `%l` (list), `%|opt1|opt2|…` (choice token).

### Packet-slot layout (`pkt_seq_t` array)

Each port has `NUM_TOTAL_PKTS = 20` slots:

| Index | Role |
|---|---|
| 0–15 | Sequence packets (cycled in `SEND_SEQ_PKTS` mode) |
| 16 (`SINGLE_PKT`) | Default single-packet template |
| 17 (`SPECIAL_PKT`) | Temporary / scratch packet |
| 18 (`RANGE_PKT`) | Template rebuilt each burst in range mode |
| 19 (`LATENCY_PKT`) | Latency probe packet |

---

## Coding Conventions

- **Formatting**: LLVM-based `.clang-format`, 100-column limit, 4-space indent, right-aligned pointers. Run `clang-format -i` on any file you touch.
- **New files**: must carry the SPDX header (`SPDX-License-Identifier: BSD-3-Clause`) and a copyright line.
- **Port iteration**: use the `forall_ports(_action)` or `foreach_port(_portlist, _action)` macros from `pktgen.h` rather than hand-rolling port loops.
- **TAILQ / CIRCLEQ**: the CLI and help systems use POSIX tail-queues extensively. New linked structures should follow the same pattern.
- **Atomic port flags**: always use `rte_atomic64_t` operations when reading or writing `port_info_t.port_flags` from worker lcores.
- **cli_map entries**: every entry in a `cli_map` array must have a corresponding `case` in the command handler's switch on `m->index`. Orphan entries cause phantom command registrations and broken auto-complete.

---

## CI Checks (GitHub Actions)

Three workflows run on every PR:

| Workflow | What it checks |
|---|---|
| `clang-format.yml` | All C/C++ files pass `clang-format --dry-run` (clang-format 21) |
| `markdownlint.yml` | All `.md` files pass `markdownlint-cli2` (config in `.markdownlint.yml`) |
| `doc.yml` | Sphinx docs build cleanly (runs on push to `main` only) |

Markdown rules of note: bare URLs must be wrapped in `< >`, numbered lists should use `1.` style, fenced code blocks must have a language hint.
