# Tasks: Basic Chapter Restructure & Learning Progression Fix

**Input**: User decision: Option A (Full Restructure) — reorder all basic chapters to correct C learning progression + split functions/strings/pointers/structs into sub-chapters.

**Target**: ~38 basic files instead of current 27. Updates coordinator, SUMMARY.md, basic_overview.md, all cross-references.

**Convention**: `<module>_sample.c` + `<module>_sample.h` naming, `main_<module>_sample()` entry points.

## Phase 1: Foundational — Analysis & Planning ✅

- [x] T001 [P] Identify current learning order issues (completed via `/speckit.clarify`)
- [x] T002 [P] Define new chapter order with sub-chapter splits (completed via `/speckit.clarify`)

---

## Phase 2: US1 — Reorder Existing Chapters (P1) ✅

**Goal**: Move chapters to correct learning progression. Fixed: operators moved from #4 to #3, control_flow from #6 to #4, loops from #7 to #5, functions from #3 to #6, arrays from #5 to #7. Added stdlib_sample call, removed duplicate callbacks call.

- [x] T003 [P] [US1] Updated ordering in `src/basic/basic.c` with section group comments
- [x] T004 [P] [US1] Updated chapter order in `docs/src/SUMMARY.md` with grouped sections
- [x] T005 [P] [US1] Updated chapter table in `docs/src/basic/basic_overview.md` (correct order)
- [x] T006 [P] [US1] Update `basic.c` section group comments (Core/Memory/Data Structures/Advanced)
- [x] T007 [US1] Update `docs/src/basic/basic_overview.md` with new group headers
- [x] T008 [US1] Update `docs/src/SUMMARY.md` with grouped hierarchy (already done)

**Quality gate**:
- [x] T009 [US1] `make build` zero warnings ✅
- [x] T010 [US1] `make run` executes chapters in correct order ✅

---

## Phase 3: US2 — Split Functions into Sub-Chapter (P1)

**Goal**: Replace single `functions.md` + `functions_sample.{c,h}` with 4 sub-chapters + overview.

New structure (functions → 4 sub-chapters):
- `functions_basics.md` + `functions_basics_sample.{c,h}` — definition, declaration, return types, parameters
- `function_scope.md` + `function_scope_sample.{c,h}` — local/global, static functions, extern, forward declarations
- `recursion.md` + `recursion_sample.{c,h}` — recursive functions, base case, practical examples (factorial, fibonacci)
- `variadic_functions.md` + `variadic_functions_sample.{c,h}` — va_list, printf-style functions

**Pattern**: Same as advance sub-chapter — `src/basic/<topic>_sample.{c,h}`, `docs/src/basic/<topic>.md`

- [ ] T011 [P] [US2] Create `docs/src/basic/functions_basics.md` (15 sections, first-person, real-world analogy)
- [ ] T012 [P] [US2] Create `src/basic/functions_basics_sample.{c,h}` (main_functions_basics_sample + demos)
- [ ] T013 [P] [US2] Create `docs/src/basic/function_scope.md`
- [ ] T014 [P] [US2] Create `src/basic/function_scope_sample.{c,h}`
- [ ] T015 [P] [US2] Create `docs/src/basic/recursion.md`
- [ ] T016 [P] [US2] Create `src/basic/recursion_sample.{c,h}`
- [ ] T017 [P] [US2] Create `docs/src/basic/variadic_functions.md`
- [ ] T018 [P] [US2] Create `src/basic/variadic_functions_sample.{c,h}`
- [ ] T019 [US2] Create `docs/src/basic/functions.md` as overview (intro + table of 4 sub-chapters + links)
- [ ] T020 [US2] Delete old `docs/src/basic/functions.md` (will be replaced by overview)

**Coordinator updates**:
- [ ] T021 [US2] Update `basic.h`: add 4 `main_*_sample()` declarations for functions sub-chapters
- [ ] T022 [US2] Update `basic.c`: add 4 function calls + group under "Functions" section

