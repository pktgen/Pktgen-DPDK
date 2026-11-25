-- Dump portStats() including full port_stats_t (pstats) table
--
-- SPDX-License-Identifier: BSD-3-Clause
--
-- Usage examples:
--   pktgen -f scripts/port_stats_dump.lua
--
-- Optional overrides (edit in this file before running):
--   PORTLIST = "0-3"

package.path = package.path .. ";?.lua;test/?.lua;app/?.lua;../?.lua"

require "Pktgen"

PORTLIST = PORTLIST or "0-1"

local function _key_to_string(k)
    if type(k) == "string" then
        return string.format("%q", k)
    end
    return tostring(k)
end

local function dump_table(value, indent, visited)
    indent = indent or ""
    visited = visited or {}

    if type(value) ~= "table" then
        print(indent .. tostring(value))
        return
    end

    if visited[value] then
        print(indent .. "<cycle>")
        return
    end

    visited[value] = true

    local keys = {}
    for k in pairs(value) do
        keys[#keys + 1] = k
    end

    table.sort(keys, function(a, b)
        local ta, tb = type(a), type(b)
        if ta == tb then
            if ta == "number" then
                return a < b
            end
            return tostring(a) < tostring(b)
        end
        if ta == "number" then
            return true
        end
        if tb == "number" then
            return false
        end
        return ta < tb
    end)

    print(indent .. "{")
    local nextIndent = indent .. "  "

    for _, k in ipairs(keys) do
        local v = value[k]
        io.write(nextIndent .. "[" .. _key_to_string(k) .. "] = ")
        if type(v) == "table" then
            io.write("\n")
            dump_table(v, nextIndent, visited)
        else
            io.write(tostring(v) .. "\n")
        end
    end

    print(indent .. "}")

    visited[value] = nil
end

local stats = pktgen.portStats(PORTLIST)
print(string.format("pktgen.portStats(%q)", PORTLIST))
dump_table(stats)
