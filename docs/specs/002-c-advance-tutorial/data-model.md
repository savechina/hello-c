# Data Model: C Advance Tutorial

**Feature**: 002-c-advance-tutorial

## Entity: AdvanceChapter

mdBook file in `docs/src/advance/` teaching advanced C concepts.

**Attributes**: Same as TutorialChapter — title, difficulty (🟡/🔴), sections, voice, bilingual.

## Entity: AdvanceSource

`_sample.c` + `_sample.h` pair in `src/advance/`.

**Attributes**: type (integrated/sample), chapter_ref, c/h/sample files, entry function `main_<topic>_sample()`, valgrind_clean=true.

## Entity: AdvanceCoordinator

`advance.c` + `advance.h` — coordinator calling all advance samples.

**Attributes**: calls main_*_sample() sequentially, exposed via main_advance() from hello.c.

## Entity: AdvanceOverview

`advance-overview.md` — landing page with chapter table + learning path.

## Entity: AdvanceReview

`review-advance.md` — comprehensive review chapter.