**Quality gate**:
- [ ] T023 [US2] Verify `make build` zero warnings on all 4 new functions files

---

## Phase 4: US3 — Split Strings into Sub-Chapter (P2)

**Goal**: Replace single `strings.md` with 4 sub-chapters.

New structure (strings → 4 sub-chapters):
- `string_basics.md` + `string_basics_sample.{c,h}` — char arrays, null terminator, string literals
- `string_operations.md` + `string_operations_sample.{c,h}` — `<string.h>` functions: strlen, strcpy, strcat, strcmp
- `safe_strings.md` + `safe_strings_sample.{c,h}` — strncpy, snprintf, bounds checking, overflow prevention (Constitution: no strcpy/sprintf)
- `string_processing.md` + `string_processing_sample.{c,h}` — parsing, tokenizing (strtok), searching (strstr, strchr)

- [ ] T024 [P] [US3] Create `docs/src/basic/string_basics.md` (15 sections, analogy: "C string is like a train where the last car is always '\0'")
- [ ] T025 [P] [US3] Create `src/basic/string_basics_sample.{c,h}`
- [ ] T026 [P] [US3] Create `docs/src/basic/string_operations.md`
- [ ] T027 [P] [US3] Create `src/basic/string_operations_sample.{c,h}`
- [ ] T028 [P] [US3] Create `docs/src/basic/safe_strings.md`
- [ ] T029 [P] [US3] Create `src/basic/safe_strings_sample.{c,h}`
- [ ] T030 [P] [US3] Create `docs/src/basic/string_processing.md`
- [ ] T031 [P] [US3] Create `src/basic/string_processing_sample.{c,h}`
- [ ] T032 [US3] Create `docs/src/basic/strings.md` as overview
- [ ] T033 [US3] Delete old `strings_sample.{c,h}` and `strings.md`

**Coordinator updates**:
- [ ] T034 [US3] Update `basic.h`: add 4 string sub-chapter declarations
- [ ] T035 [US3] Update `basic.c`: add 4 string calls

**Quality gate**:
- [ ] T036 [US3] Verify `make build` zero warnings
- [ ] T037 [US3] Verify safe_strings uses ONLY bounded ops (strncpy, snprintf) — Constitution check

---

## Phase 5: US4 — Split Pointers into Sub-Chapter (P1)

**Goal**: Replace 2 flat pointer chapters with 6 focused sub-chapters.

Current: `pointers.md` + `pointer_arith.md` (2 files)
New: 5 sub-chapters + overview:
- `pointer_basics.md` + `pointer_basics_sample.{c,h}` — &, *, NULL, pointer initialization, "pointers are like GPS coordinates"
- `pointer_arith.md` + `pointer_arith_sample.{c,h}` — existing content, keep as-is
- `pointers_and_arrays.md` + `pointers_and_arrays_sample.{c,h}` — array-pointer equivalence, multidimensional arrays
- `pointers_and_functions.md` + `pointers_and_functions_sample.{c,h}` — pointers as parameters, returning pointers, pass-by-reference
- `void_pointers.md` + `void_pointers_sample.{c,h}` — type erasure, generic functions (moved from existing `void_generic.md`)
- `const_correctness.md` + `const_correctness_sample.{c,h}` — const pointers, pointer to const, const vs non-const

