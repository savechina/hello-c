# Research: C Advance Tutorial Subcommand Support

## Decision: Hello Binary Subcommand Architecture

**Decision**: Implement subcommand support in `hello` binary matching `../hello-rust` pattern: `hello basic`, `hello advance`, `hello list`, `hello all` (explicit `all` preserves current behavior).

**Rationale**:
- Matches `../hello-rust` CLI convention (topic-level subcommands)
- Preserves backward compatibility: `./hello` (no args) = `./hello all`
- Clear learning path: `basic` → `advance` → `list` (topic discovery)
- Minimal diff: modify `main.c` + `src/advance.c` dispatcher, add `src/basic/basic.c` dispatcher

**Alternatives Considered**:
1. **Directory-level subcommands** (`hello module1`, `hello module2`) — rejected: too granular, doesn't match learning path
2. **No subcommands** (current behavior: always run all) — rejected: user explicitly requested subcommand support
3. **`hello list` only** — rejected: user wants `basic` + `advance` topic-level control

## Decision: Subcommand Implementation Pattern

**Decision**: Use `argv[1]` string matching in `main.c` to dispatch to topic-level functions:
- `hello basic` → `main_basic_sample(void)` (coordinator for all basic chapters)
- `hello advance` → `main_advance_sample(void)` (coordinator for all advance chapters)
- `hello list` → Print available topics (basic chapters + advance chapters + modules)
- `hello all` / no args → Run everything (current behavior)
- `hello help` → Print usage

**Rationale**:
- Simple C pattern: `if (strcmp(argv[1], "basic") == 0) { main_basic_sample(); }`
- No external dependencies (no getopt, no CLI framework)
- Matches "boring by default" + "minimal diff" preferences
- Tutorial-appropriate: learners see raw CLI parsing before using getopt later

**Alternatives Considered**:
1. **getopt/getopt_long** — rejected: adds POSIX dependency, overkill for tutorial
2. **Subcommand table (function pointer array)** — rejected: premature abstraction for this scope
3. **Split into separate binaries** (`hello-basic`, `hello-advance`) — rejected: project convention is single `hello` binary

## Decision: List Output Format

**Decision**: `hello list` prints:
```
Available topics:
  basic     - 27 basic C chapters (variables, data types, functions, ...)
  advance   - 12 advance C chapters (threads, pointers, error handling, ...)
  module1   - print_hello example
  module2   - print_util example
```

**Rationale**:
- Self-documenting: users see what's available + brief description
- Bilingual: Chinese descriptions (matched to `docs/src/basic/basic_overview.md`)
- Matches `../hello-rust` list output format

**Alternatives Considered**:
1. **JSON output** — rejected: tutorial project, no JSON parser in C
2. **Just topic names** — rejected: not enough context for learners

## Resolved Unknowns from Technical Context

| Unknown | Status | Resolution |
|---------|--------|-------------|
| Subcommand scope | Resolved | Topic-level: `basic`, `advance`, `list`, `all` |
| Implementation pattern | Resolved | `argv[1]` string matching in `main.c` |
| List format | Resolved | Plain text with bilingual descriptions |
| Performance goals | Resolved | N/A for CLI (human-time interaction) |
| Scale/scope | Resolved | 4 subcommands, ~100 lines new code |

## Learnings from `../hello-rust`

- `hello-rust` uses clap derive macros (Rust) → C equivalent is manual `argv` parsing
- `hello-rust` topics map to directories: `basic/`, `advance/`, `api/` → C project same structure
- `hello-rust` `list` shows descriptions → C version will hardcode (no external config file)
