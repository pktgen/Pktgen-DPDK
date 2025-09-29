# Pktgen Quick Start

This guide provides a concise path from a clean system to generating traffic with Pktgen.

## 1. Prerequisites

- A modern Linux distribution (tested on Ubuntu 22.04+)
- Root (or sudo) access
- Git, build-essential (gcc/clang, make), Python 3
- Meson & Ninja: `pip install meson ninja` (or distro packages)
- libbsd headers: `sudo apt install libbsd-dev`
- Huge pages configured (e.g. 1G or 2M pages) and IOMMU enabled if using `vfio-pci`
- Latest DPDK source (<https://www.dpdk.org/>)

## 2. Build and Install DPDK

```bash
git clone https://github.com/DPDK/dpdk.git
cd dpdk
meson setup build
meson compile -C build
sudo meson install -C build
sudo ldconfig
```

If `libdpdk.pc` is placed under a non-standard path (commonly `/usr/local/lib/x86_64-linux-gnu/pkgconfig`), export:

```bash
export PKG_CONFIG_PATH=/usr/local/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
```

## 3. Clone and Build Pktgen

```bash
git clone https://github.com/pktgen/Pktgen-DPDK.git
cd Pktgen-DPDK
meson setup builddir
meson compile -C builddir
```

(Optional) Using Make wrapper:

```bash
make        # builds via meson/ninja
make rebuild
make rebuildlua
```

## 4. Device Preparation

Bind NIC ports for DPDK use (one-time per boot). You can use `./tools/run.py -s <cfg>` or standard DPDK binding tools. Example with run script:

```bash
sudo ./tools/run.py -s default
```

Manual invocation example (adjust cores & ports):

## 5. Launch Pktgen

Using a configuration file:

```bash
sudo ./builddir/app/pktgen -l 0-3 -n 4 -- -P -m "[1:2].0" -T
```

Where:

- `-l 0-3` lcore list (control + workers)
- `-n 4` memory channels
- After `--` are Pktgen options; `-P` promiscuous, `-m` maps core pairs to port, `-T` enables themed/color output

## 6. Basic Console Commands

Inside the interactive console (`Pktgen>` prompt):

```text
help                 # list commands
port 0               # focus on port 0
set 0 size 512       # change packet size
start 0              # start transmitting on port 0
stop 0               # stop transmitting
page stats           # view statistics page
save test.lua        # save current configuration to lua script

Remote execution via socket (default port 22022):
```

## 7. Lua Scripting

Run a script at startup:

```bash
echo "print(pktgen.info.Pktgen_Version)" | socat - TCP4:localhost:22022
```

## 8. Performance Tips

- Pin isolated cores (`isolcpus=` kernel parameter or `taskset`)
- Keep NIC and memory allocation on the same NUMA node
- Ensure sufficient mbufs (configured in source or options)
- Disable power management / frequency scaling for consistent latency
- Use latest stable DPDK + recent CPU microcode

## 9. Troubleshooting

| Missing `libdpdk.so` | `ldconfig` not updated | Run `sudo ldconfig` or adjust `/etc/ld.so.conf.d` |
| Permission denied | IOMMU / vfio not enabled | Add `intel_iommu=on` (or `amd_iommu=on`) to GRUB and reboot |
| Low performance | NUMA mismatch / core sharing | Reassign cores & ensure local memory |
| Socket connect fails | Port blocked/firewall | Check `22022` listening, adjust firewall |

## 10. Next Steps

- Explore `cfg/` configs for multi-port examples
- Use `page range` or `page seq` for more advanced packet patterns
- Capture traffic with Wireshark/tcpdump to validate packet contents

---
For deeper details refer to the main README or full documentation site.
