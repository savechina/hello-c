# Tasks: C Advance Tutorial Chapters

**Input**: Design documents from `/docs/specs/002-c-advance-tutorial/` (spec.md, plan.md, research.md, data-model.md)
**Prerequisites**: Phase 1 (basic tutorial) ✅ COMPLETE
**Convention**: All source files use `<topic>_sample.c` + `<topic>_sample.h` naming in `src/advance/`. Functions use `main_<topic>_sample()` entry points.

**Path Conventions**:
- Documentation: `docs/src/advance/<topic>.md`
- Source: `src/advance/<topic>_sample.c` + `src/advance/<topic>_sample.h`
- Coordinator: `src/advance/advance.c` + `src/advance/advance.h`
- Config: `Makefile` (already glob-matches `src/advance/**.c`), `docs/src/SUMMARY.md`

---

## Phase 1: Setup ✅ (No actions needed — basic section already complete and `advance/` exists as stub)

## Phase 2: Foundational — Advance Coordinator

**Purpose**: Replace `advance.c/h` stub with coordinator pattern matching `basic.c/h`.

- [ ] T001 Replace `src/advance/advance.h` — declares `main_advance()` + all `_sample` entry points for advance chapters
- [ ] T002 Replace `src/advance/advance.c` — coordinator calling all `main_<topic>_sample()` functions, similar pattern to `basic.c`
- [ ] T003 Verify: `make build` includes advance files via Makefile glob ✅ (already matched by `**/**.c`)

---

## Phase 3: User Story 1 — Advanced Patterns (P1)

**Goal**: 6 core advance chapters covering C patterns: error handling, atomic types, opaque pointers, threads/async, iterators (data structures), advanced traits (function pointer tables).

**Each chapter delivers**:
- `docs/src/advance/<topic>.md` — 15 mandatory sections, real-world analogy opening, error-first learning, bilingual
- `src/advance/<topic>_sample.c` — defines `main_<topic>_sample()` + internal static demos
- `src/advance/<topic>_sample.h` — include guard + declare `main_<topic>_sample()`

- [ ] T004 [P] [US1] error-handling (错误处理, 🟡) — errno pattern, setjmp/longjmp, error callback chains. `docs/src/advance/error_handling.md`, `src/advance/error_handling_sample.{c,h}`
- [ ] T005 [P] [US1] atomic-types (原子类型, 🟡) — C11 `<stdatomic.h>`, volatile basics, memory barriers. `docs/src/advance/atomic_types.md`, `src/advance/atomic_types_sample.{c,h}`
- [ ] T006 [P] [US1] smart-pointers (透明指针, 🔴) — opaque pointers via void*, RAII-style macros for resource cleanup, factory patterns. `docs/src/advance/smart_pointers.md`, `src/advance/smart_pointers_sample.{c,h}`
- [ ] T007 [P] [US1] async (异步与线程, 🔴) — POSIX threads (pthread_create/join), thread safety, mutex, mutex, condition variables. `docs/src/advance/async.md`, `src/advance/async_sample.{c,h}`. **Platform: #ifdef PTHREAD on POSIX**
- [ ] T008 [P] [US1] iterators (数据结构遍历, 🔴) — linked list traversal, dynamic arrays, binary tree in-order traversal. `docs/src/advance/iterators.md`, `src/advance/iterators_sample.{c,h}`
- [ ] T009 [P] [US1] advanced-traits (高级多态, 🔴) — function pointer tables, vtable-like struct patterns, polymorphic dispatch in C. `docs/src/advance/advanced_traits.md`, `src/advance/advanced_traits_sample.{c,h}`

**Coordinator updates**:
- [ ] T010 [US1] Update `src/advance/advance.h`: add 6 `main_*_sample()` declarations for Phase 3
- [ ] T011 [US1] Update `src/advance/advance.c`: add 6 `main_*_sample()` calls

**Quality gate**:
- [ ] T012 [US1] Verify: `make build` zero warnings on all US1 advance source

---

## Phase 4: User Story 2 — System & Tools (P1)

- [ ] T013 [P] [US2] system (系统调用, 🔴) — POSIX signals, mmap, file descriptors, process management. `docs/src/advance/system.md`, `src/advance/system_sample.{c,h}. **Platform: #ifdef POSIX**`. **Platform: #ifdef POSIX**`. **Platform: #ifdef POSIX**
- [ ] T014 [P] [US2] testing (测试框架, 🟡) — custom ASSERT macros, test harness, mock functions. `docs/src/advance/testing.md`, `src/advance/testing_sample.{c,h}`
- [ ] T015 [P] [US2] tools (工具链, 🟢) — build system patterns, code coverage (gcov/lcov), static analysis (cppcheck), CI for C. `docs/src/advance/tools.md`, `src/advance/tools_sample.{c,h}`

