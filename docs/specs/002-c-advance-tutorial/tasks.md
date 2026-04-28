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

- [x] T001 Replace `src/advance/advance.h` — declares `main_advance()` + all `_sample` entry points for advance chapters
- [x] T002 Replace `src/advance/advance.c` — coordinator calling all `main_<topic>_sample()` functions, similar pattern to `basic.c`
- [x] T003 Verify: `make build` includes advance files via Makefile glob ✅ (already matched by `**/**.c`)

---

## Phase 3: User Story 1 — Advanced Patterns (P1)

**Goal**: 6 core advance chapters covering C patterns: error handling, atomic types, opaque pointers, threads/async, iterators (data structures), advanced traits (function pointer tables).

**Each chapter delivers**:
- `docs/src/advance/<topic>.md` — 15 mandatory sections, real-world analogy opening, error-first learning, bilingual
- `src/advance/<topic>_sample.c` — defines `main_<topic>_sample()` + internal static demos
- `src/advance/<topic>_sample.h` — include guard + declare `main_<topic>_sample()`

- [x] T004 [P] [US1] error-handling (错误处理, 🟡) — errno, setjmp/longjmp. ✅ COMPLETED
- [x] T005 [P] [US1] atomic-types (原子类型, 🟡) — stdatomic.h. ✅ COMPLETED
- [x] T006 [P] [US1] smart-pointers (透明指针, 🔴) — opaque pointers + RAII. ✅ COMPLETED
- [x] T007 [P] [US1] async (异步与线程, 🔴) — POSIX threads → split to 4 sub-chapters: thread, sync, pool, iomux. ✅ COMPLETED
- [x] T008 [P] [US1] iterators (数据结构遍历, 🔴) — linked lists, BST. ✅ COMPLETED
- [x] T009 [P] [US1] advanced-traits (高级多态, 🔴) — vtable pattern. ✅ COMPLETED

**Coordinator updates**:
- [x] T010 [US1] Update `src/advance/advance.h`: add all declarations ✅ COMPLETED
- [x] T011 [US1] Update `src/advance/advance.c`: add all calls ✅ COMPLETED
- [x] T012 [US1] Verify `make build` zero warnings ✅ COMPLETED

---

## Phase 4: User Story 2 — System & Tools (P1)

- [x] T013 [P] [US2] system → split to 6 sub-chapters: file, signal, mmap, process, ipc, cli. ✅ COMPLETED
- [x] T014 [P] [US2] testing (测试框架) ✅ COMPLETED
- [x] T015 [P] [US2] tools (工具链) ✅ COMPLETED

**Coordinator updates**:
- [x] T016 [US2] Update advance.h ✅ COMPLETED
- [x] T017 [US2] Update advance.c ✅ COMPLETED
- [x] T018 [US2] Verify `make build` zero warnings ✅ COMPLETED

---

## Phase 5: User Story 3 — Projects (P2)

- [x] T019 [P] [US3] database (SQLite) ✅ COMPLETED
- [x] T020 [P] [US3] web → split to 2 sub-chapters: socket, concurrent. ✅ COMPLETED
- [x] T021 [ ] Create `docs/src/advance/overview.md` → created ✅ COMPLETED
- [x] T022 [ ] Create `docs/src/advance/review.md` → created ✅ COMPLETED

**Coordinator updates**:
- [x] T023 [US3] Update advance.h ✅ COMPLETED
- [x] T024 [US3] Update advance.c ✅ COMPLETED

**Quality gate**:
- [x] T025 [US3] Verify `make build` zero warnings ✅ COMPLETED
- [x] T026 [US3] Verify valgrind (memory chapters) ✅ COMPLETED

---

## Phase N: Polish & Cross-Cutting

- [x] T027 [P] Update `docs/src/SUMMARY.md` ✅ COMPLETED
- [x] T028 [P] Update `README.md` — mentioned advance tutorial section ✅ COMPLETED (done in 1adf92a)
- [x] T029 [P] Full build: `make clean && make build` zero warnings ✅ COMPLETED
- [x] T030 [P] Valgrind verification ✅ COMPLETED
- [x] T031 [P] Update `src/hello.c` — main_advance() calls coordinator ✅ COMPLETED
- [x] T032 [P] mdBook build: docs compile ✅ COMPLETED
- [x] T033 [P] Template compliance: all chapters have 15 sections ✅ COMPLETED
- [x] T034 [P] Voice consistency: first-person + real analogies ✅ COMPLETED
- [x] T035 End-to-end: `make run` — all chapters execute ✅ COMPLETED

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
| Phase 1: Setup | T001 (no-op) | ✅ Already done |
| Phase 2: Coordinator | T001-T003 | ✅ 3 done |
| Phase 3: US1 (P1) | T004-T012 | ✅ 9 done (async split to 4 sub-chapters) |
| Phase 4: US2 (P1) | T013-T018 | ✅ 6 done (system split to 6 sub-chapters) |
| Phase 5: US3 (P2) | T019-T026 | ✅ 8 done (web split to 2 sub-chapters) |
| Phase N: Polish | T027-T035 | ✅ 9 done |
| **Total** | | **✅ 35/35 ALL COMPLETE** |

**MVP scope**: Phases 2-3 (Coordinator + 6 pattern chapters) = 12 tasks
**Full scope**: All 35 tasks
