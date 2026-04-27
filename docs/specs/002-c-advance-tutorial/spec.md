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
