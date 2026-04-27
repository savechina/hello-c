# Feature Specification: C Basic Tutorial Chapters

**Feature Branch**: `001-c-basic-tutorial`  
**Created**: 2026-04-26  
**Status**: Draft  
**Input**: User description: 参考 ../hello-rust/docs/src 教程，创建 hello C 语言 basic 教程  hello-c 项目

## Clarifications

### Session 2026-04-26

- Q: Chapter granularity — how many basic chapters, and how deeply should topics be split? → A: Match hello-rust granularity (~20+ chapters) — split complex topics into sub-chapters (e.g., pointers → pointer basics, pointer arithmetic, function pointers, void pointers)
- Q: Writing voice & narrative persona for the tutorial → A: 个人学习经验总结 (personal learning diary) — first-person narrative ("我发现", "我的理解"), casual reflective tone with "aha moment" opening stories, matching hello-rust's story/analogy approach
- Q: Code example format — standalone files vs integrated module → A: Follow hello-rust pattern — each `_sample.c` is compiled into the single `hello` binary via Makefile glob (no separate `main()`). `make sample CHAPTER=<name>` runs the specific topic. All samples integrate like hello-rust's `mod.rs` pattern.
- Q: Should basic section include review chapter and glossary like hello-rust → A: Include both — `review-basic.md` (阶段复习: 10+ Qs covering all 20+ topics) AND `basic-glossary.md` (术语表: C-specific terms with Chinese/English definitions)
- Q: Handling existing `src/basic/` files — preserve, refactor, or replace? → A: Replace entirely — delete basic.c/basic.h/datatype_sample.* and rebuild from scratch for structural coherence with 20+ chapters
- Q: Source file naming convention → A: All chapters use `<topic>_sample.h` + `<topic>_sample.c` naming convention. Function names use `main_<topic>_sample()` for coordinator entry points, internal samples use `<topic>_<subsample>_sample()`. No separate `.c`/`.h` without `_sample` suffix. `basic.h` declares all `main_*_sample()` functions. `basic.c` is the coordinator calling each `main_<topic>_sample()`.
- Q: Tutorial differentiation — what makes this C tutorial "fast to learn" and "cognitive-science friendly"? → A: Error-first learning (Option C) — show common mistakes first with compiler errors, then the fix. Combined with code-first approach (code on page 1, theory after). Matches hello-rust's common error pattern but more aggressive: every concept demonstrates the wrong way → compiler output → right way. Leverages "productive failure" cognitive science principle.
- Q: How should the tutorial handle C's most cognitively difficult concepts (pointers, memory management, undefined behavior)? → A: Option D — all three techniques: ASCII memory diagrams for spatial visualization, interactive exercises (run→modify→break levels), comparison tables with Python/JS/other languages to connect new mental models to existing ones. Applied to pointer, memory-mgmt, string-deep, and other difficult chapters.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Core C Fundamentals mdBook Tutorial (Priority: P1)

A C language beginner navigates the hello-c mdBook site (GitHub Pages), reads through the basic tutorial chapters (mirroring the hello-rust docs structure), and follows each code example by building and running the corresponding `src/basic/` source files.

**Why this priority**: This is the core deliverable — the tutorial content itself. Without it, the project is just a code skeleton with no instructional value.

**Independent Test**: A user can open the mdBook site, read a basic chapter (e.g., "数据类型"), find the matching source file in `src/basic/`, compile it with `make build`, and see the documented output — all within one chapter.

**Acceptance Scenarios**:

