# hello-c

Hello C, This is a concise tutorial for learning C, a beginner's guide to C programming.

> 🚧 **Under Construction**: This project is being actively updated with 27 new basic tutorial chapters.
> See [Basic Tutorial](docs/src/basic/basic_overview.md) for the full learning path.

## Quick Start

```bash
make build   # Compile all source files
make run     # Build + run all tutorial chapters sequentially
make clean   # Clean build artifacts
make help    # Show all available targets
```

## Tutorial Structure

### Basic — C 基础入门 (27 Chapters)

A comprehensive C programming tutorial designed for beginners, mirroring the structure of [hello-rust](../hello-rust/docs/src/basic/).

Written in **first-person narrative** (个人学习经验总结), with **error-first learning** (show the mistake → compiler error → fix), and **cognitive aids** (memory diagrams, Python/C comparisons, 3-level exercises).

| Tier      | Chapters | Topics                                                     | Time   |
| --------- | -------- | ---------------------------------------------------------- | ------ |
| 🟢 Easy   | 8        | Variables, Data Types, Functions, Operators, Arrays, Flow  | ~3.5h  |
| 🟡 Medium | 7        | Pointers, Strings, Structs, Enums, Scope                   | ~4.5h  |
| 🔴 Hard   | 12       | Memory Management, File I/O, Function Pointers, void*, etc | ~6.5h  |

**Complete chapter list**: [basic_overview.md](docs/src/basic/basic_overview.md)

**Key Resources**:
- [C 术语表](docs/src/basic/basic_glossary.md) — 46 bilingual entries
- [基础阶段复习](docs/src/basic/review_basic.md) — 20 questions with answers
- [学习路径图](docs/src/basic/basic_overview.md) — Visual learning path

### Advance

Advanced C topics — coming soon.

### Algo

Algorithm demonstrations — coming soon.

### Awesome

Curated C programming resources — coming soon.

## Development

### Build System

```bash
make build           # Compile all src/**/*.c into build/bin/hello
make run             # Build + execute
make clean           # Remove build/
make help            # Show all targets
```

All source files in `src/` are automatically picked up by the Makefile glob pattern. Each tutorial chapter uses the `_sample.c` + `_sample.h` convention and exposes a `main_<topic>_sample()` entry function.

### Code Style

- **Language**: C17 (`-std=c17`)
- **Compiler**: `gcc -Wall -Wextra -Werror`
- **Naming**: `snake_case` functions, `PascalCase` structs, `<topic>_sample.c` for chapter files
- **Headers**: Include guards (`#ifndef TOPIC_H / #define TOPIC_H / #endif`)
- **Safety**: Bounded string ops (`strncpy` over `strcpy`, `snprintf` over `sprintf`), NULL checks after `malloc`

## Documentation

The tutorial is published as an [mdBook](https://rust-lang.github.io/mdBook/) site on GitHub Pages.

```bash
cd docs && mdbook serve --open  # Local preview
```

## Related Projects

- [hello-rust](../hello-rust/) — Rust basic tutorial (structure reference)
- [hello-python](../hello-python/) — Python basic tutorial
