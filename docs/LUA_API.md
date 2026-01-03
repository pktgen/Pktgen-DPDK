# Pktgen Lua API (selected)

This document describes the Lua functions exported by the `pktgen` module that were recently updated/clarified, and the table shapes they return.

## Conventions

- Most functions return a Lua table keyed by numeric port id (`pid`).
- Many returned tables include an `n` field holding the number of ports in the returned table.
- Unless explicitly stated, tables may contain additional fields beyond what is documented here.

## `pktgen.portStats(portlist, type)`

Returns per-port statistics.

- `portlist`: port list string understood by pktgen (e.g. `"0"`, `"0-3"`, `"0,2"`).

### Signature

`pktgen.portStats(portlist)`

### Return value

A table with:

- Numeric keys: one entry per port id.
- `n`: number of ports returned.

Each per-port entry contains:

The per-port entry table mirrors a subset of the C `port_stats_t` structure.

### Per-port table layout

Each port entry contains:

- `curr`: a table containing:
  - `ipackets`, `opackets`, `ibytes`, `obytes`
  - `ierrors`, `oerrors`, `rx_nombuf`, `imissed`
- `ext`: extended counters:
  - `arp_pkts`, `echo_pkts`, `ip_pkts`, `ipv6_pkts`, `vlan_pkts`
  - `dropped_pkts`, `unknown_pkts`, `tx_failed`
  - `imissed`, `ibadcrc`, `ibadlen`, `rx_nombuf`
  - `max_ipackets`, `max_opackets`
- `sizes`: packet size distribution:
  - `_64`, `_65_127`, `_128_255`, `_256_511`, `_512_1023`, `_1024_1522`
  - `broadcast`, `multicast`, `jumbo`, `runt`, `unknown`
- `qstats`: per-queue tables keyed by queue id.
  - The number of entries is limited to the number of **configured Rx queues** for the port.
  - Queue ids start at `0`.
  - Each queue entry has:
  - `ipackets`, `opackets`, `errors`

### Example

```lua
-- Get the current port stats snapshot for ports 0-1
local t = pktgen.portStats("0-1")

for pid = 0, 1 do
  print("port", pid, "curr rx", t[pid].curr.ipackets)
  print("port", pid, "curr tx", t[pid].curr.opackets)
end
```

## `pktgen.portInfo(portlist)`

Returns per-port configuration and informational fields.

Notes:

- This API intentionally does **not** include `port_stats_t`.
- This API intentionally does **not** include `rte_eth_stats` (`stats`/`totals` style counters).

### Return value

A table with:

- Numeric keys: one entry per port id.
- `n`: number of ports returned.

Each per-port entry includes a set of informational subtables (non-exhaustive):

- `total_stats`: pktgen global totals (e.g. `max_ipackets`, `max_opackets`, cumulative rate totals)
- `info`: transmit configuration (pattern type, tx_count, tx_rate, pkt_size, bursts, eth/proto type, vlanid)
- `l2_l3_info`: L2/L3 fields (ports, TTL, src/dst IP, src/dst MAC)
- `tx_debug`: internal tx debug fields
- `802.1p`: QoS fields (`cos`, `dscp`, `ipp`)
- `VxLAN`: VxLAN fields and `link_state`
- `pci_vendor`: PCI/vendor string (top-level field, if available)

### Example

```lua
local info = pktgen.portInfo("0")
print("link", info[0].VxLAN.link_state)
print("tx rate", info[0].info.tx_rate)
```

## Example output of scripts/port_stats_dump.lua

```lua
Pktgen> load scripts/port_stats_dump.lua
Pktgen:/> load scripts/port_stats_dump.lua

Executing 'scripts/port_stats_dump.lua'
pktgen.portStats("0-1")
{
  [0] =
  {
    ["curr"] =
    {
      ["ibytes"] = 0
      ["ierrors"] = 0
      ["imissed"] = 0
      ["ipackets"] = 0
      ["obytes"] = 0
      ["oerrors"] = 0
      ["opackets"] = 0
      ["rx_nombuf"] = 0
    }
    ["ext"] =
    {
      ["arp_pkts"] = 0
      ["dropped_pkts"] = 0
      ["echo_pkts"] = 0
      ["ibadcrc"] = 0
      ["ibadlen"] = 0
      ["imissed"] = 0
      ["ip_pkts"] = 0
      ["ipv6_pkts"] = 0
      ["max_ipackets"] = 0
      ["max_opackets"] = 0
      ["rx_nombuf"] = 0
      ["tx_failed"] = 0
      ["unknown_pkts"] = 0
      ["vlan_pkts"] = 0
    }
    ["qstats"] =
    {
      [0] =
      {
        ["errors"] = 0
        ["ipackets"] = 0
        ["opackets"] = 0
      }
      [1] =
      {
        ["errors"] = 0
        ["ipackets"] = 0
        ["opackets"] = 0
      }
    }
    ["sizes"] =
    {
      ["_1024_1522"] = 0
      ["_128_255"] = 0
      ["_256_511"] = 0
      ["_512_1023"] = 0
      ["_64"] = 0
      ["_65_127"] = 0
      ["broadcast"] = 0
      ["jumbo"] = 0
      ["multicast"] = 0
      ["runt"] = 0
      ["unknown"] = 0
    }
  }
  [1] =
  {
    ["curr"] =
    {
      ["ibytes"] = 0
      ["ierrors"] = 0
      ["imissed"] = 0
      ["ipackets"] = 0
      ["obytes"] = 0
      ["oerrors"] = 0
      ["opackets"] = 0
      ["rx_nombuf"] = 0
    }
    ["ext"] =
    {
      ["arp_pkts"] = 0
      ["dropped_pkts"] = 0
      ["echo_pkts"] = 0
      ["ibadcrc"] = 0
      ["ibadlen"] = 0
      ["imissed"] = 0
      ["ip_pkts"] = 0
      ["ipv6_pkts"] = 0
      ["max_ipackets"] = 0
      ["max_opackets"] = 0
      ["rx_nombuf"] = 0
      ["tx_failed"] = 0
      ["unknown_pkts"] = 0
      ["vlan_pkts"] = 0
    }
    ["qstats"] =
    {
      [0] =
      {
        ["errors"] = 0
        ["ipackets"] = 0
        ["opackets"] = 0
      }
      [1] =
      {
        ["errors"] = 0
        ["ipackets"] = 0
        ["opackets"] = 0
      }
    }
    ["sizes"] =
    {
      ["_1024_1522"] = 0
      ["_128_255"] = 0
      ["_256_511"] = 0
      ["_512_1023"] = 0
      ["_64"] = 0
      ["_65_127"] = 0
      ["broadcast"] = 0
      ["jumbo"] = 0
      ["multicast"] = 0
      ["runt"] = 0
      ["unknown"] = 0
    }
  }
  ["n"] = 2
}
```
