# src/ Module Knowledge Base

**Generated:** 2026-04-26

## OVERVIEW

Sole module directory. All source `.c`/`.h` files live here in flat or subdirectory layout.

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Entry point | `main.c` | Calls `main_hello()`, `main_sysinfo()` — orchestration only |
| Main demo | `hello.c/.h` | Fibonacci(factorial), calls `main_basic()` + `main_advance()` |
| System info | `sysinfo.c/.h` | 327-line multi-platform: macOS/Linux/Solaris/FreeBSD via `#ifdef` |
| Tutorial stubs | `advance/`, `algo/` | advance = printfs only; algo = has `.c` with `main_algo_sample()` |
| Quick examples | `module1/`, `util/` | `print_hello()`, `print_util()` — boilerplate demos |

## CONVENTIONS

- Each module exposes `main_<module>()` as its entry point
- Header guards: `#ifndef <NAME>_H / #define <NAME>_H / #endif`
- Includes: system headers first (sorted), then local `"...h"`
- Local headers use path includes: `"advance/advance.h"`, `"basic/basic.h"`

## ANTI-PATTERNS (THIS DIR)

- **Advance stubs**: `advance/` chapters are printfs only — no real implementations yet
- **Duplicate decl**: `main_hello()` in both `../include/global.h` and `hello.h` — kept intentionally
- **algo.h naming**: `algo/algo.h` declares `main_algo()` but `algo.c` implements `main_algo_sample()` — kept for future algo implementations
