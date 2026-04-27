# Implementation Plan: C Basic Tutorial Chapters

**Branch**: `001-c-basic-tutorial` | **Date**: 2026-04-26 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/docs/specs/001-c-basic-tutorial/spec.md`

## Summary

Build a comprehensive C language basic tutorial (~27 chapters) with mdBook documentation and runnable source code, mirroring hello-rust's chapter granularity and documentation structure. Each chapter has a first-person narrative voice (个人学习经验总结), code examples, knowledge checkpoints, and bilingual Chinese/English content. Source follows hello-rust's `_sample` convention — each chapter has `<topic>.c` + `<topic>_sample.c`, all compiled into the single `hello` binary via Makefile glob.

## Technical Context

**Language/Version**: C17 (ISO/IEC 9899:2018) | gcc 12+ or clang 15+
**Primary Dependencies**: POSIX C standard library, `<stdint.h>`, `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`, `<math.h>`, `<limits.h>`, `<time.h>`, `<unistd.h>`, `<errno.h>`, `<stdarg.h>`
**Storage**: N/A — tutorial is stateless code examples
**Testing**: `valgrind --leak-check=full`, `cppcheck --enable=all`, compiler `-Wall -Wextra -Werror`; no unit test framework yet
**Target Platform**: macOS (clang), Linux (gcc), Solaris/Illumos, FreeBSD
**Project Type**: CLI/tutorial-library + mdBook documentation → GitHub Pages
**Performance Goals**: N/A for tutorial code — examples must be correct, not fast. No memory leaks
**Constraints**: Zero compiler warnings (`-Wall -Wextra -Werror`), zero valgrind errors, bounded string ops only, include guards mandatory
**Scale/Scope**: 27 chapters + overview + glossary + review = 30 mdBook files; ~81 source files (27 topics × 3 files: `<topic>.c`, `<topic>.h`, `<topic>_sample.c`) + coordinator files

### Unknowns Resolved

All unknowns resolved via Phase 0 research (see `research.md`):

| Unknown | Resolution | Reference |
|---------|-----------|-----------|
| Chapter mapping from hello-rust to C | 27 chapters mapped; see research.md R-001 | R-001 |
| Multiple `main()` conflicts in C binary | `_sample.c` files expose `main_<topic>()` functions (NO `main()`), compiled into single `hello` binary via glob — same as hello-rust's module pattern | R-002 |
| mdBook chapter template structure | 15 mandatory sections per chapter; see research.md R-003 | R-003 |
| Coordinator orchestration | `basic.c` calls `main_<topic>()`; each chapter has `.c` + `.h` pair | R-004 |
| Existing file disposition | Delete `basic.c/h`, `datatype_sample.*`; clean rebuild | R-005 |
| book.toml language setting | Change `"en"` → `"zh"` | R-006 |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify all 5 Hello-C Constitution principles:

- [x] **I. Memory Safety**: All dynamic memory examples use `malloc`/`free` with NULL checks; bounded string ops mandated; no unbounded allocations planned
- [x] **II. Code Quality**: `-Wall -Wextra -Werror` in Makefile; include guards enforced; Doxygen comments required for public APIs
- [x] **III. Modular Architecture**: Each chapter is a single-responsibility module; `main.c` remains thin; no circular dependencies
- [x] **IV. Multi-Platform Portability**: Platform-specific code isolated behind `#ifdef`; `<stdint.h>` types used; conditional compilation is itself a chapter topic
- [x] **V. SDD Workflow**: Spec ✓, plan ✓, tasks TBD, manual commit acknowledged

## Project Structure

### Documentation (this feature)

```text
docs/specs/001-c-basic-tutorial/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (skipped — internal tutorial, no external APIs)
└── tasks.md             # Phase 2 output (/speckit.tasks command)
```

### Source Code (repository root)

```text
docs/
├── book.toml                              # mdBook config (language: "zh")
└── src/
    ├── SUMMARY.md                         # Sidebar navigation
    ├── chapter_1.md                       # DELETE — replaced by basic/
    └── basic/
        ├── basic-overview.md              # Chapter overview + learning path diagram
        ├── variables.md                   # 变量与表达式 (🟢)
        ├── datatype.md                    # 数据类型 (🟢)
        ├── functions.md                   # 函数 (🟢)
        ├── operators.md                   # 运算符与表达式 (🟢)
        ├── arrays.md                      # 数组基础 (🟢)
        ├── control-flow.md                # 控制流: if/else/switch (🟢)
        ├── loops.md                       # 循环: for/while/do-while (🟢)
        ├── preprocessor.md                # 预处理器与宏 (🟢)
        ├── pointers.md                    # 指针基础 (🟡)
        ├── pointer-arith.md               # 指针运算与数组 (🟡)
        ├── strings.md                     # 字符串深度 (🟡)
        ├── structs.md                     # 结构体 (🟡)
        ├── struct-fields.md               # 结构体字段与内存布局 (🟡)
        ├── enums.md                       # 枚举与联合体 (🟡)
        ├── function-pointers.md           # 函数指针与"方法" (🔴)
        ├── scope.md                       # 作用域与生命周期 (🟡)
        ├── memory-mgmt.md                 # 内存管理: malloc/free (🔴)
        ├── file-io.md                     # 文件 I/O (🟡)
        ├── headers.md                     # 头文件与模块系统 (🟡)
        ├── callbacks.md                   # 回调函数与多态 (🔴)
        ├── logging.md                     # 日志与格式化输出 (🟢)
        ├── debugging.md                   # 调试与错误处理 (🟡)
        ├── conditional-comp.md            # 条件编译 (🟢)
        ├── void-generic.md                # void* 泛型编程 (🔴)
        ├── bit-ops.md                     # 位运算与内存操作 (🟡)
        ├── cli-args.md                    # 命令行参数与 I/O 重定向 (🟡)
        ├── stdlib.md                      # 标准库精要 (🟡)
        ├── basic-glossary.md              # C 术语表
        └── review-basic.md                # 阶段复习: comprehensive knowledge check
src/
├── main.c                                 # Entry point (orchestration only) — UPDATED
├── hello.c/.h                             # Feature module — calls main_basic()
├── sysinfo.c/.h                           # Platform detection (existing)
├── basic/
│   ├── basic.c                            # Coordinator: calls all main_<topic>()
│   ├── basic.h                            # Declares all main_<topic>() functions
│   ├── variables.c                        # Integrated module (main_variables)
│   ├── variables.h
│   ├── variables_sample.c             # Sample: hello-rust convention, main_variables()
│   ├── datatype.c                         # Integrated module
│   ├── datatype.h
│   ├── datatype_sample.c              # Sample: hello-rust convention, main_datatype()
│   ...                                    # ~25 more chapter pairs (<topic>.c + <topic>.h + <topic>_sample.c per chapter)
├── advance/                               # Existing stub (untouched)
├── algo/                                  # Stub (untouched)
├── module1/                               # Boilerplate example (untouched)
├── module2/                               # Boilerplate example (untouched)
└── h.c                                    # Orphan file (candidate for future cleanup)
include/
└── global.h                               # Project-wide declarations (untouched)
```

**Structure Decision**: C single-project layout matching hello-rust's convention. Each chapter delivers `<topic>.c` + `<topic>.h` (coordinator module) + `<topic>_sample.c` (example code). All compiled into single `hello` binary via Makefile glob — no filter-out needed. Existing `datatype_sample.*` kept and refactored into the new chapter structure.

## Complexity Tracking

> No violations — single project structure, well within the 10-module limit (basic/ is one module with sub-files).
