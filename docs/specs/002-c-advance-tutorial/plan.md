# Implementation Plan: Add Unity & CMock for Test Sample

**Branch**: `002-c-advance-tutorial` | **Date**: 2026-05-02 | **Spec**: `/docs/specs/002-c-advance-tutorial/spec.md`
**Input**: Feature specification from `/docs/specs/002-c-advance-tutorial/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

**Status**: Phase0 (Research) ✅ COMPLETE | Phase1 (Design) ✅ COMPLETE | Ready for `/speckit.tasks`

## Summary

Add Unity test framework and CMock to the hello-c tutorial project. Keep the existing custom testing framework in `src/advance/testing_sample.c` for educational purposes, and create a new `test/` directory with a Unity-based sample test case for `calc_add()` from `testing_sample.c`. Also add CMock source to the repo for future mocking examples. Update the Makefile with a `make test` target.

## Technical Context

**Language/Version**: C17 (ISO/IEC 9899:2018) | gcc 12+ or clang 15+
**Primary Dependencies**: POSIX C standard library, `<stdint.h>`, `<stdio.h>`, Unity (v2.6.1), CMock
**Storage**: N/A (test framework, no persistence)
**Testing**: Unity test framework (unit tests), CMock (mocking for future chapters), `valgrind`, `cppcheck`
**Target Platform**: macOS, Linux, Solaris/Illumos, FreeBSD
**Project Type**: CLI/tutorial
**Performance Goals**: Test execution <1s for sample test case
**Constraints**: Zero compiler warnings (`-Wall -Wextra -Werror`), no memory leaks in test code, header-only Unity integration (copy source files)
**Scale/Scope**: 3 Unity source files, 1 CMock directory, 1 sample test file, Makefile updates

## Constitution Check

*GATE: Must pass before Phase0 research. Re-check after Phase1 design.*

Verify all 5 Hello-C Constitution principles:

- [x] **I. Memory Safety**: Unity/CMock are well-established C frameworks with proper memory management. Test code will use bounded operations. No `malloc` without `free` in test cases.
- [x] **II. Code Quality**: C17 standard, `-Wall -Wextra -Werror` clean for all test code. Unity headers use proper include guards. Test files will have Doxygen comments.
- [x] **III. Modular Architecture**: `test/` directory separate from `src/`. Unity/CMock in `test/vendor/` or `third_party/`. No circular dependencies. `main.c` unchanged.
- [x] **IV. Multi-Platform Portability**: Unity supports macOS, Linux, Solaris, FreeBSD. Platform-specific code (if any) behind `#ifdef`. No hardcoded paths.
- [x] **V. SDD Workflow**: Spec created (`/speckit.specify`), clarifications done (`/speckit.clarify`), plan reviewed (this document), manual commit policy acknowledged.

**GATE STATUS: PASS** — All 5 principles satisfied. Proceeding to Phase0.

---

## Phase0: Research ✅ COMPLETE

**Artifacts generated**:
- `research.md` — Unity v2.6.1 (3 files), CMock v2.6.0 (Ruby), directory structure `test/vendor/`, Makefile `test` target patterns, sample test case design for `calc_add()`

**Key decisions**:
- Unity v2.6.1 (header-only, 3 files) — beginner-friendly, zero deps
- CMock v2.6.0 added to repo for educational completeness
- `test/vendor/unity/` and `test/vendor/cmock/` directory structure
- `make test` target with `build/test/` for binaries
- Sample test: `test/test_calc_add.c` testing `calc_add()`

---

## Phase1: Design ✅ COMPLETE

**Artifacts generated**:
- `data-model.md` — Added entities: TestFile, TestVendor, CalcModule
- `quickstart.md` — Added "Add Test with Unity" section
- Agent context updated via `.specify/scripts/bash/update-agent-context.sh opencode`

**Constitution Check (Post-Design)**:
- [x] **I. Memory Safety**: Test code uses Unity (established framework), no unbounded ops
- [x] **II. Code Quality**: All test code will follow C17, `-Wall -Wextra -Werror`
- [x] **III. Modular Architecture**: `test/` separate from `src/`, vendor files isolated
- [x] **IV. Multi-Platform**: Unity supports all target platforms
- [x] **V. SDD Workflow**: Spec ✅, Clarify ✅, Plan ✅, ready for `/speckit.tasks`

**GATE STATUS: PASS** — Post-design review complete. Ready for Phase2 (task decomposition via `/speckit.tasks`).

## Project Structure

### Documentation (this feature)

```text
docs/specs/002-c-advance-tutorial/
├── spec.md              # Feature specification (input)
├── plan.md              # This file (Phase0/1 output)
├── research.md          # Phase0 research findings
├── data-model.md        # Phase1 output (N/A for test framework addition)
└── tasks.md             # Phase2 output (/speckit.tasks command)
```

### Source Code (repository root)

```text
hello-c/
├── src/
│   ├── advance/
│   │   ├── testing_sample.c     # Existing custom framework (KEEP)
│   │   └── testing_sample.h
│   └── ... (other modules)
├── test/                        # NEW: Test directory
│   ├── vendor/                  # Unity + CMock source files
│   │   ├── unity/              # Unity (unity.c, unity.h, unity_internals.h)
│   │   └── cmock/              # CMock source
│   ├── test_calc_add.c          # Sample Unity test for calc_add()
│   └── Makefile.test           # (optional) Separate test Makefile
├── Makefile                     # MODIFY: Add `make test` target
└── include/                     # (optional) Shared test headers
```

**Structure Decision**: Use `test/vendor/` for Unity/CMock sources (self-contained, no external deps). Sample test in `test/test_calc_add.c`.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

No violations — all 5 principles pass cleanly.
