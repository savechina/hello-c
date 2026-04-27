# Implementation Plan: C Advance Tutorial

**Branch**: `002-c-advance-tutorial` | **Date**: 2026-04-27 | **Spec**: spec.md

## Summary

Build 12 advance C tutorial chapters mirroring hello-rust's advance structure, with mdBook docs and runnable `_sample.c`/`_sample.h` source files in `src/advance/`.

## Technical Context

**Language/Version**: C17, gcc 12+ or clang 15+
**Primary Dependencies**: POSIX C (pthread, signal), C11 stdatomic.h, SQLite3 (database chapter)
**Storage**: N/A — tutorial code examples (SQLite demo uses temp files)
**Testing**: `make advance` runs coordinator, individual `make sample CHAPTER=<name>`, valgrind clean required
**Target Platform**: macOS (pthread available), Linux (full POSIX)
**Constraints**: `-Wall -Wextra -Werror -std=c17`, zero memory leaks, bounded string ops only
**Scale/Scope**: 12 chapters → 24 source files + 14 mdBook docs + coordinator

## Constitution Check

- [ ] **I. Memory Safety**: All advance samples must valgrind-clean, RAII-style macros for resource cleanup
- [ ] **II. Code Quality**: C17 standard, zero warnings, include guards, Doxygen on coordinator
- [ ] **III. Modular Architecture**: Each chapter is single-responsibility module, coordinator in `advance.c`
- [ ] **IV. Multi-Platform Portability**: `#ifdef` for thread APIs (pthread on POSIX), fallback for platforms without stdatomic.h
- [ ] **V. SDD Workflow**: Spec ✓, plan ✓, tasks TBD

## Project Structure

```
src/advance/
├── advance.c                 # Coordinator: calls all main_*_sample()
├── advance.h
├── error_handling_sample.{c,h}
├── atomic_types_sample.{c,h}
├── smart_pointers_sample.{c,h}
├── async_sample.{c,h}
├── iterators_sample.{c,h}
├── advanced_traits_sample.{c,h}
├── system_sample.{c,h}
├── testing_sample.{c,h}
├── tools_sample.{c,h}
├── database_sample.{c,h}
├── web_sample.{c,h}
docs/src/advance/
├── advance-overview.md
├── error-handling.md
├── atomic-types.md
├── smart-pointers.md
├── async.md
├── iterators.md
├── advanced-traits.md
├── system.md
├── testing.md
├── tools.md
├── database.md
├── web.md
└── review-advance.md
```

## Phase 0: Research — hello-rust Advance Mapping

Need to map each hello-rust advance topic to C equivalent:
1. `smart-pointers` → Opaque pointers + RAII macros
2. `atomic-types` → C11 `<stdatomic.h>` + volatile
3. `async/` → POSIX threads (pthread), select/poll
4. `iterators` → Linked lists, dynamic arrays, trees in C
5. `advanced-traits` → Function pointer tables, vtable pattern
6. `error-handling` → errno, setjmp/longjmp, error callbacks
7. `system/` → POSIX syscalls: signals, mmap, process mgmt
8. `testing/` → Custom ASSERT framework
9. `database/` → SQLite3 C API basics
10. `web/` → Bare-bones HTTP server with sockets
11. `tools/` → Build system patterns, code coverage
12. `review-advance` → Comprehensive review chapter

## Phase 1: Design

**Coordinator**: `advance.c` follows same pattern as `basic.c` — declares and calls all `main_<topic>_sample()` functions sequentially.
**Makefile**: Update `SOURCES` glob to pick up `src/advance/**.c` (already does via `**/**.c` pattern — `advance/` is already included).
**Makefile**: Add `make advance` target (same as `make sample` but for advance section).

## Phase 2: Implementation Phases

### Phase N: Polish & Cross-Cutting Concerns

1. Update `docs/src/SUMMARY.md` — add Advance section hierarchy
2. Update `README.md` — mention advance tutorial section
3. Update `src/hello.c` — ensure `main_advance()` calls coordinator properly
4. Full build + valgrind verification
