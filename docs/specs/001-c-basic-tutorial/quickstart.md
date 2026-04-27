# Quickstart: C Basic Tutorial Development

**Feature**: 001-c-basic-tutorial
**Date**: 2026-04-26

## Prerequisites

- GCC 12+ (Linux) or Clang 15+ (macOS)
- GNU Make 3.81+
- mdBook (`cargo install mdbook`)
- `valgrind` (Linux) or `Instruments` (macOS) for memory checking
- `cppcheck` for static analysis

## Build the Tutorial Code

```bash
# Clone and build
cd hello-c
make build

# Run all examples
make run

# Clean build artifacts
make clean
```

## Build a Single Standalone Chapter

```bash
# Compile and run one chapter independently
make sample CHAPTER=pointers

# Build all sample chapters at once
make sample-all
```

## Build the Documentation

```bash
# Serve mdBook locally (opens browser)
cd docs && mdbook serve --open
```

## Add a New Chapter

1. Create documentation: `docs/src/basic/<topic>.md`
2. Update `docs/src/basic/basic-overview.md` table
3. Update `docs/src/SUMMARY.md` sidebar
4. Create source code: `src/basic/<topic>.c`, `src/basic/<topic>.h`
5. Add `main_<topic>()` declaration to `basic.h`
6. Add `main_<topic>()` call in `basic.c`
7. Create sample: `src/basic/<topic>_sample.c`
8. Verify: `make build && make sample CHAPTER=<topic>`
9. Check: `valgrind --leak-check=full build/sample/<topic>`
10. Check: `cppcheck --enable=all src/basic/<topic>.c`

## Quality Checklist

- [ ] `make build` — zero warnings
- [ ] `valgrind --leak-check=full` — zero leaks (memory chapters)
- [ ] `cppcheck --enable=all` — zero warnings
- [ ] mdBook chapter has all 15 mandatory sections
- [ ] First-person narrative voice ("我发现", "我的理解")
- [ ] Bilingual: Chinese primary + English in parentheses
- [ ] Code examples compile with `-Wall -Wextra -Werror -std=c17`
- [ ] Memory examples: malloc/free paired, NULL-checked
