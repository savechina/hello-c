# hello-c Project Knowledge Base

**Generated:** 2026-04-26
**Commit:** fc6fd65
**Branch:** main

## OVERVIEW

C programming tutorial — beginner's guide. Single Makefile build, modular `src/` layout,
multi-platform sysinfo demos (macOS/Linux/Solaris/FreeBSD). Documentation via mdBook → GitHub Pages.

## STRUCTURE

```
hello-c/
├── Makefile              # GNU Make, C17, out-of-tree build (build/obj/, build/bin/)
├── Makefile.old          # Legacy clang-based, reference only
├── Dockerfile            # Ubuntu 24.04 dev container (full toolchain + mingw + zsh)
├── Dockerfile.v24        # Lighter 24.04 (no mingw/zsh)
├── Dockerfile.v18        # Legacy 18.04 (rustup-init.sh, protobuf)
├── include/
│   └── global.h          # Project-wide decls (DUPLICATE: also declares main_hello)
├── src/
│   ├── main.c            # SOLE entry point — orchestration only
│   ├── hello.c/.h        # Main demo: fibonacci(factorial?), structs, basic/advance
│   ├── sysinfo.c/.h      # Multi-platform OS detection (327 lines, macOS/Linux/Solaris/FreeBSD)
│   ├── h.c               # ORPHAN main_h() — never called, no header
│   ├── basic/            # Tutorial: datatypes, strings, structs, Unicode
│   ├── advance/          # Stub — only printfs, no content yet
│   ├── algo/             # ORPHAN header only (no impl)
│   ├── module1/          # Example: print_hello
│   └── module2/          # Example: print_util
├── test/                 # Unity/CMock tests, mirrors src/ layout
│   ├── vendor/          # Unity v2.6.1, CMock v2.6.0
│   ├── advance/         # Tests for advance/ modules (e.g., test_calc_add.c)
│   ├── basic/           # Tests for basic/ modules
│   └── mocks/          # Generated CMock mocks (gitignored)
├── docs/                 # mdBook (book.toml → GitHub Pages)
└── build/                # Generated files (gitignored)
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Add new tutorial chapter | `src/basic/` or new subdir + header | Follow `main_<topic>()` pattern |
| Add platform support | `src/sysinfo.c` `#elif defined(...)` block | Check `Dockerfile` for cross-compiler |
| Modify build flags | `Makefile` CFLAGS line | Don't forget `-D__PLATFORM__` for sysinfo |
| Update docs | `docs/src/` + `SUMMARY.md`, then push | mdBook auto-deploys via GitHub Actions |

## CODE MAP

| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `main()` | function | src/main.c:7 | Sole entry point |
| `main_hello()` | function | src/hello.c:16 | Demo coordinator |
| `main_sysinfo()` | function | src/sysinfo.c:41 | Platform info dispatcher |
| `main_basic()` | function | src/basic/basic.c:49 | Basic concepts demo |
| `main_advance()` | function | src/advance/advance.c:4 | Stub |
| `get_system_info()` | function | src/sysinfo.c:55+ | Platform-specific, #ifdef-gated |
| `create_person()` | function | src/basic/datatype_sample.c:183 | Heap allocation demo |
| `calc_add()` | function | src/advance/calc.c:13 | Non-static addition for Unity testing |
| `calc_multiply()` | function | src/advance/calc.c:24 | Non-static multiplication |
| `calc_is_valid()` | function | src/advance/calc.c:34 | Result range validation (0-10000) |

## CONVENTIONS

- **Modules**: Each `.c` exposes `main_<module>()` entry + module-local functions
- **Headers**: `#ifndef NAME_H / #define NAME_H / #endif` — all have guards ✓
- **Includes**: System headers sorted, then project headers. Some use subdirectory paths (`"advance/advance.h"`)
- **C Standard**: C17 (`-std=c17`), `<stdint.h>` types preferred
- **Compiler flags**: `-Wall -Wextra -g -O2 -MMD`
- **Comments**: Bilingual (Chinese primary + English technical terms)
- **Naming**: `snake_case` functions, `PascalCase` structs/typedefs

