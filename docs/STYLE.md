# Documentation & Markdown Style Guide

This guide summarizes the conventions enforced (and encouraged) for all Markdown and docs in this repository.
It complements `CONTRIBUTING.md` and the root `.markdownlint.yml` configuration.

## 1. Linting

- All Markdown must pass `markdownlint` (locally via the pre-commit hook and in CI).
- Disabled rule: MD013 (line length) – long lines allowed where readability (tables / URLs / command lines) benefits.
- Do not locally override rules unless absolutely necessary; prefer restructuring content.

## 2. Headings

- Use ATX (`#`, `##`, `###`) style only.
- Start at a single `#` per file (title) and increment by one level; avoid skipping levels.
- Surround each heading with exactly one blank line above and below.
- Avoid trailing punctuation (no final colon, period, or question mark) unless part of a proper noun.

## 3. Lists

- Ordered lists: use `1.` for every item (markdownlint will auto-render numbers).
- Unordered lists: use `-` (not `*` or `+`).
- Put a blank line before and after a list block (unless tightly bound to a parent list item sub-block).
- Indent nested list content by two spaces.

## 4. Code Blocks

- Always use fenced blocks (backticks). Indented code blocks are not allowed.
- Supply a language hint whenever possible (`bash`, `python`, `lua`, `text`, `console`).
- Surround fenced blocks with one blank line before and after.
- For shell commands, omit the leading `$` unless demonstrating interactive copy/paste prevention; if shown, also show output to satisfy MD014.
- Use `text` for pseudo-code or command output with mixed content.

## 5. Inline Formatting

- Wrap bare URLs in angle brackets `<https://example.com>`.
- Use backticks for filenames, commands, CLI flags, environment variables, and literal code tokens.
- Prefer emphasis (`*italic*`) sparingly; bold only for strong warnings or section callouts.

## 6. Tables

- Surround tables with one blank line above and below.
- Keep header separator row aligned but do not over-focus on spacing (lint does not require column width alignment).
- Use backticks for code-like cell content.

## 7. Blockquotes

- No blank lines inside a single logical blockquote group.
- Each quoted paragraph or list line should start with `>`.
- Use blockquotes for external notices, important upstream references, or migration notes—not for styling.

## 8. Line Length & Wrapping

- Long lines are acceptable (MD013 disabled). Do not hard-wrap paragraphs unless clarity significantly improves.
- Keep tables and link references on single lines when feasible.

## 9. File Naming & Placement

- Use `ALL_CAPS.md` for top-level meta-docs (README, LICENSE, CONTRIBUTING, INSTALL).
- Place style / guide / topic-specific docs under `docs/`.
- Use descriptive filenames (e.g. `QUICKSTART.md`, `STYLE.md`).

## 10. Tone & Clarity

- Prefer imperative, concise wording ("Run the script" vs. "You should run").
- Avoid marketing language—be factual and actionable.
- Provide context before commands; describe what a block does if non-obvious.

## 11. Common Patterns

| Pattern | Example |
|---------|---------|
| Command sequence | `meson setup builddir && meson compile -C builddir` |
| Config file snippet | ``setup = { 'devices': ('81:00.0',) }`` |
| Shell pipeline | ``echo \"test\" \| socat - TCP4:localhost:22022`` |
| Lua script | ``printf("Hello!\n")`` |
| Output capture | ``Pktgen Version: 25.08.0`` |

## 12. Adding New Docs

- Link new end-user docs from the main `README.md` or an appropriate existing doc section.
- Keep `QUICKSTART.md` focused—avoid duplicating deep details already covered elsewhere.
- When adding a new feature, include: purpose, quick example, limitations, and any performance considerations.

## 13. Do / Avoid Summary

| Do | Avoid |
|----|-------|
| Provide a runnable example for new features | Giant monolithic examples without explanation |
| Use consistent fenced code | Mixing indentation and fenced styles |
| Reference existing sections instead of duplicating content | Copy-pasting large README segments |
| Keep tables compact and scannable | Over-formatting table spacing |
| Add language hints to all code blocks | Leaving unlabeled fences |

## 14. Exceptions

Rare cases (generated content, pasted command output, license headers) may intentionally violate some guidelines—these should be minimal and explained in a comment or preceding sentence.

---
Questions or suggestions? Update this file or open an issue.
