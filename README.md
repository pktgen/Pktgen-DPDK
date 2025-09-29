# Pktgen — DPDK Traffic Generator

High‑performance, scriptable packet generator capable of wire‑rate transmission with 64‑byte frames.

Pronounced: “packet‑gen”

[Documentation](https://pktgen.github.io/Pktgen-DPDK/) · [Releases](https://github.com/pktgen/Pktgen-DPDK/releases)

---

## Table of Contents

1. [Overview](#1-overview)
1. [Features](#2-features-partial-list)
1. [Quick Start](#3-quick-start)
1. [Building](#4-building-details)
1. [Configuration Files](#5-configuration-files-cfg)
1. [Configuration Key Reference](#6-configuration-key-reference)
1. [Runtime Modes & Pages](#7-runtime-modes--pages)
1. [Automation & Remote Control](#8-automation--remote-control)
1. [Advanced Topics](#9-advanced-topics)
1. [Contributing](#10-contributing)
1. [License](#11-license)
1. [Related Links](#12-related-links)
1. [Acknowledgments](#13-acknowledgments)

---

## 1. Overview

Pktgen is a multi‑port, multi‑core traffic generator built on top of [DPDK]. It targets realistic, repeatable performance and functional packet tests while remaining fully controllable via an interactive console, Lua scripts, or a remote TCP socket.

> Primary repository: <https://github.com/pktgen/Pktgen-DPDK>

## 2. Features (Partial List)

- Wire‑rate 64B packet generation (hardware and core count permitting)
- Multi‑port / multi‑queue scaling
- IPv4 / IPv6, TCP / UDP, VLAN, GRE, GTP-U support
- Packet sequence, range, random, pcap replay and latency modes
- Latency & jitter measurement, per‑queue and extended stats pages
- Lua scripting (local or remote) + TCP control socket (default port 22022)
- Configurable theming and multiple display pages (main, seq, range, rnd, pcap, stats, xstats)
- Plugin architecture (`lib/plugin`), capture and pcap dumping
- Dynamic rate control and pacing recalculated on size/speed changes

## 3. Quick Start

Prerequisites (typical Ubuntu 22.04+):

- Latest DPDK (build + install using Meson/Ninja)
- libbsd (`sudo apt install libbsd-dev`)
- Hugepages configured (e.g. 1G or 2M pages) and NICs bound to `vfio-pci` or `igb_uio`
- Python 3.x for helper scripts

Clone and build:

```bash
git clone https://github.com/pktgen/Pktgen-DPDK.git
cd Pktgen-DPDK
meson setup builddir
meson compile -C builddir
```

Initial device setup (only once per boot) then run a config:

```bash
sudo ./tools/run.py -s default   # bind devices & prepare environment
sudo ./tools/run.py default      # launch using cfg/default.cfg
```

Minimal manual run (no helper script) example (adjust cores/ports):

```bash
sudo ./builddir/app/pktgen -l 0-3 -n 4 -- -P -m "[1:2].0" -T
```

## 4. Building (Details)

Pktgen uses Meson/Ninja. A convenience `Makefile` and `tools/pktgen-build.sh` wrap standard steps.

1. Build and install DPDK so `libdpdk.pc` is installed (often under `/usr/local/lib/x86_64-linux-gnu/pkgconfig`).
1. Export or append to `PKG_CONFIG_PATH` if that path is non‑standard:

```bash
export PKG_CONFIG_PATH=/usr/local/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
```

1. Build Pktgen:

```bash
meson setup builddir
meson compile -C builddir
```

Or use:

```bash
make          # uses Meson/Ninja under the hood
make rebuild  # clean reconfigure & build
make rebuildlua
```

1. (Optional) Install Pktgen artifacts via Meson if desired.

Troubleshooting hints:

- If runtime fails to find DPDK libs: run `sudo ldconfig` or add the library path to `/etc/ld.so.conf.d/`.
- Enable IOMMU for vfio: edit GRUB adding `intel_iommu=on` (or `amd_iommu=on`) then `update-grub` and reboot.

## 5. Configuration Files (`cfg/`)

Configuration files are Python fragments consumed by `tools/run.py`. They define two dictionaries: `setup` (device binding, privilege wrapper) and `run` (execution parameters).

Trimmed example:

```python
description = 'Simple default configuration'

setup = {
    'devices': (
        '81:00.0','81:00.1'
    ),
    'uio': 'vfio-pci'
}

run = {
    'app_name': 'pktgen',
    'cores': '14,15-16',      # control lcore, then worker/core ranges
    'map': ('[15:16].0',),     # tx:rx core pair -> port
    'opts': ('-T','-P'),       # -T: color theme, -P: promiscuous
    'theme': 'themes/black-yellow.theme'
}
```

Common keys (not exhaustive):

- `devices`: PCI BDFs to bind.
- `blocklist`: exclude listed devices.
- `cores`: control and worker core list/ranges.
- `map`: mapping of core pairs to ports `[tx:rx].port`.
- `opts`: extra runtime flags passed after `--`.
- `nrank`, `proc`, `log`, `prefix`: process / logging / multi‑process tuning.

See existing examples in `cfg/` (e.g. `default.cfg`, `two-ports.cfg`, `pktgen-1.cfg`, `pktgen-2.cfg`).

## 6. Configuration Key Reference

| Key | Location | Type | Example | Description |
|-----|----------|------|---------|-------------|
| devices | setup | tuple/list | `('81:00.0','81:00.1')` | PCI BDFs to bind to DPDK |
| uio | setup | string | `vfio-pci` | Kernel driver to bind (vfio-pci / igb_uio / uio_pci_generic) |
| exec | setup/run | tuple | `('sudo','-E')` | Wrapper for privileged execution |
| app_name | run | string | `pktgen` | Binary name; resolved via `app_path` list |
| app_path | run | tuple/list | `('./app/%(target)s/%(app_name)s', ...)` | Candidate paths to locate binary |
| cores | run | string | `14,15-16` | Control + worker cores; ranges and commas allowed |
| map | run | tuple/list | `('[15:16].0',)` | TX:RX core pair mapped to port id |
| opts | run | tuple/list | `('-T','-P')` | Extra runtime flags passed after `--` |
| theme | run | string | `themes/black-yellow.theme` | Color/theme selection |
| blocklist | run | tuple/list | `('81:00.2',)` | Exclude listed PCI devices |
| nrank | run | string/int | `4` | Multi-process ranking parameter (advanced) |
| proc | run | string | `auto` | Process type / role selection |
| log | run | string/int | `7` | Log verbosity level |
| prefix | run | string | `pg` | DPDK shared resource (memzone) prefix |

> Not all keys are required; unused advanced keys can be omitted. Refer to examples in `cfg/` for patterns.

## 7. Runtime Modes & Pages

Modes: single (default), sequence, range, random, pcap replay, latency.

Display pages correspond to configuration areas: `page main|seq|range|rnd|pcap|stats|xstats`.
Each mode maintains separate packet template buffers—configure the active mode explicitly.

## 8. Automation & Remote Control

Pktgen exposes a TCP socket (default port `22022`) offering a Lua REPL‑like interface (no prompt). Examples:

Interactive with socat:

```bash
socat -d -d READLINE TCP4:localhost:22022
```

Run a Lua script remotely:

```bash
socat - TCP4:localhost:22022 < test/hello-world.lua
```

Single command:

```bash
echo "f,e=loadfile('test/hello-world.lua'); f();" | socat - TCP4:localhost:22022
```

Example script (`test/hello-world.lua`):

```lua
package.path = package.path .. ";?.lua;test/?.lua;app/?.lua;"
printf("Lua Version: %s\n", pktgen.info.Lua_Version)
printf("Pktgen Version: %s\n", pktgen.info.Pktgen_Version)
printf("Pktgen Copyright: %s\n", pktgen.info.Pktgen_Copyright)
printf("Pktgen Authors: %s\n", pktgen.info.Pktgen_Authors)
printf("\nHello World!!!!\n")
```

## 9. Advanced Topics

- Multiple instances: see `pktgen-1.cfg` / `pktgen-2.cfg` for running concurrently (ensure isolated devices/cores).
- Themes: located under `themes/`, selected via `-T` or config `theme` key.
- Latency: latency packets can be injected in any mode; view stats on `page stats` / latency display.
- Capture & PCAP: capture to pcap files or replay existing pcaps (`pcap/` directory contains samples).
- Performance tuning: pin isolated cores, match NUMA locality (ports & mempools), ensure sufficient mbufs, verify TSC stability.
- Plugins: extend via modules under `lib/plugin`.

## 10. Contributing

Please fork and submit pull requests via GitHub. Patches sent to the DPDK mailing list are not accepted for this repo.

For detailed guidelines (coding style, commit message format, documentation rules) see [`CONTRIBUTING.md`](./CONTRIBUTING.md).
Extended Markdown formatting conventions are documented in [`docs/STYLE.md`](./docs/STYLE.md).
Community expectations and reporting process: see [`CODE_OF_CONDUCT.md`](./CODE_OF_CONDUCT.md).

Report issues / feature requests: <https://github.com/pktgen/Pktgen-DPDK/issues>

When filing issues include:

- Pktgen version (`cat VERSION` → current: 25.08.0)
- DPDK version & build config
- NIC model(s) & driver
- Reproduction steps + minimal config

### 10.1 Markdown Linting (Pre-Commit Hook)

Documentation style is enforced via markdownlint.

Enable the hook:

```bash
ln -sf ../../.githooks/pre-commit .git/hooks/pre-commit
chmod +x .githooks/pre-commit
```

Requirements:

```bash
node --version   # ensure Node.js installed
npm install --no-save markdownlint-cli2  # optional; hook auto-installs if missing
```

On commit, staged `*.md` files are linted. If violations are found the commit is aborted; some fixable rules may be auto-corrected—re-add and recommit.

## 11. License

SPDX-License-Identifier: BSD-3-Clause
Copyright © 2010-2025 Intel Corporation

Full license text is available in [`LICENSE`](./LICENSE).

## 12. Related Links

- Documentation: <https://pktgen.github.io/Pktgen-DPDK/>
- Install guide (legacy / extended details): [`INSTALL.md`](./INSTALL.md)
- Example pcaps: `pcap/`

## 13. Acknowledgments

Created and maintained by Keith Wiles @ Intel Corporation with contributions from the community.

---

If this tool helps your testing, consider starring the project or contributing improvements.

[DPDK]: https://www.dpdk.org/