- [ ] T038 [P] [US4] Create `docs/src/basic/pointer_basics.md`
- [ ] T039 [P] [US4] Create `src/basic/pointer_basics_sample.{c,h}`
- [ ] T040 [P] [US4] Rename `docs/src/basic/pointer_arith.md` — update content to include array-pointer section
- [ ] T041 [P] [US4] Create `docs/src/basic/pointers_and_arrays.md`
- [ ] T042 [P] [US4] Create `src/basic/pointers_and_arrays_sample.{c,h}`
- [ ] T043 [P] [US4] Create `docs/src/basic/pointers_and_functions.md`
- [ ] T044 [P] [US4] Create `src/basic/pointers_and_functions_sample.{c,h}`
- [ ] T045 [P] [US4] Move `void_generic.md` → `void_pointers.md` (rename + update content)
- [ ] T046 [P] [US4] Create `docs/src/basic/const_correctness.md`
- [ ] T047 [P] [US4] Create `src/basic/const_correctness_sample.{c,h}`
- [ ] T048 [US4] Create `docs/src/basic/pointers.md` as overview
- [ ] T049 [US4] Delete old `pointers_sample.{c,h}` (merged into sub-chapters)

**Coordinator updates**:
- [ ] T050 [US4] Update `basic.h`: add 5 pointer sub-chapter declarations
- [ ] T051 [US4] Update `basic.c`: add 5 pointer calls

**Quality gate**:
- [ ] T052 [US4] Verify `make build` zero warnings
- [ ] T053 [US4] Verify valgrind on pointer sub-chapters — no invalid reads/writes

---

## Phase 6: US5 — Split Structs into Sub-Chapter (P2)

**Goal**: Replace 2 flat struct chapters with 5 focused sub-chapters.

Current: `structs.md` + `struct_fields.md` (2 files)
New: 4 sub-chapters + overview:
- `struct_basics.md` + `struct_basics_sample.{c,h}` — definition, initialization, member access — analogy: "struct is like a business card"
- `nested_structs.md` + `nested_structs_sample.{c,h}` — struct within struct, complex data types
- `struct_functions.md` + `struct_functions_sample.{c,h}` — passing struct to/returning from functions, struct pointers
- `struct_memory_layout.md` + `struct_memory_layout_sample.{c,h}` — padding, alignment, sizeof, `#pragma pack`
- `unions_and_enums.md` + `unions_and_enums_sample.{c,h}` — union basics, enum patterns, tagged unions with enum discriminant — merges `enums.md` with NEW unions section

- [ ] T054 [P] [US5] Create `docs/src/basic/struct_basics.md`
- [ ] T055 [P] [US5] Create `src/basic/struct_basics_sample.{c,h}`
- [ ] T056 [P] [US5] Create `docs/src/basic/nested_structs.md`
- [ ] T057 [P] [US5] Create `src/basic/nested_structs_sample.{c,h}`
- [ ] T058 [P] [US5] Create `docs/src/basic/struct_functions.md`
- [ ] T059 [P] [US5] Create `src/basic/struct_functions_sample.{c,h}`
- [ ] T060 [P] [US5] Create `docs/src/basic/struct_memory_layout.md`
- [ ] T061 [P] [US5] Create `src/basic/struct_memory_layout_sample.{c,h}`
- [ ] T062 [US5] Create `docs/src/basic/structs.md` as overview

**Coordinator updates**:
- [ ] T063 [US5] Update `basic.h`: add 4 struct sub-chapter declarations
- [ ] T064 [US5] Update `basic.c`: add 4 struct calls

**Quality gate**:
- [ ] T065 [US5] Verify `make build` zero warnings

---

## Phase 7: US6 — Add Missing Chapters and Reorder Remaining (P2)

**Goal**: Add typedef, unions, and reorder remaining chapters into correct groups.

New chapters to create:
- `typedef.md` + `typedef_sample.{c,h}` — type aliases, function pointer typedefs
- `unions.md` merged with enums in US5 above

Remaining flat chapters that stay as-is (just reposition):
21. logging (stays)
22. debugging (stays)
23. headers (stays)
24. conditional_comp (stays)
25. cli_args (stays)
26. stdlib (stays)
27. review_basic (stays)

Position changes:
- typedef → position after pointer sub-chapters (around ~17)
- logging → position after file_io (~22)
- conditional_comp → position after debugging (~24)

