# Contributing to Pktgen-DPDK

Thank you for your interest in improving Pktgen-DPDK. This short guide complements the main `README.md` and highlights the key expectations for contributions.

## 1. Before You Start

- Skim the project overview and feature list in the README.
- Make sure you can build and run the tool (see "Quick Start" and "Clone and Build" sections).
- Search existing issues and pull requests to avoid duplication.

## 2. Development Environment

- Build system: Meson/Ninja (optional simple `Makefile` wrapper provided).
- Required: A recent DPDK build with a valid `libdpdk.pc` on your `PKG_CONFIG_PATH`.
- Recommended: Enable the pre-commit hook for markdown linting:
  - `ln -sf ../../.githooks/pre-commit .git/hooks/pre-commit && chmod +x .githooks/pre-commit`

## 3. Coding & Style

- Follow existing C coding patterns; prefer readability and minimal global state changes.
- Keep commits focused; one logical change per commit when possible.
- Document non-obvious code with concise comments.
- Avoid introducing new external dependencies without discussion.

## 4. Documentation Standards

- All Markdown must pass `markdownlint` (CI will enforce).
- Use fenced code blocks with language hints.
- Wrap bare URLs in `< >`.
- Numbered lists may all use `1.` style.
- If adding new user-facing features, update the relevant README section or create a doc under `docs/`.
- Refer to the extended style guidance in [`docs/STYLE.md`](./docs/STYLE.md) for formatting patterns and examples.

## 5. Commit Messages

Use the following structure:

```text
<short summary imperative>

<context / motivation>
<what changed>
<any impact / migration notes>
```

Examples:

```text
stats: fix link status refresh on state change

Previously link duplex/speed changes were ignored unless carrier toggled. Update
logic to trigger display refresh on any field difference.
```

## 6. Submitting Pull Requests

- Fork, branch from `main`.
- Rebase (not merge) to update your branch before final push.
- Ensure `meson compile` succeeds and (if applicable) tests or sample scripts still run.
- CI must be green (markdownlint, build, etc.).
- Reference related issues with `Fixes #NNN` when appropriate.

## 7. Reporting Issues

Include (when relevant):

- Pktgen version (`cat VERSION`)
- DPDK version and configuration
- NIC model(s) and driver versions
- Reproduction steps and minimal config (`cfg/` file if relevant)
- Expected vs. actual behavior

## 8. Performance / Feature Discussions

Open an issue first for large changes (subsystems, new protocol modules, architectural refactors). Describe:

- Problem statement / use case
- Proposed design sketch
- Anticipated impacts (performance, config format, user CLI)

## 9. Security

Do not publicly disclose potential security-impacting bugs without prior coordination—open a private issue if possible or contact maintainers directly.

See also the community standards in the [`CODE_OF_CONDUCT.md`](./CODE_OF_CONDUCT.md).

## 10. Licensing

By contributing you agree your changes are under the existing project license (BSD-3-Clause). Ensure any new files include the appropriate SPDX identifier header where applicable.

## 11. Quick Reference

| Area | Where to Look |
|------|---------------|
| Build & Run | README Quick Start / Build sections |
| Config examples | `cfg/` directory |
| Lua automation | README Automation section / `test/*.lua` |
| Themes | `themes/` directory |
| Plugins | `lib/plugin/` |

---
Questions? Open an issue or start a discussion. Happy hacking!
