# Feature Specification: C Advance Tutorial

**Feature Branch**: `002-c-advance-tutorial`
**Created**: 2026-04-27
**Status**: Draft

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Advance C Topics Tutorial (P1)

A learner who completed basic section reads advance chapters (threads, opaque pointers, error patterns, etc.), follows runnable examples, and builds advanced C skills.

**Independent Test**: `make build` compiles all advance sample files with zero warnings. Each chapter's mdBook doc links to working source code.

### User Story 2 — Advanced Practice Projects (P2)

Mini-projects combine multiple advance concepts: linked list library, thread pool, simple HTTP server, SQLite integration.

**Independent Test**: Each project compiles and runs correctly, passes valgrind with zero leaks.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 12 advance chapters covering: error-handling, atomic-types, smart-pointers (opaque pointers), async (threads), iterators (data structures), advanced-traits (function pointer tables), system (POSIX calls), testing (ASSERT framework), tools (build/coverage), database (SQLite C API), web (HTTP server), review-advance
- **FR-002**: Each chapter: `_sample.c` + `_sample.h` in `src/advance/`, mdBook doc in `docs/src/advance/`
- **FR-003**: All code compiles `-Wall -Wextra -Werror -std=c17`
- **FR-004**: `advance.c` coordinator calls all `main_<topic>_sample()` functions
- **FR-005**: Zero valgrind errors on all advance code
- **FR-006**: bilingual Chinese + English, first-person voice, real-world analogy openings
- **FR-007**: All internal (static) demo functions named with `_sample` suffix: `<module>_<feature>_sample`. Entry points are `main_<module>_sample(void)`. No `demo_*` function names allowed.

## Success Criteria *(mandatory)*

- **SC-001**: All advance source compiles with zero warnings
- **SC-002**: 12+ advance chapters with full documentation
- **SC-003**: All advance code passes valgrind with zero leaks
- **SC-004**: Advance section has overview + review chapters

## Assumptions

- POSIX threading available on macOS/Linux
- Target audience has completed all 27 basic chapters
- SQLite available via package manager

## Clarifications

### Session 2026-04-27

- Q: Advance scope? → A: Map hello-rust advance (async, atomics, smart-pointers, iterators, traits, error-handling, database, system, testing, tools, web) to C equivalents using C paradigms.
- Q: Function naming for sample code → A: ALL internal (static) functions use `_sample` suffix pattern: `<module>_<feature>_sample`. No `demo_*` naming convention. Entry points: `main_<module>_sample(void)`. Applied across all 20 advance sample files.
- Q: Basic section learning progression restructure → A: Option A (Full Restructure) — Reorder ALL basic chapters to correct C learning progression + split functions/strings/pointers/structs into sub-chapters. Results in ~38 basic files instead of 27. Updates coordinator, SUMMARY.md, basic_overview.md, and all cross-references. New correct order:
  Core: variables → datatype → operators → control_flow → loops → functions → arrays → preprocessor
  Memory: pointers → pointer_arith → pointers_functions → void_pointers → const_correctness
  Data: strings (basics, ops, safe, processing) → structs (basics, nested, functions, memory layout) → unions_enums → typedef
  Advanced: memory_mgmt → file_io → function_pointers → callbacks → headers → debugging → conditional_comp → bit_ops → cli_args → stdlib
  Each split topic gets its own `_sample.c/.h` files following the advance sub-chapter pattern.

### Session 2026-05-02

- Q: How to integrate Unity/CMock? → A: Option A — Copy Unity (3 source files) + CMock source into repo. Suggested directory: `include/` or `third_party/`.
- Q: Replace or coexist with custom testing framework? → A: Option B — Keep custom framework as-is for educational value; add separate `test/` directory with Unity sample test.
- Q: What should the Unity sample test case test? → A: Option A — Test `calc_add()` from `testing_sample.c`, showing migration from custom `ASSERT_EQ_RUN()` to Unity `TEST_ASSERT_EQUAL_INT()`.
- Q: CMock usage in sample? → A: Option B — Unity-only for the sample test case; CMock source added to repo for educational completeness, demonstrated separately.
- Q: Makefile integration (`make test` target)? → A: Option A — Add `make test` target that compiles and runs Unity test(s) from `test/`.

### Session 2026-05-03

- Q: What subcommands should hello binary support initially? → A: Option B — `basic`, `advance`, `list`, `all` (explicit `all` preserves current behavior, adds topic-level subcommands).
