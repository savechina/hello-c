# Quickstart: C Advance Tutorial — Subcommand Support#

## Prerequisites#

- Completed: All 27 basic C chapters
- Build tools: `gcc` (macOS) or `clang` (Linux)
- Makefile: `make build`, `make test`, `make run` already working

## New Feature: Subcommand Support#

The `hello` binary now accepts topic-level subcommands, matching `../hello-rust` behavior.

### Usage#

```bash
# Run all sample chapters (default behavior, backward compatible)
./build/bin/hello
# or explicitly:
./build/bin/hello all

# Run all basic chapters only
./build/bin/hello basic

# Run all advance chapters only
./build/bin/hello advance

# List available topics
./build/bin/hello list

# Show usage
./build/bin/hello help
```

### Example Output (`./build/bin/hello list`)#

```
Available topics:
  basic     - 27 basic C chapters (variables, data types, functions, ...)
  advance   - 12 advance C chapters (threads, pointers, error handling, ...)
  module1   - print_hello example
  module2   - print_util example
```

## Implementation Steps#

1. Modify `src/main.c` to parse `argv[1]` and dispatch to subcommands
2. Update `src/basic/basic.c` — add `main_basic_sample(void)` coordinator
3. Update `src/advance/advance.c` — add `main_advance_sample(void)` coordinator
4. Update Makefile help target to show new subcommands
5. Update `docs/src/SUMMARY.md` with subcommand documentation

## Quality Checklist#

- [ ] `make build` zero warnings
- [ ] `./build/bin/hello basic` runs all basic chapters
- [ ] `./build/bin/hello advance` runs all advance chapters
- [ ] `./build/bin/hello list` prints available topics
- [ ] `./build/bin/hello all` / no args runs everything (backward compatible)
- [ ] `./build/bin/hello help` prints usage
- [ ] Bilingual output (Chinese + English terms)
