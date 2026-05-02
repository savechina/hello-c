# Tasks: Add Unity & CMock for Test Sample

**Input**: Design documents from `/docs/specs/002-c-advance-tutorial/`
**Feature**: Add Unity v2.6.1 and CMock v2.6.0, implement one Unity sample test case for `calc_add()`

**Tests**: Tests are included (Unity test cases) as explicitly requested.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Download external dependencies and create directory structure.

- [x] T001 [P] Create `test/` directory structure: `test/vendor/unity/`, `test/vendor/cmock/lib/`, `test/mocks/`
- [x] T002 [P] Download Unity v2.6.1 sources to `test/vendor/unity/`:
  - `curl -L -o /tmp/unity.zip https://github.com/ThrowTheSwitch/Unity/archive/refs/tags/v2.6.1.zip`
  - Extract `unity.c`, `unity.h`, `unity_internals.h` to `test/vendor/unity/`
- [x] T003 [P] Download CMock v2.6.0 sources to `test/vendor/cmock/`:
  - `curl -L -o /tmp/cmock.zip https://github.com/ThrowTheSwitch/CMock/archive/refs/tags/v2.6.0.zip`
  - Extract `lib/` directory (all `.rb` files) to `test/vendor/cmock/`
- [x] T004 [P] Update `.gitignore` to include `test/mocks/` (generated mocks should be gitignored)

**Checkpoint**: Unity/CMock sources available in `test/vendor/`, directory structure ready.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Extract testable functions from `testing_sample.c` so they can be tested with Unity.

**⚠️ CRITICAL**: Must complete before User Story 1 — `calc_add()` is `static` in `testing_sample.c` and cannot be directly tested.

- [x] T005 Create `src/advance/calc.c` with non-static functions: `calc_add()`, `calc_multiply()`, `calc_is_valid()` (extracted from `testing_sample.c` lines 253-269)
- [x] T006 Create `src/advance/calc.h` declaring: `int calc_add(int a, int b)`, `int calc_multiply(int a, int b)`, `int calc_is_valid(int result)`
- [x] T007 Update `src/advance/testing_sample.c`:
  - Add `#include "advance/calc.h"`
  - Replace inline `calc_add()`, `calc_multiply()`, `calc_is_valid()` implementations with calls to the new `calc_*` functions (or remove duplicates and use the new functions directly)
  - Ensure `testing_sample.c` still compiles and runs correctly
- [x] T008 [P] Update `src/advance/advance.h` — add `#include "advance/calc.h"` if needed for coordination

**Quality Gate**:
- [x] T009 `make build` — zero warnings with new `calc.c/calc.h` files
- [x] T010 `make run` — `testing_sample` chapter still works correctly

---

## Phase 3: User Story 1 — Unity Sample Test Case (P1) 🎯 MVP

**Goal**: Add Unity test framework and implement one sample test case for `calc_add()` in `test/test_calc_add.c`.

**Independent Test**: `make test` compiles and runs `test_calc_add` with all tests passing (green output).

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [x] T011 [P] [US1] Create `test/advance/test_calc_add.c` with Unity test framework (moved from `test/` root):
  - Include `unity.h` and `advance/calc.h`
  - Implement `setUp()` and `tearDown()` (can be empty for this test)
  - Write `test_calc_add_basic()`: `TEST_ASSERT_EQUAL_INT(5, calc_add(2, 3))`, `TEST_ASSERT_EQUAL_INT(0, calc_add(0, 0))`, `TEST_ASSERT_EQUAL_INT(0, calc_add(-1, 1))`
  - Write `test_calc_add_negative()`: `TEST_ASSERT_EQUAL_INT(-1, calc_add(-2, 1))`, `TEST_ASSERT_EQUAL_INT(-5, calc_add(-2, -3))`
  - Implement `main()` with `UNITY_BEGIN()` → `RUN_TEST()` → `UNITY_END()`
  - **Verify**: Compilation fails or tests fail before Makefile integration (manual compile test)

### Implementation for User Story 1

