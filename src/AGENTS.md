# src/ Module Knowledge Base

**Generated:** 2026-04-26 · **Reviewed:** 2026-10-04

## OVERVIEW

Sole module directory. All source `.c`/`.h` files live here in flat or subdirectory layout.

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Entry point | `main.c:22` | `main(int argc, char *argv[])` — argv dispatch only |
| Main demo | `hello.c:16` | `factorial()` demo; calls `main_basic_sample()` + `main_advance_sample()` |
| System info | `sysinfo.c:41` | 327-line multi-platform: macOS/Linux/Solaris/FreeBSD via `#ifdef` |
| Basic chapters | `basic/` | ~45 chapters, one `_sample.c` each |
| Advance chapters | `advance/` | ~24 chapters with real implementations (threads, memory safety, net) |
| Algorithms | `algo/algo.c:125` | `main_algo_sample()` + sorting demos |
| Quick examples | `module1/`, `util/` | `print_hello()`, `print_util()` — boilerplate demos |

## CONVENTIONS

- Each module exposes `main_<module>_sample(void)` as its entry point
- Header guards: `#ifndef <NAME>_H / #define <NAME>_H / #endif`
- Includes: system headers first (sorted), then local `"...h"`
- Local headers use path includes: `"advance/advance.h"`, `"basic/basic.h"`

## MEMORY-SAFETY CHAPTERS

| Topic | Location |
|-------|----------|
| RAII / scoped teardown | `advance/raii_sample.c` — `__attribute__((cleanup))`, DEFER macro, arena, `Option<T>`, `Buf64` |
| Opaque pointers, refcount | `advance/smart_pointers_sample.c` — factory/destroy, `free_item` destructor, RAII macros |
| Linked list / tree / dynarray | `advance/iterators_sample.c` — node alloc+delete, `realloc` growth |
| Strict aliasing / effective type | `advance/strict_aliasing_sample.c` — legal puns via `memcpy`, illegal casts as comments only |
| restrict contract | `advance/restrict_sample.c` — non-overlap contract for vectorizable buffers |
| Heap basics | `basic/memory_mgmt_sample.c` — `create_person()` factory at `:211` |
| Lifetime / UAF demos | `basic/scope_sample.c` |

## ANTI-PATTERNS (THIS DIR)

- **Duplicate decl**: `main_hello()` in both `../include/global.h` and `hello.h` — kept intentionally
- **Detached threads**: `advance/async_thread_sample.c` — must pass values via
  `(void *)(intptr_t)N`, never `&stack_local`, since a detached thread outlives its parent's frame
- **Unchecked allocation**: every `malloc`/`calloc` result must be checked before dereference