1. **Given** the user opens the mdBook site, **When** they navigate to the Basic section, **Then** they see a chapter overview page with a table of all basic chapters (matching hello-rust's basic-overview.md structure)
2. **Given** a user reads a chapter, **When** they copy the code example into their editor and run `make build && make run`, **Then** the code compiles with zero warnings and produces the documented output
3. **Given** the mdBook `SUMMARY.md` links all chapters, **When** the user clicks a chapter title in the sidebar, **Then** they are taken directly to that chapter's content

---

### User Story 2 - Runnable Code Examples (Priority: P1)

Every mdBook chapter has a corresponding runnable source file (`.c` + `.h`) in `src/basic/` that demonstrates the concept being taught, following the `main_<topic>()` pattern established by existing modules.

**Why this priority**: Code is the essence of a programming tutorial. Without runnable examples, the book is theory-only and fails the project's pedagogical mission.

**Independent Test**: Each `src/basic/*.c` file defines `main_<topic>()`, the Makefile glob picks it up, and calling it from `main_basic()` produces correct output.

**Acceptance Scenarios**:

1. **Given** the build system scans `src/**/*.c`, **When** new basic chapter files are added, **Then** `make build` automatically compiles them without build file changes
2. **Given** the memory management chapter's code, **When** run under valgrind, **Then** zero leaks and zero errors are reported

---

### User Story 3 - Bilingual Documentation with Learning Aids (Priority: P2)

Each chapter follows the hello-rust documentation template: opening story/analogy, learning objectives, concept explanation with code examples, practice exercises, knowledge checkpoints (with collapsible answers), and further reading suggestions — all in bilingual Chinese + English.

**Why this priority**: The instructional quality and format consistency make this a professional tutorial rather than a collection of code files.

**Independent Test**: Reviewing any three random chapters confirms they all follow the same structural template (opening story → objectives → concepts → examples → checkpoint → next steps).

**Acceptance Scenarios**:

1. **Given** a chapter's "知识检查" section, **When** the user clicks the collapsible answer, **Then** they see the correct answer with explanation
2. **Given** the chapter's difficulty rating, **When** a beginner reads it, **Then** the content depth matches the stated difficulty

---

### User Story 4 - Multi-Platform Portability Examples (Priority: P3)

The tutorial demonstrates how to write portable C code using `#if defined(...)` guards, referencing the project's existing `sysinfo.c` as a real-world example of cross-platform C programming.

**Why this priority**: Differentiates hello-c from other tutorials. Portability is a core constitutional requirement of the project.

**Independent Test**: Code containing platform-specific conditionals compiles and runs correctly on macOS (clang) and Linux (gcc) without modification.

**Acceptance Scenarios**:

1. **Given** a chapter on `#ifdef` and platform selection, **When** built on macOS, **Then** the platform detection output shows "macOS"
2. **Given** the same code built on Linux, **Then** the output shows "Linux" and no compilation warnings appear

---

### Edge Cases

- Older compiler versions: Tutorial assumes C17 with GCC 12+ / Clang 15+. Earlier versions may fail on `_Generic` or other C11/C17 features — the book notes these requirements upfront
- Known `fibonacci()` naming bug in `src/hello.c`: New tutorial chapters will not replicate this bug; function names accurately describe their behavior
- Skipping chapters: Each `main_<topic>()` is independently callable from `main_basic()`, allowing non-linear study
- Platform-specific headers: All `#include` for platform headers (e.g., `<mach/mach.h>`, `<sys/utsname.h>`) stay within appropriate `#ifdef` blocks
- **Cognitive load**: Error-first learning means each concept shows the WRONG approach first — the tutorial must immediately follow with WHY it's wrong (compiler output/memory diagram) and the CORRECT fix, to avoid learners memorizing mistakes

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The Basic section MUST have an overview page (`docs/src/basic/basic-overview.md`) matching the hello-rust structure: chapter table with difficulty ratings, learning objectives, and a visual learning path diagram
- **FR-002**: Each tutorial chapter MUST be an mdBook markdown file in `docs/src/basic/` following the hello-rust chapter template with error-first learning approach:
  - Opening story/analogy (开篇故事)
  - Target audience (本章适合谁)
  - Learning objectives (你会学到什么)
  - Prerequisites (前置要求)
  - First code example — runnable code on page 1, explanation after (第一个例子)
  - Error-first concept explanation — common mistake → compiler error → correct fix → theory (常见错误 integrated into 原理解析)
  - Knowledge checkpoint with collapsible answers (知识检查)
  - Further reading suggestions (延伸阅读)
  - Next chapter link (下一步)
- **FR-003**: Each chapter's code example MUST have a corresponding runnable `.c` and `.h` file pair in `src/basic/` following the `main_<topic>()` entry function pattern. Samples use `_sample.c` suffix (e.g., `datatype_sample.c`) following hello-rust's `_sample.rs` convention
- **FR-013**: Each chapter MUST ALSO have a `_sample.c` file (e.g., `src/basic/variables_sample.c`) following the hello-rust convention. Sample files are compiled into the single `hello` binary alongside the main module. No separate `main()` — samples share the coordinator pattern with `main_<topic>()` functions
- **FR-004**: All source files MUST compile with zero warnings under `-Wall -Wextra -Werror -std=c17`
- **FR-005**: All header files MUST use include guards (`#ifndef TOPIC_H / #define TOPIC_H / #endif`)
- **FR-006**: The tutorial MUST cover at least 20 chapters mirroring hello-rust's basic section granularity. Complex topics MUST be split into sub-chapters (e.g., pointers → pointer basics, pointer arithmetic, function pointers, void pointers) covering: variables & data types, operators & expressions, control flow (if/switch/loops), functions & scope, arrays, pointers, strings, structs, enums, typedef, unions, dynamic memory management, file I/O, preprocessor directives, multi-platform `#ifdef` patterns, and the C standard library overview
- **FR-007**: Each chapter's code examples MUST be linked from the mdBook page to the `src/basic/` source file (via GitHub code link)
- **FR-008**: `docs/src/SUMMARY.md` MUST be updated to include all new basic chapters in the sidebar navigation hierarchy
- **FR-008**: Difficult concepts (pointers, memory management, undefined behavior) MUST include three cognitive aids: (1) ASCII memory layout diagrams showing stack/heap/pointer relationships, (2) three-level interactive exercises — "run this" → "change this" → "break this", (3) comparison tables with Python/JavaScript to connect C concepts to familiar mental models
- **FR-009**: All dynamic memory examples MUST run cleanly under `valgrind --leak-check=full` with zero leaks and zero errors
- **FR-010**: Memory allocation examples MUST follow constitution requirements: malloc/free pairing, NULL checks, bounded string operations (`strncpy` over `strcpy`, `snprintf` over `sprintf`)
- **FR-011**: Documentation MUST use first-person narrative voice ("我发现", "我的理解") as personal C learning experience summary — casual, reflective tone with "aha moment" opening stories. Each chapter MUST follow the hello-rust story/analogy template (e.g., "工具箱" metaphor). English technical terms appear in parentheses after Chinese
- **FR-012**: The mdBook language setting in `book.toml` MUST be changed from "en" to "zh" to reflect the Chinese-language content
- **FR-013**: Each chapter MUST ALSO have a `_sample.c` file (e.g., `src/basic/variables_sample.c`) following the hello-rust convention. Sample files are compiled into the single `hello` binary alongside the main module. No separate `main()` — samples share the coordinator pattern with `main_<topic>()` functions
- **FR-014**: The basic section MUST include `review-basic.md` (阶段复习) — a cumulative chapter with 10+ questions covering all 20+ topics, with multiple difficulty levels (🟢🟡🔴) and collapsible answers
- **FR-015**: The basic section MUST include `basic-glossary.md` (术语表) — a C-specific terminology reference with Chinese/English bilingual definitions, cross-referenced to relevant chapters

### Key Entities

- **mdBook Chapter**: A markdown file in `docs/src/basic/` that teaches one C concept with narrative explanation, code examples, knowledge checkpoints, and navigation links
- **Source Module**: A `_sample.c` + `_sample.h` file pair in `src/basic/` that provides a runnable demonstration of the chapter's concept, exposing `main_<topic>_sample()`
- **Chapter Overview**: The `basic-overview.md` entry page with chapter table, learning path diagram, difficulty ratings, and estimated study times
- **Knowledge Checkpoint**: A self-assessment section at each chapter's end with collapsible `<details>` answers in mdBook

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All basic tutorial `_sample.c` files compile with zero warnings (`make build` exits cleanly)
- **SC-002**: The mdBook documentation covers at least 20 distinct C topics with full chapter structure, matching hello-rust basic section granularity
- **SC-003**: A complete beginner can follow any chapter, copy its code example, and have it compile and run within 5 minutes
- **SC-004**: All memory management code passes valgrind with zero leaks and zero errors
- **SC-005**: The mdBook sidebar (`SUMMARY.md`) displays all chapters in a logical learning hierarchy with nested subchapters where appropriate

## Assumptions

- The hello-rust `docs/src/basic/` structure is used as a template for chapter organization — not as direct content translation. Rust-specific topics (ownership, lifetimes, traits, async) are replaced with C equivalents (malloc/free, function pointers, `#ifdef`, `const` correctness, `sizeof`, pointer arithmetic)
- The existing `src/basic/basic.c`, `src/basic/basic.h`, `src/basic/datatype_sample.c`, and `src/basic/datatype_sample.h` files will be deleted and replaced with the new 20+ chapter file set. Existing content may be copied into new files if pedagogically useful
- Target audience: C beginners with basic programming knowledge (understand variables, loops, functions) — the tutorial does not teach programming fundamentals from ground zero
- The existing Makefile glob (`src/**/*.c`) automatically picks up new files in `src/basic/` without build modifications
- mdBook is already configured (`docs/book.toml`) — only new chapter files and `SUMMARY.md` updates are needed
- GitHub Pages deployment is already configured via existing GitHub Actions workflow
- No test framework currently exists; correctness is validated via compilation with `-Werror` and manual valgrind execution
