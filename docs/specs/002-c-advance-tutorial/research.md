# Research & Decisions: C Advance Tutorial

**Feature**: 002-c-advance-tutorial
**Date**: 2026-04-27

## R-001: hello-rust Advance Mapping to C

Decision: Map each Rust advance topic to C equivalent paradigm.

| hello-rust | C Equivalent | Rationale |
|------------|-------------|-----------|
| smart-pointers | Opaque pointers + RAII macros | C has no RAII; use macros for cleanup patterns, opaque pointers for encapsulation |
| atomic-types | C11 `<stdatomic.h>` | Direct equivalent; volatile for non-atomic hardware registers |
| async | POSIX threads + select/poll | C has no async/await; pthread for concurrency, poll/select for I/O multiplexing |
| iterators | Linked lists, dynamic arrays, trees | C iterators are manual pointer-based traversal; data structures chapter covers this |
| advanced-traits | Function pointer tables, vtable pattern | C has no traits/mirrors Rust trait dispatch via struct of function pointers |
| error-handling | errno, setjmp/longjmp, error callbacks | C error patterns (vs Rust's Result/panic) |
| system | POSIX syscalls: signals, mmap, process mgmt | System programming in C |
| testing | Custom ASSERT macros, test harness | C has no built-in testing; write mini framework |
| database | SQLite3 C API | C-compatible database (no ORM in C) |
| web | HTTP server with sockets | Bare-bones socket programming |
| tools | Build patterns, coverage, static analysis | Developer tools for C projects |
| review-advance | Comprehensive review | Mirror hello-rust review-advance.md |

Alternatives considered:
- Use C++ features (smart pointers, std::thread) → rejected, this is C17 not C++
- Use external libraries (libuv, Boost) → rejected, use POSIX/C standard only

## R-002: Unity Test Framework Selection

**Date**: 2026-05-02

Decision: Use Unity v2.6.1 (3 source files: unity.c, unity.h, unity_internals.h)

**Rationale**:
- Header-only style (3 files, no linking required)
- Actively maintained by ThrowTheSwitch organization
- Excellent beginner documentation
- Supports C17 standard, works on macOS/Linux/Solaris/FreeBSD
- Rich assertion macros

**Download URLs** (v2.6.1):
- Release: https://github.com/ThrowTheSwitch/Unity/releases/tag/v2.6.1
- `unity.c`: https://raw.githubusercontent.com/ThrowTheSwitch/Unity/v2.6.1/src/unity.c
- `unity.h`: https://raw.githubusercontent.com/ThrowTheSwitch/Unity/v2.6.1/src/unity.h
- `unity_internals.h`: https://raw.githubusercontent.com/ThrowTheSwitch/Unity/v2.6.1/src/unity_internals.h

**Alternatives considered**:

| Alternative | Rejected Because |
| ----------- | ----------------- |
| Check       | Requires linking libcheck, more complex API |
| CMocka      | Requires linking libcmocka, mocking needs `--wrap` flag |
| minunit     | Too minimal — limited assertions |
| Criterion   | Not header-only, requires linking |

## R-003: CMock Mocking Framework

**Date**: 2026-05-02

Decision: Add CMock v2.6.0 source to `test/vendor/cmock/` for educational completeness

**Rationale**:
- Companion tool to Unity (same organization)
- Generates mock C files from headers using Ruby
- Perfect for demonstrating mocking in later tutorial chapter
- Keep separate from Unity sample — no mocks needed for `calc_add()`

**Download URL** (v2.6.0):
- Release: https://github.com/ThrowTheSwitch/CMock/releases/tag/v2.6.0
- Required: `lib/` directory (all `.rb` files)

**Dependencies**: Ruby 3.0+ (for mock generation only)

## R-004: Directory Structure for Test Files

**Date**: 2026-05-02

Decision: Use `test/vendor/unity/` and `test/vendor/cmock/`

**Rationale**:
- `test/` separates testing infra from `src/` production code
- `vendor/` clearly indicates external dependencies
- Conventional naming in C projects
- Matches Ceedling's default structure

**Structure**:
```
hello-c/
├── src/advance/testing_sample.c   # Keep custom framework (educational)
├── test/
│   ├── vendor/
│   │   ├── unity/              # Unity v2.6.1 (3 files)
│   │   └── cmock/              # CMock v2.6.0 (Ruby files)
│   ├── mocks/                  # Generated mocks (gitignored)
│   └── test_calc_add.c          # Sample Unity test
└── Makefile                     # Add `make test` target
```

## R-005: Makefile `test` Target

**Date**: 2026-05-02

Decision: Add `make test` target with out-of-tree build in `build/test/`

**Rationale**:
- Follows existing pattern: `build/obj/`, `build/bin/`
- Unity compiled once to `build/unity.o`, linked to each test
- Loop through test binaries, capture exit codes
- Return non-zero if any test fails (CI integration)

**Key Makefile Patterns**:
```makefile
TEST_DIR    := test
UNITY_DIR   := $(TEST_DIR)/vendor/unity
UNITY_OBJ   := $(BUILD_DIR)/unity.o
TEST_BINS   := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/test/%, $(wildcard $(TEST_DIR)/*.c))

$(UNITY_OBJ): $(UNITY_DIR)/unity.c | $(BUILD_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(BUILD_DIR)/test/%: $(TEST_DIR)/%.c $(UNITY_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -c $< -o $@.o
	$(CC) $(TEST_CFLAGS) $@.o $(UNITY_OBJ) -o $@

.PHONY: test
test: $(TEST_BINS)
	@failed=0; \
	for t in $(TEST_BINS); do \
		./$$t || { failed=1; }; \
	done; \
	[ $$failed -eq 0 ]
```

## R-006: Sample Test Case Design

**Date**: 2026-05-02

Decision: Test `calc_add()` from `testing_sample.c` using Unity

**Rationale**:
- `calc_add()` is simple, self-contained, no dependencies
- Allows direct comparison: custom `ASSERT_EQ_RUN()` vs Unity `TEST_ASSERT_EQUAL_INT()`
- Perfect for tutorial: "Here's how we tested before, here's Unity"

**Note on `static` functions**: `calc_add()` is `static` in `testing_sample.c`. Options:
1. Move testable functions to separate `calc.c/calc.h` (recommended)
2. Make `calc_add` non-static and declare in header
3. Use `#include "testing_sample.c"` in test (not recommended)

**Recommended**: Create `src/advance/calc.c` and `src/advance/calc.h` with `calc_add()`, `calc_multiply()`, `calc_is_valid()` for testability.

## R-007: Unity Compiler Flags

**Date**: 2026-05-02

**Recommended flags**:
| Flag | Purpose |
| ---- | ------- |
| `-DUNITY_OUTPUT_COLOR` | Colored pass/fail output |
| `-DUNITY_SUPPORT_VARIADIC_MACROS` | Enable message variants |
| `-DUNITY_INCLUDE_CONFIG_H` | Optional: use custom `unity_config.h` |

**TEST_CFLAGS**:
```makefile
TEST_CFLAGS := -std=c17 -Wall -Wextra -g -O2 -MMD \
               -I$(UNITY_DIR) \
               -DUNITY_OUTPUT_COLOR \
               -DUNITY_SUPPORT_VARIADIC_MACROS
```
