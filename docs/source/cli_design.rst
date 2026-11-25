CLI design notes (lib/cli)
==========================

This page documents the design and usage of the CLI library shipped in this
repository under ``lib/cli``.

For a developer-focused walkthrough (with examples and pointers to the
implementation), see the in-tree document:

* ``lib/cli/DESIGN.md``

When the optional ``myst_parser`` Sphinx extension is available, the Markdown
source can also be included directly in the documentation build.

Overview
--------

The CLI is a small shell-like environment with:

* A directory-like node tree (commands, files, aliases, directories)
* History and line editing
* TAB completion
* Optional map-driven parsing for command variants

Key ideas
---------

* The active CLI instance is stored in TLS as ``this_cli``.
* The command tree is made of ``struct cli_node`` entries.
* Commands are implemented as ``int cmd(int argc, char **argv)`` callbacks.
* Command variants can be selected by matching argv against a ``struct cli_map[]``
  table via ``cli_mapping()``.

Auto-complete
-------------

TAB completion is primarily shell-like (commands and paths), but it can also be
context-aware if a command has an associated map table registered via
``cli_register_cmd_map()`` / ``cli_register_cmd_maps()``.

See also
--------

* ``cli.rst`` and ``cli_lib.rst`` in this documentation set
* ``lib/cli/DESIGN.md`` in the source tree