- [ ] T066 [P] [US6] Create `docs/src/basic/typedef.md` (15 sections, first-person, analogy)
- [ ] T067 [P] [US6] Create `src/basic/typedef_sample.{c,h}`
- [ ] T068 [US6] Reorder remaining flat chapters in `basic.c` to match learning progression
- [ ] T069 [US6] Update `basic.h` with typedef declaration

**Quality gate**:
- [ ] T070 [US6] Verify all chapter calls in `basic.c` match `basic.h` declarations

---

## Phase N: Polish & Integration

- [ ] T071 [P] Update `docs/src/basic/basic_overview.md` — new chapter table (~38 entries with nested sub-chapter links)
- [ ] T072 [P] Update `docs/src/SUMMARY.md` — nested hierarchy for Functions, Strings, Pointers, Structs groups
- [ ] T073 [P] Update cross-references in all affected docs (continue learning links point to new chapter names)
- [ ] T074 [P] Full build: `make clean && make build` — zero warnings across ALL source files
- [ ] T075 [P] Full run: `make run` — all chapters execute in correct learning order
- [ ] T076 [P] Update README.md — mention new chapter count and structure
- [ ] T077 [P] Update AGENTS.md — new basic chapter structure documented

---

## Dependencies

```
Phase 1 (T001-T002) ✅ DONE — Analysis complete
     │
     └──→ Phase 2 (T003-T010): Reorder existing 8 chapters ← MVP
              │
              ├──→ Phase 3 (T011-T023): Split functions (4 sub-chapters)
              │
              ├──→ Phase 4 (T024-T037): Split strings (4 sub-chapters)
              │
              ├──→ Phase 5 (T038-T053): Split pointers (5 sub-chapters)
              │
              ├──→ Phase 6 (T054-T065): Split structs (4 sub-chapters)
              │
              └──→ Phase 7 (T066-T070): Add typedef + reorder remaining
                      │
                      └──→ Phase N (T071-T077): Polish + Integration
```

## Task Summary

| Phase | Tasks | Status  |
| ----- | ----- | ------- |
| Phase 1: Foundational | T001-T002 | ✅ 2 done |
| Phase 2: US1 (P1) Reorder existing | T003-T010 | ✅ 8 done |
| Phase 3: US2 (P1) Functions split | T011-T023 | ⏳ 13 pending |
| Phase 4: US3 (P2) Strings split | T024-T037 | ⏳ 14 pending |
| Phase 5: US4 (P1) Pointers split | T038-T053 | ⏳ 16 pending |
| Phase 6: US5 (P2) Structs split | T054-T065 | ⏳ 12 pending |
| Phase 7: US6 (P2) Missing chapters | T066-T070 | ⏳ 5 pending |
| Phase N: Polish | T071-T077 | ⏳ 7 pending |
| **Total** | | **77 tasks, 2 done, 75 pending** |

**MVP scope**: Phases 1-2 (reorder existing 8 chapters, minimal file moves) = 10 tasks
**Full scope**: All 77 tasks = ~38 basic files with sub-chapters

## Implementation Strategy

### MVP First (Reorder Only)
1. Complete Phase 2: Move 5 chapters to correct positions (T003-T010)
2. **STOP AND VALIDATE**: `make build && make run` — chapters in correct order
3. Demo: Learner experiences correct C learning progression

### Incremental Delivery
1. MVP: Reorder existing 8 chapters
2. Functions split (4 sub-chapters)
3. Strings split (4 sub-chapters)
4. Pointers split (5 sub-chapters)
5. Structs split (4 sub-chapters)
6. Missing chapters + polish

### Parallel Execution Strategy
- T011-T018 (Functions sub-chapters docs+source) → 4 parallel agents
- T024-T031 (Strings sub-chapters docs+source) → 4 parallel agents
- T038-T047 (Pointers sub-chapters docs+source) → 5 parallel agents
- T054-T061 (Structs sub-chapters docs+source) → 4 parallel agents
