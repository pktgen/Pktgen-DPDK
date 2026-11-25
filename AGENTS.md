# Repository guidance

## Build and verification

- Build with `meson setup builddir` then `meson compile -C builddir`. Meson requires `libdpdk` via pkg-config plus NUMA, pcap and BSD development libraries; set `PKG_CONFIG_PATH` if DPDK is installed outside the default search path. Defaults are release, static libraries and warnings as errors.
- Lua is **disabled by default**. For Lua scripts or the remote Lua socket, configure with `meson setup builddir -Denable_lua=true` (or `meson configure builddir -Denable_lua=true` on an existing build); Lua development libraries are required.
- `make` wraps Meson/Ninja and also installs into `./usr/local` by default. `make rebuild`, `make buildlua`, `make debug`, `make debuglua` and `make debugopt` first delete `builddir` (or `PKTGEN_BUILD`); do not use them casually on an existing build. The README's `make rebuildlua` target does not exist.
- There are no Meson test definitions. `test/*.lua` are Pktgen runtime scripts, not standalone unit tests; some start traffic or assume specific ports and addresses. Runtime verification generally requires configured hugepages and suitable DPDK ports.
- CI checks C formatting with clang-format 21 and Markdown with `npx markdownlint-cli2 "**/*.md" "!usr/**" "!builddir/**"`. For a focused C check, use `clang-format --dry-run --Werror path/to/file.c` with version 21. See `.clang-format`, `.markdownlint.yml` and `CONTRIBUTING.md`.
- To build docs without DPDK, use a separate directory: `meson setup build-docs -Donly_docs=true` then `ninja -C build-docs doc`. Sphinx is needed for HTML; `-Denable_docs=true` additionally requires Doxygen and Sphinx.

## Layout and runtime

- `app/pktgen-main.c` initializes DPDK EAL and launches worker lcores; `app/` contains the packet, CLI and display implementation. Shared components are built from `lib/`; Lua bindings are in `app/lpktgenlib.c` and `lib/lua/`. `examples/pktperf` is built only when optional `libfgen` is found.
- `tools/run.py` loads `cfg/*.cfg` as executable Python, resolves `run['app_path']`, and constructs EAL arguments before `--` and Pktgen arguments after it. `./tools/run.py -n <config>` prints the command without launching, but still loads the config and requires an existing executable.
- Do not run `tools/run.py -s` without checking the selected config and host: it changes kernel modules and hugepages and binds the configured PCI devices. Example configs contain machine-specific cores and PCI addresses; `cfg/default.cfg` searches installed binaries, not `builddir/app/pktgen`.
- Packet modes have separate buffers/configuration; changes to one mode do not automatically configure the others.