**Coordinator updates**:
- [ ] T016 [US2] Update `src/advance/advance.h`: add 3 `main_*_sample()` declarations for Phase 4
- [ ] T017 [US2] Update `src/advance/advance.c`: add 3 `main_*_sample()` calls

**Quality gate**:
- [ ] T018 [US2] Verify: `make build` zero warnings

---

## Phase 5: User Story 3 — Projects (P2)

- [ ] T019 [P] [US3] database (数据库, 🟡) — SQLite3 C API: open, CREATE, INSERT, SELECT, prepared statements. `docs/src/advance/database.md`, `src/advance/database_sample.{c,h}`. **Dependency: SQLite3 installed (`#include <sqlite3.h>`)**
- [ ] T020 [P] [US3] web (HTTP 服务器, 🔴) — bare-bones HTTP server: socket bind, listen, accept, parse request, send response. `docs/src/advance/web.md`, `src/advance/web_sample.{c,h}`
- [ ] T021 [ ] Create `docs/src/advance/advance-overview.md` — landing page: chapter table, learning path diagram.
- [ ] T022 [ ] Create `docs/src/advance/review-advance.md` — comprehensive review of all 12 advance topics.

**Coordinator updates**:
- [ ] T023 [US3] Update `src/advance/advance.h`: add 2 `main_*_sample()` declarations for Phase 5
- [ ] T024 [US3] Update `src/advance/advance.c`: add 2 `main_*_sample()` calls

**Quality gate**:
- [ ] T025 [US3] Verify: `make build` zero warnings
- [ ] T026 [US3] Verify: `valgrind --leak-check=full build/advance/*_sample` — zero leaks (memory-mgmt, system chapters)

---

## Phase N: Polish & Cross-Cutting

- [ ] T027 [P] Update `docs/src/SUMMARY.md` — add Advance section hierarchy: overview → 12 chapters → review.
- [ ] T028 [P] Update `README.md` — mention advance tutorial section.
- [ ] T029 [P] Full build: `make clean && make build` — zero warnings across ALL source files (basic + advance).
- [ ] T030 [P] Valgrind: `memory_mgmt_sample` chapter under `valgrind --leak-check=full` — zero leaks.
- [ ] T031 [P] Update `src/hello.c` if needed: ensure `main_advance()` calls coordinator properly.
- [ ] T032 [P] mdBook build: `mdbook build docs` — verify all advance links resolve.
- [ ] T033 [P] Template compliance: all 12 advance chapters have 15 mandatory sections.
- [ ] T034 [P] Voice consistency: first-person explanations + real-world analogy openings.
- [ ] T035 End-to-end: `make run` — all chapters (basic + advance) print expected output.

---

## Dependencies

```
Phase 1 (Setup ✅) → Phase 2 (Coordinator: T001-T003)
                          │
                          ├──→ Phase 3 (US1: 6 chapters + T010-T011 + T012) ← MVP advance
                          │       (error-handling, atomic-types, smart-pointers, async, iterators, advanced-traits)
                          │
                          ├──→ Phase 4 (US2: 3 chapters + T016-T017 + T018)
                          │       (system, testing, tools)
                          │
                          ├──→ Phase 5 (US3: 2 chapters + docs + T023-T024 + quality gate)
                          │       (database, web, advance-overview, review-advance)
                          │
                          └──→ Phase N (Polish + Integration)
```

## Task Summary

| Phase | Tasks | Status |
|-------|-------|--------|
| Phase 1: Setup | T001 (no-op) | ✅ Already exists |
| Phase 2: Coordinator | T001-T003 | ⏳ 3 pending |
| Phase 3: US1 (P1) | T004-T012 | ⏳ 9 pending |
| Phase 4: US2 (P1) | T013-T018 | ⏳ 6 pending |
| Phase 5: US3 (P2) | T019-T026 | ⏳ 8 pending |
| Phase N: Polish | T027-T035 | ⏳ 9 pending |
| **Total** | | **35 pending** |

**MVP scope**: Phases 2-3 (Coordinator + 6 pattern chapters) = 12 tasks
**Full scope**: All 35 tasks
