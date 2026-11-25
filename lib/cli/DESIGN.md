# CLI library (lib/cli) — design and usage

This directory contains Pktgen’s interactive CLI library (a DPDK-style “cmdline workalike”).
It implements a small shell-like environment with a directory tree of nodes (commands, files,
aliases, and directories), history and editing, and TAB completion.

If you want the full historical narrative and sample application details, also see:
- `lib/cli/cli.rst` (CLI sample application guide)
- `lib/cli/cli_lib.rst` (CLI library guide)

This document is intended as a **developer-focused** overview of how the code is structured,
how to extend it, and how completion and map-driven parsing work in this repository.

## High-level architecture

### Core concepts

- **CLI instance (`struct cli`)**
  - Stored in TLS as `this_cli` (per-lcore/thread instance).
  - Holds the current working directory, search path, history, environment variables, screen
    state, and registered command maps.

- **Node tree (`struct cli_node`)**
  - The CLI is organized like a filesystem.
  - Node types include: directory, command, file, alias, and string nodes.
  - Nodes are linked in per-directory lists; directories have child lists.

- **Executable search path**
  - Similar to shell `PATH`: the CLI maintains a list of “bin directories” to search for
    commands.

### Key modules

- `cli.c`
  - Core runtime: building/managing nodes, executing commands, history integration, prompt, etc.
  - Contains the **command→map registry** used by map-driven tooling.

- `cli_input.c`, `cli_vt100.c`, `cli_scrn.c`, `cli_gapbuf.c`
  - Terminal I/O, editing primitives, cursor movement, and the gap buffer.

- `cli_search.c`
  - Locating nodes by name/path and enumerating directories.

- `cli_env.c`
  - Environment variables and command-line substitution of `$(VAR)`.

- `cli_map.c`
  - Map (pattern) matching for command variants.

- `cli_auto_complete.c`
  - TAB completion for commands/paths and (optionally) map-driven, context-aware token
    suggestions.

## Creating commands and trees

Most applications build an initial tree using the helper macros defined in `cli.h`:

- `c_dir("/bin")` / `c_bin("/bin")`: create a directory (bin marks it as executable path)
- `c_cmd("show", show_cmd, "...")`: register a command callback
- `c_file("copyright", file_cb, "...")`: file node backed by callback
- `c_alias("ll", "ls -l", "...")`: shell-like alias
- `c_str("FOO", func, "default")`: string node (often used as an env-like value)
- `c_end()`: terminator

A command callback uses an argv-style signature:

```c
int my_cmd(int argc, char **argv);
```

The CLI library does not convert arguments for you. Treat argv tokens as strings and parse
as needed.

## Map-driven parsing (cli_map)

The CLI supports two common ways to interpret a command line:

1. **Direct argv parsing** in your command callback.
2. **Map-driven selection** using `struct cli_map` and `cli_mapping()`.

A map is an array of format strings with an index:

```c
static struct cli_map my_map[] = {
    {10, "show"},
    {20, "show %s"},
    {30, "show %P stats"},
    {40, "show %P %|link|errors|missed stats"},
    {-1, NULL}
};
```

The first map entry whose format matches the user’s `argc/argv` is returned, and the index
is typically used in a switch statement.

### Format tokens

The map format is similar to `printf`, but `%` has CLI-specific meaning:

- `%d`, `%D`, `%u`, `%U`, `%b`, `%n`: numeric placeholders
- `%h`, `%H`: hex placeholders
- `%s`: generic string placeholder
- `%c`: comma-separated list (string)
- `%m`: MAC-like token
- `%4`, `%6`: IPv4/IPv6 token
- `%P`, `%C`: portlist/corelist tokens
- `%k`: key/value argument blob
- `%l`: list string (may require quoting if it contains spaces)
- `%|a|b|c`: fixed choice list (one of the options must match)

See `cli_map.c` for exact validation behavior.

## Auto-complete (TAB)

TAB completion provides two layers:

1. **Shell-like completion** for commands, directories, and files by scanning the CLI tree and
   search path.
2. **Map-driven completion** when a command has a map registered.

### Registering maps for completion

To enable map-driven completion, the CLI needs to know which map applies to a command.
The registry lives in the CLI instance and can be populated via:

- `cli_register_cmd_map(cmd, map)`
- `cli_register_cmd_maps(map)` (registers all command names referenced in the map)

Once registered, `cli_auto_complete.c` can interpret “fixed” tokens (e.g., `tcp`, `dst`, `vlan`)
vs placeholders (e.g., `%d`, `%P`) and offer:

- a list of valid next fixed tokens
- placeholder hints such as `<portlist>` or `<comma-list>` when appropriate

### Placeholder hints

When there is no concrete completion (because the next token is a user-provided value), the
engine may print a *hint* like `<portlist>` or `<ipv4-addr>`.

In general:
- hints are printed when the completion prefix is empty
- hints are not inserted into the input line

## Common debugging tips

- Use the CLI “help” or map dump mechanisms to verify the expected token order.
- If completion suggests an unexpected placeholder hint, it usually means multiple map entries
  appear compatible with the already-typed tokens.
  - Ensure fixed keywords are represented as literal tokens or choice tokens in the map.
  - Ensure placeholders are specific enough (e.g., `%c` for comma-list) rather than `%s`.

## Where to look next

- For adding a new command: locate where the application builds its `cli_tree` and add a
  `c_cmd(...)` entry.
- For a command with multiple syntaxes: add/extend its `struct cli_map[]` and use
  `cli_mapping()` in the callback.
- For completion improvements: start in `cli_auto_complete.c`, then check the command’s map
  registration and token formats.
