# Hello C

English | [简体中文](README.zh.md)

[![Docs](https://img.shields.io/badge/docs-online-blue)](https://savechina.github.io/hello-c/)
[![License](https://img.shields.io/badge/license-Apache%202.0-green)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/savechina/hello-c?style=social)](https://github.com/savechina/hello-c)

[**Hello C Tutorial**](https://renyan.org/hello/c) | [**GitHub Pages**](https://savechina.github.io/hello-c/)

A comprehensive C programming tutorial for beginners, designed with **error-first learning** and **bilingual (Chinese/English)** instruction. Covers everything from variables to memory management across 50+ progressive chapters — with runnable code examples, multi-platform system info demos, and a full test suite.

## 📖 Online Tutorial

- 🌐 **Official Site**: [renyan.org/hello/c](https://renyan.org/hello/c)
- 📚 **GitHub Pages**: [savechina.github.io/hello-c](https://savechina.github.io/hello-c/)

## ✨ Features

- **🎯 Error-First Learning** — Each concept is introduced by showing the mistake first, then the compiler error, then the fix. You learn *why* rules exist by breaking them.
- **🌏 Bilingual by Design** — Chinese explanations paired with English code terminology and technical terms. Built for Chinese-speaking learners who want to read real-world C code.
- **🧠 Cognitive Aids** — Memory diagrams visualize stack/heap layout, Python/C side-by-side comparisons bridge from higher-level languages, and every chapter has 3-level exercises.
- **🖥️ Multi-Platform Sysinfo** — A single-file [`sysinfo.c`](src/sysinfo.c) that detects macOS, Linux, Solaris, and FreeBSD via `#ifdef` maze — a real-world demonstration of conditional compilation.
- **🧪 Tested with Unity + CMock** — Every module has corresponding unit tests under `test/`, using the Unity test framework with CMock-generated mocks.
- **📚 mdBook Documentation** — Full tutorial published as an [mdBook](https://rust-lang.github.io/mdBook/) site, auto-deployed to GitHub Pages.
- **🔧 C17 with Strict Flags** — Compiled with `-std=c17 -Wall -Wextra -Werror`, promoting safe and portable C from day one.

## 🚀 Quick Start

```bash
git clone https://github.com/savechina/hello-c.git
cd hello-c
make run     # Build + run all tutorial chapters sequentially
make test    # Compile and run Unity tests
```

## 📦 Project Modules

### Basic — C 基础入门 (33 Chapters)

Core C syntax and concepts for beginners, organized by difficulty (top-level chapters; hub pages expand into sub-chapters):

| Tier | Chapters | Topics |
|------|----------|--------|
| 🟢 Easy (13) | [Variables](docs/src/basic/variables.md), [Data Types](docs/src/basic/datatype.md), [Operators](docs/src/basic/operators.md), [Control Flow](docs/src/basic/control_flow.md), [Loops](docs/src/basic/loops.md), [Functions](docs/src/basic/functions.md), [Arrays](docs/src/basic/arrays.md), [Preprocessor](docs/src/basic/preprocessor.md), [Conditional Comp](docs/src/basic/conditional_comp.md), [Typedef](docs/src/basic/typedef.md), [Static Assert](docs/src/basic/static_assert.md), [Logging](docs/src/basic/logging.md), [CLI Args](docs/src/basic/cli_args.md) | Syntax, flow control, fundamentals |
| 🟡 Medium (14) | [Pointers](docs/src/basic/pointers.md), [Strings](docs/src/basic/strings.md), [Structs](docs/src/basic/structs.md), [Enums](docs/src/basic/enums.md), [Scope](docs/src/basic/scope.md), [Unions](docs/src/basic/unions.md), [Headers & Modules](docs/src/basic/headers.md), [Debugging](docs/src/basic/debugging.md), [File I/O](docs/src/basic/file_io.md), [Bit Ops](docs/src/basic/bit_ops.md), [Stdlib](docs/src/basic/stdlib.md), [Recursion](docs/src/basic/recursion.md), [Type Generic](docs/src/basic/type_generic.md), [Designated Init](docs/src/basic/designated_init.md) | Memory, data structures, intermediate patterns |
| 🔴 Hard (6) | [Memory Mgmt](docs/src/basic/memory_mgmt.md), [Function Pointers](docs/src/basic/function_pointers.md), [Callbacks](docs/src/basic/callbacks.md), [Void* Generic](docs/src/basic/void_generic.md), [Variadic Functions](docs/src/basic/variadic_functions.md), [Integer Safety](docs/src/basic/integer_safety.md) | Memory safety, advanced C concepts |

**Complete chapter list**: [basic_overview.md](docs/src/basic/basic_overview.md) · [Glossary (46 entries)](docs/src/basic/basic_glossary.md) · [Review (20 questions)](docs/src/basic/review_basic.md)

### Sysinfo — Multi-Platform System Info

| File | Content |
|------|---------|
| [`sysinfo.c`](src/sysinfo.c) | macOS, Linux, Solaris, FreeBSD detection in 327 lines via `#ifdef` |
| [`sysinfo.h`](src/sysinfo.h) | Public API: `get_system_info()`, `main_sysinfo()` |

### Advance — Advanced C (15 Chapters)

Production-grade C topics, each with a runnable sample and a walkthrough chapter:

- **Memory-safety engineering** — [cleanup/arena/Option](docs/src/advance/memory-safety.md), [strict aliasing & effective type](docs/src/advance/strict-aliasing.md), [restrict contract](docs/src/advance/restrict.md), [opaque pointers](docs/src/advance/opaque-pointers.md)
- **Concurrency** — [threads, sync, thread pools, select/poll](docs/src/advance/async.md), [atomics](docs/src/advance/atomic-types.md), [volatile](docs/src/advance/volatile.md)
- **System programming** — [file/signal/mmap/fork/IPC/CLI](docs/src/advance/system.md)
- **Web servers** — [raw-socket HTTP parsing](docs/src/advance/web/socket.md), [concurrent server model](docs/src/advance/web/concurrent.md)
- **Engineering practice** — [error handling](docs/src/advance/error-handling.md), [SQLite](docs/src/advance/database.md), [Unity testing](docs/src/advance/testing.md), [sanitizer toolchain](docs/src/advance/tools.md)

**Complete chapter list**: [advance/overview.md](docs/src/advance/overview.md)

### Algo — Algorithm Demos

Bubble sort, binary search, and big-number Fibonacci, explained in [算法示例](docs/src/algo/algo.md) (`src/algo/algo.c`).

### Awesome — Curated Resources

[资源精选](docs/src/awesome.md) — documents the `src/awesome/` scaffold honestly and the conventions for adding your own resource entries.

## 🛠️ Tech Stack

| Category | Choice |
|----------|--------|
| **Language** | C17 (`-std=c17`) |
| **Compiler** | GCC 12+ / Clang 15+ |
| **Build** | GNU Make, out-of-tree (`build/obj/`, `build/bin/`) |
| **Test Framework** | Unity v2.6.1 + CMock v2.6.0 |
| **Docs** | mdBook → GitHub Pages |
| **Platforms** | macOS, Linux, Solaris, FreeBSD |

## 📋 Project Structure

```
hello-c/
├── src/
│   ├── main.c              # Single entry point
│   ├── hello.c/.h          # Demo coordinator
│   ├── sysinfo.c/.h        # Multi-platform OS detection
│   ├── basic/              # 45 tutorial chapters (_sample.c + _sample.h)
│   ├── advance/            # 24 chapters: threads, system, web, memory safety
│   ├── algo/               # Algorithm demos
│   └── awesome/            # Curated-resources scaffold
├── test/
│   ├── vendor/             # Unity v2.6.1, CMock v2.6.0
│   ├── basic/              # Unit tests for basic/ modules
│   └── advance/            # Tests for advance/ modules
├── include/                # Project-wide headers (global.h)
├── docs/                   # mdBook documentation
├── Makefile                # GNU Make, C17, out-of-tree build
└── build/                  # Generated artifacts (gitignored)
```

## 📝 License

Apache 2.0 — see [LICENSE](LICENSE).

## 🔗 Related Projects

- [hello-rust](https://github.com/savechina/hello-rust) — Rust basic tutorial (structure reference)
- [hello-python](https://github.com/savechina/hello-python) — Python basic tutorial
- [hello-go](https://github.com/savechina/hello-go) — Go basic tutorial
- [hello-swift](https://github.com/savechina/hello-swift) — Swift basic tutorial
- [hello-ruby](https://github.com/savechina/hello-ruby) — Ruby basic tutorial
