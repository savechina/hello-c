# Research & Decisions: C Advance Tutorial

**Feature**: 002-c-advance-tutorial
**Date**: 2026-04-27

## R-001: hello-rust Advance Mapping to C

Decision: Map each Rust advance topic to C equivalent paradigm.

| hello-rust | C Equivalent | Rationale |
|------------|-------------|-----------|
| smart-pointers | Opaque pointers + RAII macros | C has no RAII; use macros for cleanup patterns, opaque pointers for encapsulation |
| atomic-types | C11 `<stdatomic.h>` | Direct equivalent; volatile for non-atomic hardware registers |
| async | POSIX threads + select/poll | C has no async/await; pthread for concurrency, poll/select for I/O multiplexing |
| iterators | Linked lists, dynamic arrays, trees | C iterators are manual pointer-based traversal; data structures chapter covers this |
| advanced-traits | Function pointer tables, vtable pattern | C has no traits/mirrors Rust trait dispatch via struct of function pointers |
| error-handling | errno, setjmp/longjmp, error callbacks | C error patterns (vs Rust's Result/panic) |
| system | POSIX syscalls: signals, mmap, process mgmt | System programming in C |
| testing | Custom ASSERT macros, test harness | C has no built-in testing; write mini framework |
| database | SQLite3 C API | C-compatible database (no ORM in C) |
| web | HTTP server with sockets | Bare-bones socket programming |
| tools | Build patterns, coverage, static analysis | Developer tools for C projects |
| review-advance | Comprehensive review | Mirror hello-rust review-advance.md |

Alternatives considered:
- Use C++ features (smart pointers, std::thread) → rejected, this is C17 not C++
- Use external libraries (libuv, Boost) → rejected, use POSIX/C standard only