- [x] T012 [US1] Update `Makefile` — add test configuration section:
  - `TEST_DIR := test`
  - `UNITY_DIR := $(TEST_DIR)/vendor/unity`
  - `UNITY_OBJ := $(BUILD_DIR)/unity.o`
  - `TEST_CFLAGS := $(CFLAGS) -I$(UNITY_DIR) -DUNITY_OUTPUT_COLOR -DUNITY_SUPPORT_VARIADIC_MACROS`
  - `TEST_SOURCES := $(wildcard $(TEST_DIR)/*.c)`
  - `TEST_BINS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/test/%, $(TEST_SOURCES))`

- [x] T013 [US1] Update `Makefile` — add Unity compilation rule:
  - `$(UNITY_OBJ): $(UNITY_DIR)/unity.c | $(BUILD_DIR)` — compile unity.c once
  - `$(BUILD_DIR)/test/%: $(TEST_DIR)/%.c $(UNITY_OBJ) | $(BUILD_DIR)/test` — compile and link each test

- [x] T014 [US1] Update `Makefile` — add `test` target:
  ```makefile
  .PHONY: test
  test: $(TEST_BINS)
  	@failed=0; \
  	for t in $(TEST_BINS); do \
  		./$$t || { failed=1; }; \
  	done; \
  	[ $$failed -eq 0 ]
  ```

- [x] T015 [US1] Update `Makefile` — add `$(BUILD_DIR)/test` directory creation rule (order-only prerequisite)

- [x] T016 [US1] Update `Makefile` — add test artifacts to `clean` target: `rm -rf $(BUILD_DIR)/unity.o $(BUILD_DIR)/test/`

**Quality Gate**:
- [x] T017 [US1] `make test` — runs `test_calc_add` and outputs colored PASS/FAIL
- [x] T018 [US1] `make build` — still compiles all `src/` files with zero warnings (Unity not linked to main binary)
- [x] T019 [US1] Verify test output shows `test_calc_add_basic` and `test_calc_add_negative` with PASS

---

## Phase 4: User Story 2 — CMock Education (P2)

**Goal**: Demonstrate CMock usage in the tutorial (separate from sample test case).

**Independent Test**: `test/mocks/mock_sensor.h` generated successfully from a header file.

- [ ] T020 [P] [US2] Create `docs/src/advance/testing.md` section on CMock:
  - Explain CMock is a Ruby-based mock generator
  - Show command: `ruby test/vendor/cmock/lib/cmock.rb -otest/mocks src/advance/sensor.h`
  - Explain difference between Unity (test runner) and CMock (mock generator)

- [ ] T021 [US2] Update `testing_sample.c` — add comment in Section 8 (Mock functions):
  - Reference that CMock can generate mocks automatically for headers like `sensor.h`
  - Keep existing manual function pointer mocking as educational contrast

**Quality Gate**:
- [ ] T022 [US2] `make build` — zero warnings (CMock is Ruby, not compiled)
- [ ] T023 [US2] Verify `ruby test/vendor/cmock/lib/cmock.rb --help` runs (if Ruby available)

---

## Phase N: Polish & Cross-Cutting Concerns

**Purpose**: Documentation updates and final integration.

- [ ] T024 [P] Update `docs/src/advance/testing.md` — add Unity section:
  - Explain Unity vs custom `ASSERT_EQ_RUN()` (side-by-side comparison)
  - Show `test/test_calc_add.c` as the Unity example
  - Explain `make test` target for running tests

- [ ] T025 [P] Update `README.md` — add `make test` to Quick Start section

- [ ] T026 [P] Update `AGENTS.md` — add Unity/CMock to Active Technologies section

- [ ] T027 [P] Update `docs/specs/002-c-advance-tutorial/quickstart.md` — verify "Add Test with Unity" section matches implementation

- [ ] T028 Create `test/README.md` — document:
  - How to add new test files (create `test/test_*.c`, it auto-discovers via Makefile wildcard)
  - Unity flags used (`-DUNITY_OUTPUT_COLOR`, etc.)
  - How to generate mocks with CMock

