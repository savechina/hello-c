# Data Model: C Basic Tutorial Entities

**Feature**: 001-c-basic-tutorial
**Date**: 2026-04-26

## Entity: TutorialChapter

A self-contained teaching unit covering one C concept.

**Attributes**:
- `title` (string) — Chinese title, e.g., "指针基础"
- `title_en` (string) — English counterpart, e.g., "Pointer Basics"
- `filename` (string) — mdBook file, e.g., `docs/src/basic/pointers.md`
- `difficulty` (enum) — `🟢简单` | `🟡中等` | `🔴困难`
- `estimated_time` (string) — study estimate, e.g., "45 分钟"
- `prerequisites` (list[string]) — linked chapter filenames
- `sections` (list) — ordered list of mandatory section headings (见 research.md R-003)
- `voice` — first-person narrative ("我发现", "我的理解")
- `language` — bilingual: Chinese primary + English technical terms in parentheses

**Relationships**:
- Each `TutorialChapter` has 1..2 `SourceModule` files in `src/basic/`
- Each `TutorialChapter` links to 1 or more neighbor chapters (previous/next)
- All `TutorialChapter` entries are listed in `docs/src/basic/basic-overview.md`

## Entity: SourceModule

A runnable C source file that demonstrates a chapter's concept.

**Attributes**:
- `type` (enum) — `integrated` | `sample`
- `chapter_ref` (string) — linked TutorialChapter filename stem
- `c_file` (string) — source file, e.g., `src/basic/pointers.c`
- `h_file` (string) — header file, e.g., `src/basic/pointers.h`
- `sample_file` (string) — sample file, e.g., `src/basic/pointers_sample.c`
- `entry_function` (string) — `main_<topic>()` in both integrated and sample files (no `main()`, avoids link conflicts)
- `valgrind_clean` (boolean) — true if zero leaks/zero errors under valgrind
- `warnings` (integer) — must be 0 under `-Wall -Wextra -Werror`

**Relationships**:
- Each integrated `SourceModule` is called from `src/basic/basic.c`
- Each sample `SourceModule` is compiled via the same Makefile glob and run via `make sample CHAPTER=<name>`
- All `SourceModule` files share the `basic.h` header declarations (or have individual headers)

## Entity: ChapterOverview

The `basic-overview.md` landing page for the Basic section.

**Attributes**:
- `filename` — `docs/src/basic/basic-overview.md`
- `chapter_table` (list) — all TutorialChapters with difficulty, time, link
- `learning_path_diagram` (ascii) — visual dependency flow
- `learning_objectives` (list[string]) — 5-7 outcomes
- `learning_checkpoint` (list[string]) — self-assessment checklist

## Entity: Glossary

The `basic-glossary.md` reference of C terminology.

**Attributes**:
- `filename` — `docs/src/basic/basic-glossary.md`
- `entries` (list) — each entry has: term_en, term_cn, definition_cn, related_chapters

## Entity: ReviewChapter

The `review-basic.md` cumulative knowledge check.

**Attributes**:
- `filename` — `docs/src/basic/review-basic.md`
- `questions` (list) — 10+ questions with difficulty (🟢🟡🔴), answer, explanation
- `coverage` (list) — which chapters are covered

## Validation Rules

| Rule | Entity | Condition |
|------|--------|-----------|
| V-001 | SourceModule | `warnings == 0` (compiler `-Werror`) |
| V-002 | SourceModule | `valgrind_clean == true` for memory management chapters |
| V-003 | TutorialChapter | All 15 mandatory sections present (R-003) |
| V-004 | SourceModule | integrated: `main_<topic>()` declared in header |
| V-005 | SourceModule | sample: exposes `main_<topic>()`, file ends in `_sample.c`, no `main()` |
| V-006 | SourceModule | All memory allocations freed, NULL-checked before dereference |
| V-007 | TutorialChapter | `language == "bilingual"` — English terms in parentheses |
| V-008 | ChapterOverview | All TutorialChapters listed in chapter_table |
| V-009 | SourceModule | Header has include guard (`#ifndef/<name>_H/#define`);

## State Transitions

Not applicable — entities are static files, not runtime stateful objects.
