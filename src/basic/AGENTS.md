# src/basic/ Module Knowledge Base

**Generated:** 2026-04-26

## OVERVIEW

C tutorial chapter: datatypes, strings, arrays, time, structs, Unicode. Largest source file is `datatype_sample.c` (276 lines).

## STRUCTURE

```
basic/
├── basic.c               # main_basic() coordinator, calls all sample functions
├── basic.h               # Declares main_basic()
├── datatype_sample.c     # ALL content: strings, arrays, date, structs, Unicode
└── datatype_sample.h     # String/array/date/struct function declarations
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Variable demos | `basic.c:8-43` | `int`, `long`, `long long`, `int64_t`, `const` |
| String ops | `datatype_sample.c:47-100` | strncpy, strncat, strstr, strtok, sprintf, sprintf |
| Unicode/UTF-8 | `datatype_sample.c:19-45` | char16_t/char32_t, mbrtoc32, macOS compatibility shim |
| Array demo | `datatype_sample.c:102-120` | int array, pointer arithmetic |
| Date/time | `datatype_sample.c:122-170` | time_t, localtime, strftime, gettimeofday |
| Structs | `datatype_sample.c:172-276` | Stack vs heap, Person struct, nested structs |

## CONVENTIONS

- All sample functions declared in `datatype_sample.h`, called via `main_basic()` in `basic.c`
- Safe string functions used (`strncpy`, `strncat`) alongside unsafe (`strcpy`, `sprintf`) — pedagogical mix
- macOS lacks `<uchar.h>` — `datatype_sample.c` manually defines `char16_t`/`char32_t` as fallback

## ANTI-PATTERNS (THIS DIR)

- **Unsafe ops for teaching**: `strcpy(p1.name, "张三")` on line 213 — stack buffer, no bounds check
- **Uninitialized array**: `scores[100]` on line 105 — all elements except [0] and [99] are indeterminate
- **Missing header**: `datatype_sample.c` declares `string_sample()` returns `int` but impl returns `void`
- **Platform-specific**: `<sys/signal.h>` and `<sys/time.h>` used unconditionally (POSIX only)