**Final Quality Gate**:
- [ ] T029 `make build` — zero warnings across ALL source files
- [ ] T030 `make test` — all Unity tests PASS
- [ ] T031 `make run` — all advance chapters (including testing_sample) work correctly
- [ ] T032 `make clean && make build` — clean build succeeds

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — can start immediately ✅
- **Phase 2 (Foundational)**: Depends on Phase 1 — **BLOCKS all user stories**
- **Phase 3 (US1)**: Depends on Phase 2 completion — **MVP**, can run independently after Phase 2
- **Phase 4 (US2)**: Depends on Phase 2 completion — can run in parallel with US1 (different files)
- **Phase N (Polish)**: Depends on US1 + US2 completion

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Phase 2 — No dependencies on US2
- **User Story 2 (P2)**: Can start after Phase 2 — No dependencies on US1

### Within Each User Story

- Tests (T011) → Implementation (T012-T016) → Quality Gate (T017-T019)
- Models before integration

### Parallel Opportunities

- Phase 1: T001, T002, T003, T004 can ALL run in parallel (different files, no deps)
- Phase 2: T005, T006 can run in parallel; T007 depends on T005+T006; T008 depends on T005
- Phase 3: T012, T013, T014, T015 can run in parallel (all Makefile edits, different rule additions)
- Phase 4: T020, T021 can run in parallel
- Phase N: T024, T025, T026, T027, T028 can ALL run in parallel

---

## Parallel Execution Examples

### Phase 1: Setup (All Parallel)

```bash
# Launch all Setup tasks together
Task: "Create test/ directory structure"  (T001)
Task: "Download Unity v2.6.1"  (T002)
Task: "Download CMock v2.6.0"  (T003)
Task: "Update .gitignore"  (T004)
```

### Phase 3: User Story 1 (Mostly Parallel)

```bash
# Launch all Makefile tasks in parallel (T012-T015)
Task: "Add test config variables to Makefile"  (T012)
Task: "Add Unity compilation rule"  (T013)
Task: "Add test target"  (T014)
Task: "Add test directory rule"  (T015)

# Then after Makefile is complete:
Task: "Verify make test runs"  (T017)
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Download Unity/CMock ✅
2. Complete Phase 2: Extract `calc.c/calc.h` ✅
3. Complete Phase 3: Unity sample test + `make test` ✅
4. **STOP AND VALIDATE**: `make test` shows green, `make build` zero warnings
5. Demo: Show learner the Unity test vs custom framework comparison

### Incremental Delivery

1. MVP: Phase 1 + 2 + Phase 3 (Unity test case) = **19 tasks (T001-T019)**
2. Add CMock education: Phase 4 = **4 tasks (T020-T023)**
3. Polish: Phase N = **9 tasks (T024-T032)**
4. Each phase adds value without breaking previous phases

### Parallel Team Strategy

With multiple developers:
1. Team completes Phase 1 + Phase 2 together
2. Once Phase 2 is done:
   - Developer A: Phase 3 (US1 — Unity test)
   - Developer B: Phase 4 (US2 — CMock education)
3. Both stories complete independently, then Phase N (Polish) together

---

## Task Summary

| Phase | Tasks | Status | Description |
| ----- | ----- | ------ | ----------- |
| Phase 1: Setup | T001-T004 | ✅ COMPLETE | Download Unity/CMock, create directories |
| Phase 2: Foundational | T005-T010 | ✅ COMPLETE | Extract `calc.c/calc.h` for testability |
| Phase 3: US1 (P1) MVP | T011-T019 | ✅ COMPLETE | Unity sample test + `make test` |
| Phase 4: US2 (P2) | T020-T023 | Pending | CMock education in tutorial |
| Phase N: Polish | T024-T032 | Pending | Documentation updates |

**Total Tasks**: 32
**Completed**: 19 (T001-T019)
**MVP Scope**: ✅ Phase 1 + 2 + 3 = 19 tasks COMPLETE

**MVP Scope**: Phase 1 + Phase 2 + Phase 3 = **19 tasks** (T001-T019)

**Full Scope**: All 32 tasks = Complete Unity + CMock integration with documentation

---

## Notes

- [P] tasks = different files, no dependencies on incomplete tasks — can run in parallel
- [US1] / [US2] labels map tasks to user stories for traceability
- Each user story is independently completable and testable
- Verify tests fail before implementing (T011 → T012+)
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same-file conflicts, cross-story dependencies that break independence
