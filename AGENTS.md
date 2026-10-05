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
├── include/                # global.h — project-wide declarations (main_hello, etc.)
├── src/
│   ├── main.c            # SOLE entry point — orchestration only
│   ├── hello.c/.h        # Main demo: factorial, structs, basic/advance
│   ├── sysinfo.c/.h      # Multi-platform OS detection (327 lines, macOS/Linux/Solaris/FreeBSD)
│   ├── basic/            # ~45 chapters, one `_sample.c` per topic (memory/strings/structs/IO)
│   ├── advance/          # ~24 chapters with real implementations (threads, memory safety, net)
│   ├── algo/             # Has algo.c with main_algo_sample() + sorting demos
│   ├── awesome/          # Curated resources
│   ├── module1/          # Example: print_hello
│   └── util/             # Example: print_util (renamed from module2)
├── test/                 # Unity/CMock tests, mirrors src/ layout
│   ├── vendor/          # Unity v2.6.1, CMock v2.6.0
│   ├── advance/         # Tests for advance/ modules (test_calc_*.c, test_raii_safety.c)
│   └── mocks/           # Generated CMock mocks (gitignored)
├── docs/                 # mdBook (book.toml → GitHub Pages)
└── build/                # Generated files (gitignored)
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Add new tutorial chapter | `src/basic/` or new subdir + header | Follow `main_<topic>_sample()` pattern — every chapter is `<topic>_sample.c` + `.h` |
| Add platform support | `src/sysinfo.c` `#elif defined(...)` block | Check `Dockerfile` for cross-compiler |
| Modify build flags | `Makefile` CFLAGS line | Don't forget `-D__PLATFORM__` for sysinfo |
| Update docs | `docs/src/` + `SUMMARY.md`, then push | mdBook auto-deploys via GitHub Actions |
| **Check memory safety** | `make asan` | ASan+UBSan gate; run before committing |
| **Check data races** | `make tsan` | ThreadSanitizer gate; `test/tsan.supp` suppresses only the 2 intentional race demos |
| **Find leaks/double-free** | `make analyze` | GCC `-fanalyzer`; needs real GCC (`make analyze CC=gcc-14`) |

## CODE MAP

| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `main()` | function | src/main.c:22 | Sole entry point; argv dispatch (basic/advance/algo/awesome/list) |
| `main_hello()` | function | src/hello.c:16 | Demo coordinator |
| `main_sysinfo()` | function | src/sysinfo.c:41 | Platform info dispatcher |
| `main_basic_sample()` | function | src/basic/basic.c:4 | Basic concepts coordinator |
| `main_advance_sample()` | function | src/advance/advance.c:4 | Advance coordinator |
| `main_raii_sample()` | function | src/advance/raii_sample.c | Memory-safety chapter (cleanup attr, arena, Option<T>, Buf64) |
| `get_system_info()` | function | src/sysinfo.c:55+ | Platform-specific, #ifdef-gated |
| `create_person()` | function | src/basic/memory_mgmt_sample.c:211 | Heap allocation demo (returns raw ptr, caller frees) |
| `calc_add()` | function | src/advance/calc.c:13 | Non-static addition for Unity testing |
| `calc_multiply()` | function | src/advance/calc.c:24 | Non-static multiplication |
| `calc_is_valid()` | function | src/advance/calc.c:34 | Result range validation (0-10000) |

## CONVENTIONS

- **Modules**: Each `.c` exposes `main_<module>_sample()` entry + module-local functions
- **Headers**: `#ifndef NAME_H / #define NAME_H / #endif` — all have guards ✓
- **Includes**: System headers sorted, then project headers. Some use subdirectory paths (`"advance/advance.h"`)
- **C Standard**: C17 (`-std=c17`), `<stdint.h>` types preferred
- **Compiler flags**: `-Wall -Wextra -Werror -g -O2 -MMD`
- **Comments**: Bilingual (Chinese primary + English technical terms)
- **Naming**: `snake_case` functions, `PascalCase` structs/typedefs

## ANTI-PATTERNS (THIS PROJECT)

- **Makefile globstar**: `**.c` non-portable (works on macOS, not all Make versions)
- **Duplicate declarations**: `main_hello()` declared in both `include/global.h` AND `src/hello.h` — kept intentionally
- **Deliberate unsafe call**: `src/basic/logging_sample.c:34` `sprintf` is an intentional counter-example (comment says 危险！无边界检查), wrapped in a scoped `#pragma clang diagnostic ignored "-Wdeprecated-declarations"` because `-fsanitize=address` makes ASan re-resolve the deprecated declaration and `-Werror` would fail the build

## UNIQUE STYLES

- **Multi-platform sysinfo**: `src/sysinfo.c` handles macOS/Linux/Solaris/FreeBSD in single file via `#ifdef` maze
- **Tutorial pattern**: Each `.c` exposes `main_*()` for sequential demo execution
- **Bilingual**: Chinese comments with English code — intentional for target audience

## COMMANDS

```bash
make build     # Compile all src/**/*.c → build/bin/hello
make asan      # Build + run under AddressSanitizer + UBSan (memory-safety gate)
make analyze   # GCC -fanalyzer (needs real GCC: make analyze CC=gcc-14)
make test      # Compile and run Unity tests
make test-asan # Run Unity tests under ASan+UBSan (catches what plain `test` cannot)
make tsan      # Build + run under ThreadSanitizer (data-race gate)
make run       # Build + execute
make clean     # Remove build/
make help      # Show usage
```

## NOTES

- No `.clang-format`, `.clang-tidy`, or `.editorconfig` — formatting is manual
- `Makefile.old` kept for reference — don't use
- `Dockerfile` has hardcoded proxy (`192.168.2.7:1087`) and Aliyun mirrors — adjust for your environment
- Valgrind has **no arm64-Darwin support** — on Apple Silicon use `make asan` instead of `make test-valgrind`
- LeakSanitizer is Linux-only; `make asan`/`make test-asan` set `detect_leaks=0` on Darwin. Use `MallocStackLogging=1 leaks --atExit` for leak hunting there — leaks are NOT auto-detected on macOS
- On macOS `gcc` is Apple clang, which lacks `-fanalyzer`; `make analyze` probes and skips
- **Test coverage**: `calc`, `raii_sample`, `async_thread`, `variables`, `type_generic`, `static_assert`, `designated_init`, `integer_safety`, `volatile`, `strict_aliasing` and `restrict` have Unity tests (13 binaries; `test/basic/` exists via `test/basic/test_variables_sample.c`)
- The test link rule links every object except `main.o`, so adding a test needs no Makefile change

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

## Skill routing

When the user's request matches an available skill, invoke it via the Skill tool. When in doubt, invoke the skill.

Key routing rules:
- Product ideas/brainstorming → invoke /office-hours
- Strategy/scope → invoke /plan-ceo-review
- Architecture → invoke /plan-eng-review
- Design system/plan review → invoke /design-consultation or /plan-design-review
- Full review pipeline → invoke /autoplan
- Bugs/errors → invoke /investigate
- QA/testing site behavior → invoke /qa or /qa-only
- Code review/diff check → invoke /review
- Visual polish → invoke /design-review
- Ship/deploy/PR → invoke /ship or /land-and-deploy
- Save progress → invoke /context-save
- Resume context → invoke /context-restore
- Author a backlog-ready spec/issue → invoke /spec
