# src/basic/ Module Knowledge Base

**Generated:** 2026-04-26 · **Reviewed:** 2026-10-04

## OVERVIEW

~47 beginner chapters. Convention is one `<topic>_sample.c` + `<topic>_sample.h` pair per
topic; each exposes `main_<topic>_sample(void)` which `basic.c` calls in sequence.

## STRUCTURE

```
basic/
├── basic.c                 # main_basic_sample() — calls 41 sample functions in order
├── basic.h
├── datatype_sample.c       # int/float/char + string/date samples (69 lines)
├── memory_mgmt_sample.c    # malloc/calloc/realloc/free, create_person() factory
├── scope_sample.c          # stack vs heap lifetime, UAF/dangling demos
├── safe_strings_sample.c   # bounded string ops, snprintf vs sprintf
├── strings_sample.c        # strncpy/strtok/strstr edge cases
└── ...                     # ~40 more _sample.c chapters (structs, unions, enums, IO, …)
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Coordinator | `basic.c:4` `main_basic_sample(void)` | Calls each `main_<topic>_sample()` |
| Heap allocation | `memory_mgmt_sample.c` | Canonical realloc-via-temp at `:122-138` |
| Object factory | `memory_mgmt_sample.c:211` `create_person()` | Returns raw `Person *`, caller frees at `:239` |
| Pointer lifetime | `scope_sample.c` | `return_heap_address()` at `:132` — caller-must-free contract |
| Safe string ops | `safe_strings_sample.c` | Non-literal format at `:64-65` is a teaching demo |
| Int/float/char basics | `datatype_sample.c:14` | Only 69 lines now — not the old 276-line monolith |

## CONVENTIONS

- Every chapter exposes `main_<topic>_sample(void)`; declared in its own `_sample.h`
- Bilingual comments: Chinese prose + English technical terms
- Bounded string ops by default (`strncpy` over `strcpy`, `snprintf` over `sprintf`)
- Structs embedding fixed buffers (`char name[32]`) are the norm, not heap-allocated

## ANTI-PATTERNS (THIS DIR)

- **Deliberate unsafe call**: `logging_sample.c:34` `sprintf` — intentional counter-example
  (comment says 危险！无边界检查). Requires a scoped
  `#pragma clang diagnostic ignored "-Wdeprecated-declarations"`, because
  `-fsanitize=address` makes ASan re-resolve the deprecated declaration and `-Werror`
  would otherwise fail the build.
- **Signed overflow is UB, not wraparound**: `variables_sample.c` now teaches this correctly —
  do not reintroduce `INT32_MAX + 1` on a signed type as a "wraps to negative" demo.
- **Platform-specific**: `<sys/signal.h>` and `<sys/time.h>` used unconditionally (POSIX only)
