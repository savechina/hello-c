# Research & Decisions: C Basic Tutorial

**Feature**: 001-c-basic-tutorial
**Date**: 2026-04-26

---

## R-001: Chapter Mapping (hello-rust → hello-c)

**Decision**: 27 chapters + overview + glossary + review = 30 total files in `docs/src/basic/`.

**Rationale**: hello-rust has 19 basic chapters. 19 map directly or with C-semantic translation. 8 new C-exclusive chapters cover concepts irrelevant to Rust (preprocessor macros, null-terminated strings, system calls, build systems).

**Chapter List (in learning order)**:

| # | C Chapter | hello-rust equivalent | Difficulty |
|---|-----------|----------------------|------------|
| 1 | variables.md — 变量与表达式 | variables/expression | 🟢 |
| 2 | datatype.md — 数据类型 | datatype | 🟢 |
| 3 | functions.md — 函数 | functions | 🟢 |
| 4 | operators.md — 运算符与表达式 | expression (sub) | 🟢 |
| 5 | arrays.md — 数组基础 | datatype (sub) | 🟢 |
| 6 | control-flow.md — 控制流: if/else/switch | (implicit in expression) | 🟢 |
| 7 | loops.md — 循环: for/while/do-while | (implicit in expression) | 🟢 |
| 8 | preprocessor.md — 预处理器与宏 | cfg_if (expanded) | 🟢 |
| 9 | pointers.md — 指针基础 | pointer | 🟡 |
| 10 | pointer-arith.md — 指针运算与数组 | pointer (sub) | 🟡 |
| 11 | strings.md — 字符串深度 | (C-exclusive) | 🟡 |
| 12 | structs.md — 结构体 | struct | 🟡 |
| 13 | struct-fields.md — 结构体字段与内存布局 | struct-fields | 🟡 |
| 14 | enums.md — 枚举与联合体 | enums | 🟡 |
| 15 | function-pointers.md — 函数指针与"方法" | struct-methods | 🔴 |
| 16 | scope.md — 作用域与生命周期 | lifetimes | 🟡 |
| 17 | memory-mgmt.md — 内存管理: malloc/free | ownership | 🔴 |
| 18 | file-io.md — 文件 I/O | (C-exclusive) | 🟡 |
| 19 | headers.md — 头文件与模块系统 | module | 🟡 |
| 20 | callbacks.md — 回调函数与多态 | traits | 🔴 |
| 21 | logging.md — 日志与格式化输出 | logger | 🟢 |
| 22 | debugging.md — 调试与错误处理 | tracing | 🟡 |
| 23 | conditional-comp.md — 条件编译 | cfg_if | 🟢 |
| 24 | void-generic.md — void* 泛型编程 | generic | 🔴 |
| 25 | bit-ops.md — 位运算与内存操作 | (C-exclusive) | 🟡 |
| 26 | cli-args.md — 命令行参数与 I/O 重定向 | (C-exclusive) | 🟡 |
| 27 | stdlib.md — 标准库精要 | (C-exclusive) | 🟡 |
| — | basic-glossary.md — C 术语表 | glossary | — |
| — | review-basic.md — 阶段复习 | review-basic | — |

**Alternatives considered**:
- Consolidate to ~15 broader chapters → rejected per clarification (Q1: match hello-rust granularity)
- Start with 12 core chapters and expand later → rejected, user wants full set

---

## R-002: Build System — Sample Files (hello-rust Pattern)

**Decision**: `_sample.c` files ARE included in the main SOURCES glob — no filter-out needed. All samples compile into the single `hello` binary. Each sample exposes a function (e.g., `main_<topic>()`), NOT `main()`, avoiding link errors.

**Rationale**: hello-rust compiles ALL `_sample.rs` files into the single `hello` binary via `mod.rs`. Each sample function is called via CLI dispatch (`hello basic <topic>`). Hello-c mirrors this: `_sample.c` files define functions, `basic.c` coordinator calls them via `main_basic()`. Makefile glob (`src/**/*.c`) picks everything up automatically.

**Makefile**: No changes needed for sample exclusion. `make sample CHAPTER=<name>` is a convenience target that runs `hello` (the single binary) — future dispatcher can support `hello basic <topic>` per-topic selection.

**Alternatives considered**:
- Filter-out + separate compilation → rejected, doesn't match hello-rust's unified binary pattern
- Separate `main()` per sample → rejected, would cause link errors and break the `hello` binary

---

## R-003: mdBook Chapter Template Structure

**Decision**: Each chapter follows the hello-rust template with these mandatory sections:

1. `# [章节名]`
2. `## 开篇故事` — opening analogy (first-person voice per Q2)
3. `## 本章适合谁`
4. `## 你会学到什么`
5. `## 前置要求`
6. `## 第一个例子` — minimal runnable code + link to full source
7. `## 原理解析` — deep dive with `###` subsections
8. `## 常见错误` — ❌ code → compiler error → ✅ fix
9. `## 动手练习` — 3 exercises 🟢🟡🔴 with `<details>` answers
10. `## 故障排查 (FAQ)`
11. `## 知识扩展 (选学)`
12. `## 小结` — key points + terminology
13. `## 术语表` — English/中文 table
14. `## 延伸阅读`
15. `## 继续学习` — prev/next links

Optional sections: `## 知识检查`, `## 内存布局可视化`, `💡 小知识`, `🌟 工业界应用`, `🧪 动手试试`

**Code format**: Fenced ` ```c ` blocks for in-chapter examples; ` ```c,ignore ` for error demos; GitHub link after "第一个例子" to full source.

---

## R-004: Coordinator Orchestration

**Decision**: `src/basic/basic.c` becomes the coordinator — declares and calls `main_<topic>()` for each integrated chapter file. `main_basic()` is called from `src/hello.c` → `src/main.c`. Each chapter's `.c` exposes `main_<topic>()`; the header `basic.h` declares all functions (or each module has its own `.h`).

**Rationale**: Maintains the existing pattern. `main_basic()` runs all basic lessons sequentially. `make run` executes the full tutorial.

**File structure per chapter**:
- `src/basic/<topic>.c` — defines `main_<topic>()`, called from `basic.c`
- `src/basic/<topic>.h` — declares `main_<topic>()` with Doxygen comment
- `src/basic/<topic>_sample.c` — sample code following hello-rust's `_sample.rs` convention, defines `main_<topic>()`, compiled into `hello` binary via Makefile glob

---

## R-005: Existing File Disposition

**Decision**: Delete `src/basic/basic.c`, `src/basic/basic.h`, `src/basic/datatype_sample.c`, `src/basic/datatype_sample.h`. Replace with new chapter structure.

**Rationale**: Confirmed by clarification Q5. Clean slate avoids conflicts. Useful code snippets can be copied into new files.

---

## R-006: book.toml Language

**Decision**: Change `book.toml` `language = "en"` to `language = "zh"` for Chinese-language content.

**Rationale**: mdBook uses this for HTML lang attribute and search index language. FR-012 requirement.