## ANTI-PATTERNS (THIS PROJECT)

- **fibonacci/factorial bug**: `src/hello.c:7-11` — named `fibonacci()` but implements factorial logic (n * recursive, not F(n-1)+F(n-2))
- **Duplicate declarations**: `main_hello()` declared in both `include/global.h` AND `src/hello.h`
- **Orphan code**: `src/h.c` (`main_h()`) — never called anywhere
- **Orphan header**: `src/algo/algo.h` — declares `main_algo()`, no `.c` implementation
- **Dead `#include`**: `src/main.c:6` — `// #include "global.h"` leftover from refactoring
- **Makefile duplicate**: `-include $(OBJECTS:.o=.d)` appears twice (lines 69 & 100) ✅ FIXED
- **Makefile globstar**: `**.c` non-portable (works on macOS, not all Make versions)
- **Makefile circular**: `build <- build` dependency dropped ✅ FIXED (removed `build/` from `DIRS`, removed `clean` from `build` target)

## UNIQUE STYLES

- **Multi-platform sysinfo**: `src/sysinfo.c` handles macOS/Linux/Solaris/FreeBSD in single file via `#ifdef` maze
- **Tutorial pattern**: Each `.c` exposes `main_*()` for sequential demo execution
- **Bilingual**: Chinese comments with English code — intentional for target audience

## COMMANDS

```bash
make build     # Compile all src/**/*.c → build/bin/hello
make test      # Compile and run Unity tests
make run       # Build + execute
make clean     # Remove build/
make help      # Show usage
```

## NOTES

- No `.clang-format`, `.clang-tidy`, or `.editorconfig` — formatting is manual
- `Makefile.old` kept for reference — don't use
- `Dockerfile` has hardcoded proxy (`192.168.2.7:1087`) and Aliyun mirrors — adjust for your environment
- `hello.c:17` — `fibonacci(60)` will overflow; the function is actually factorial
- `src/basic/datatype_sample.c` mixes safe (`strncpy`) and unsafe (`strcpy`, `sprintf`) — pedagogical intent

## Active Technologies
- C17 (ISO/IEC 9899:2018) | gcc 12+ or clang 15+ + POSIX C standard library, `<stdint.h>`, `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`, `<math.h>`, `<limits.h>`, `<time.h>`, `<unistd.h>`, `<errno.h>`, `<stdarg.h>` (001-c-basic-tutorial)
- N/A — tutorial is stateless code examples (001-c-basic-tutorial)
- C17, gcc 12+ or clang 15+ + POSIX C (pthread, signal), C11 stdatomic.h, SQLite3 (database chapter) (002-c-advance-tutorial)
- N/A — tutorial code examples (SQLite demo uses temp files) (002-c-advance-tutorial)
- C17 (ISO/IEC 9899:2018) | gcc 12+ or clang 15+ + POSIX C standard library, `<stdint.h>`, `<stdio.h>`, Unity v2.6.1 (test framework), CMock v2.6.0 (mock generator) (002-c-advance-tutorial)
- Unity v2.6.1: Lightweight C test framework (3 source files: unity.c, unity.h, unity_internals.h), zero dependencies, header-only integration (002-c-advance-tutorial)
- CMock v2.6.0: Ruby-based mock generator, automatically creates mock functions from C headers (test/vendor/cmock/) (002-c-advance-tutorial)
- C17 (ISO/IEC 9899:2018) | gcc 12+ or clang 15+ + POSIX C standard library, `<stdint.h>`, `<stdio.h>`, etc. (002-c-advance-tutorial)

## Recent Changes
- 001-c-basic-tutorial: Added C17 (ISO/IEC 9899:2018) | gcc 12+ or clang 15+ + POSIX C standard library, `<stdint.h>`, `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`, `<math.h>`, `<limits.h>`, `<time.h>`, `<unistd.h>`, `<errno.h>`, `<stdarg.h>`
