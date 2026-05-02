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

## Entity: TestFile

Unity test file in `test/` directory.

**Attributes**: path (`test/test_*.c`), framework (Unity), tested_module (reference to `src/advance/*.c`), test_cases (list of `test_*()` functions), setup_teardown (optional setUp/tearDown).

## Entity: TestVendor

External test framework source files in `test/vendor/`.

**Attributes**: name (unity/cmock), version, source_files (list), config_flags (compiler defines).

**Relationships**: TestFile depends on TestVendor (Unity).

## Entity: CalcModule (NEW for testability)

`src/advance/calc.c` + `src/advance/calc.h` — extract testable functions from `testing_sample.c`.

**Attributes**: functions (`calc_add`, `calc_multiply`, `calc_is_valid`), visibility (non-static for testing), header (`calc.h`).

**Relationships**: TestFile tests CalcModule.
