<!--
SYNC IMPACT REPORT
==================
Version Change: 1.1.1 -> 2.0.0 (MAJOR: Complete rewrite from Rust to C engineering principles)
Modified Principles:
  - I. Code Quality: Rust 2024 idioms -> C17 standard, memory safety, header discipline
  - II. Test-First: TDD/cargo test -> C unit testing (Unity/CUnit), memory leak detection
  - III. UX Consistency: Rust CLI/gRPC -> C CLI conventions, multi-platform portability
  - IV. Performance: async/Tokio -> C stack/heap discipline, Valgrind profiling
  - V. SDD Harness: 8-phase workflow preserved, tooling adapted for C ecosystem
Added Sections:
  - Memory Safety & Resource Management (non-negotiable for C)
  - Header File Discipline & Include Guards
  - Multi-Platform Compatibility (macOS/Linux/Solaris/FreeBSD)
  - C-Specific Build & CI (Makefile, gcc/clang, no cargo)
Removed Sections:
  - All Rust-specific tooling (cargo clippy, cargo fmt, cargo test, tokio, serde, etc.)
  - gRPC/Axum/Tonic framework references
  - Rust crate ecosystem references
Templates Requiring Updates:
  - .specify/templates/plan-template.md: Language field needs C17 update (pending manual review)
  - .specify/templates/tasks-template.md: Python examples remain (pending manual review)
Follow-up TODOs:
  - TODO(PLAN_TEMPLATE): Update sample tasks from Python to C examples
  - TODO(TASKS_TEMPLATE): Replace Python paths with C module paths
==================
-->

# Hello-C Constitution

## Core Principles

### I. Memory Safety & Resource Management (NON-NEGOTIABLE)

All C code MUST manage memory explicitly and prevent leaks, double-frees, and
buffer overflows.

**Requirements:**
- Every `malloc`/`calloc`/`realloc` MUST have a corresponding `free` in the same
  logical scope or a documented ownership transfer
- ALL pointer dereferences MUST be NULL-checked before use
- Buffer operations MUST use bounded functions (`strncpy`, `snprintf`, `memcpy`
  with explicit size) — never `strcpy`, `sprintf`, `gets`
- Stack allocation preferred for small, lifetime-bound data; heap only for
  large or long-lived data
- Return codes MUST indicate success/failure; callers MUST check them
- No use-after-free, no dangling pointers, no uninitialized reads

**Quality Gates:**
- `valgrind --leak-check=full --error-exitcode=1 ./build/bin/hello` MUST pass
  with zero leaks and zero errors
- `gcc -fsanitize=address,undefined` builds MUST run cleanly
- Static analysis (`cppcheck` or `clang-tidy`) MUST produce zero warnings on
  production code

**Rationale:** C provides no garbage collection or borrow checker. Memory bugs
are the #1 cause of C security vulnerabilities. Discipline here is non-negotiable
for a learning project — students must internalize safe patterns from day one.

### II. Code Quality & C Standards (NON-NEGOTIABLE)

All code MUST follow C17 standard conventions with strict compiler discipline.

**Requirements:**
- Compiler: C17 standard (`-std=c17`) mandatory; no C++ extensions
- Compiler flags: `-Wall -Wextra -Werror` MUST pass with zero warnings
- All declarations MUST precede use; forward declarations required for
  cross-module calls
- Every `.c` file MUST have a corresponding `.h` header (or be internal-only
  with `static` visibility)
- All headers MUST use include guards (`#ifndef HEADER_H / #define HEADER_H`)
- Functions without parameters MUST use `(void)` not `()`
- `const` correctness: pointers to immutable data MUST be `const`
- No global mutable state; globals MUST be `const` or documented with rationale

**Documentation Quality:**
- All public functions MUST have Doxygen-style comments (`/** ... */`)
  describing purpose, parameters, return values, and error conditions
- Complex algorithms MUST include inline comments explaining the approach
- Code examples in docs MUST compile and link against the project

**Rationale:** Strict compiler warnings catch undefined behavior early. Header
discipline enforces clean module boundaries. Documentation ensures the tutorial
value is preserved.

**Quality Gates:**
- `make build` MUST complete with zero warnings
- `cppcheck --enable=all --error-exitcode=1 src/` MUST pass
- Header dependency graph MUST be acyclic (no circular includes)

### III. Modular Architecture & Separation of Concerns

The codebase MUST maintain clean module boundaries with well-defined interfaces.

**Requirements:**
- Each module (directory under `src/`) MUST have a single responsibility
- Public APIs are declared in headers under that module's directory or
  `include/`; internal helpers MUST be `static`
- Cross-module dependencies MUST flow downward: `main.c` -> feature modules
  -> utility modules; no upward or circular dependencies
- `include/` directory holds project-wide shared headers only; module-specific
  headers stay with their source
- Module count SHOULD stay under 10; if exceeded, justify with architectural review
- `main.c` MUST be thin: initialization, orchestration, and cleanup only — no
  business logic

**Module Structure:**
```
src/
├── main.c              # Entry point, orchestration only
├── hello.c/.h          # Feature module entry
├── sysinfo.c/.h        # Platform-specific system info
├── basic/              # Basic C concepts tutorial
├── advance/            # Advanced C concepts tutorial
├── module1/            # Example module
├── module2/            # Example module
└── algo/               # Algorithm implementations
include/
└── global.h            # Project-wide declarations
```

**Rationale:** Clean modules make the tutorial navigable. Each section of the
book should map to a module. Tight coupling destroys readability for learners.

### IV. Multi-Platform Portability

All code MUST compile and run correctly on macOS, Linux, and Solaris/FreeBSD.

**Requirements:**
- Platform-specific code MUST be isolated behind `#if defined(...)` guards
- No platform-specific assumptions (path separators, line endings, endianness)
- All `#include` directives for platform headers MUST be within the appropriate
  `#ifdef` block
- Makefile MUST support both `gcc` and `clang` without modification
- Cross-compilation support via `CROSS_COMPILE` prefix (existing pattern)
- Platform detection code MUST NOT leak into business logic

**Portability Anti-Patterns:**
- Hardcoded paths (`/tmp`, `/Users/`, `C:\`)
- Assuming `sizeof(long)` or `sizeof(int)` (use `<stdint.h>` types)
- Assuming availability of POSIX extensions without guards
- Using non-standard library functions without fallback implementations

**Testing Requirements:**
- Code MUST compile on at least macOS (clang) and Linux (gcc)
- Platform-specific features MUST have graceful degradation on unsupported systems
- `make build` MUST succeed on all supported platforms without modification

**Rationale:** The project already demonstrates multi-platform sysinfo code.
Portability is a core teaching objective — learners should see how to write
C that runs everywhere.

### V. Build Engineering & SDD Workflow

Specification Driven Development (SDD) workflows MUST follow the **8-Phase
Development Lifecycle** with triple quality gates.

**Development Phases:**

**Phase 0: Product Strategy & Requirements**
- `/office-hours` — Product discovery (YC 6-question forcing framework)
- `/plan-ceo-review` — Scope challenge (4 modes)
- `/speckit.specify` — Generate feature specifications
- **Quality Gate**: Metis intent analysis + Momus spec review (>=8/10)

**Phase 1: Technical Architecture & Design**
- `/speckit.plan` — Technical design with constitution check
- `/plan-eng-review` — Engineering review
- `/design-consultation` + `/plan-design-review` — Design system (UI projects)
- **Quality Gate**: Metis deep planning + Momus plan review (>=8/10)

**Phase 2: Task Decomposition**
- `/speckit.tasks` — Granular task breakdown (<4hr per task)
- `/speckit.analyze` — Cross-artifact consistency analysis
- **Quality Gate**: No CRITICAL/HIGH inconsistencies

**Phase 3: Quality Checklists**
- `/speckit.checklist` — Multi-domain checklists
- **Quality Gate**: 100% checklist coverage

**Phase 4: Implementation**
- `/speckit.implement` — Test-first execution with task delegation
- **Quality Gate**: `make build` success + zero compiler warnings
- **Manual Review**: Changes MUST be manually reviewed before commit
- **Manual Commit**: ALL commits MUST be manually committed and pushed by user
- **Prohibited**: NO automatic commits or pushes to remote repositories

**Phase 5: Testing & Validation**
- `make test` (when test harness exists) — Automated testing
- `/review` — Pre-landing PR review
- `/qa` — End-to-end QA testing
- **Quality Gate**: 100% tests pass + no CRITICAL issues

**Phase 6: Delivery & Release**
- `/document-release` — Update all documentation
- `/ship` — Merge, version bump, create PR (with user approval)
- **Quality Gate**: All quality gates passed
- **Manual Verification**: User MUST verify all changes before deployment

**Phase 7: Retrospective**
- `/retro` — Engineering retro with trend analysis
- **Output**: Improvement action items for next iteration

**Triple Quality Gates:**

| Gate | Role | Timing | Purpose |
|------|------|--------|---------|
| **Metis** | Pre-planning consultant | Before each phase | Intent analysis, ambiguity detection, routing strategy |
| ** Momus** | Post-delivery reviewer | After each phase | Clarity/verifiability/completeness evaluation |
| **GStack** | Professional specialist | During execution | Domain-specific expertise (reviews, QA, shipping) |

**Workflow Requirements:**
- Feature specifications via `/speckit.specify` (mandatory for all features)
- Implementation plans via `/speckit.plan` (mandatory before coding)
- Constitution check at Phase 1 (verify all 5 principles)
- All quality gates MUST pass before proceeding to next phase
- Document all decisions in `docs/specs/{N}-{feature}/`
- **Manual Control**: User MUST manually review, commit, and push all changes

**Workflow Enforcement:**
- No direct commits to `main` branch (use feature branches with PRs)
- All PRs MUST reference a spec document in `docs/specs/`
- **CRITICAL**: NO automatic commits or pushes — user maintains full control

**Tool Stack:**
- **Compiler**: gcc (Linux/Solaris) or clang (macOS/FreeBSD), C17 standard
- **Build**: GNU Make (`Makefile`)
- **Static Analysis**: `cppcheck`, `clang-tidy`
- **Memory Safety**: `valgrind`, AddressSanitizer (`-fsanitize=address`)
- **Testing**: Unity test framework or CUnit (TBD — test harness not yet established)
- **Speckit Framework**: 8-phase SDD workflow
- **GStack Skills**: Quality automation skills
- **OhMyOpenCode Agents**: Triple quality gates (metis, momus, oracle, explore, librarian)
- **Manual Commit Policy**: ALL commits require user review and manual execution

## Technology Stack

**Core:**
- Language: C17 (ISO/IEC 9899:2018)
- Compiler: gcc 12+ or clang 15+
- Standard Library: POSIX + C standard library only
- Integer Types: `<stdint.h>` (`int32_t`, `int64_t`, `uint8_t`, etc.)

**Build:**
- Build System: GNU Make
- Makefile: Out-of-tree build (`build/obj/`, `build/bin/`)
- Cross-compilation: `CROSS_COMPILE` prefix support
- Flags: `-Wall -Wextra -Werror -std=c17 -MMD -g -O2`

**Analysis & Safety:**
- Static Analysis: `cppcheck --enable=all`, `clang-tidy`
- Dynamic Analysis: `valgrind --leak-check=full`
- Sanitizers: `-fsanitize=address,undefined` (GCC/Clang)
- Memory Profiling: `heaptrack` (Linux), `Instruments` (macOS)

**Documentation:**
- Source Docs: `docs/` directory with markdown
- Tutorials: mdBook for structured documentation
- Primary Language: Chinese (Simplified) with English technical terms
- API Docs: Doxygen-style comments inline

**Target Platforms:**
- macOS (Apple Silicon + Intel)
- Linux (x86_64, ARM64)
- Solaris/Illumos
- FreeBSD

## Development Workflow

### Feature Development Lifecycle

1. **Specification** (`/speckit.specify`)
   - Create feature spec in `docs/specs/<###-feature-name>/spec.md`
   - Define user stories, acceptance criteria, success metrics
   - Quality checklist validation

2. **Planning** (`/speckit.plan`)
   - Technical design document
   - Architecture decisions with rationale
   - Constitution check (verify compliance with all 5 principles)
   - Dependency analysis

3. **Implementation** (`/speckit.tasks` -> `/speckit.implement`)
   - Granular task breakdown (<4hr per task)
   - Write code following C17 standards and memory safety rules
   - **Manual Review**: ALL changes MUST be manually reviewed
   - **Manual Commit**: User MUST execute all git commits
   - **Manual Push**: User MUST execute all git pushes
   - **Prohibited**: NO automatic commits or pushes

4. **Quality Assurance** (`/qa`)
   - Build with zero warnings (`make build`)
   - Memory safety validation (`valgrind`)
   - Static analysis (`cppcheck`)
   - Cross-platform compilation test

5. **Review** (`/review`)
   - Pre-landing code review
   - Constitution compliance check
   - Documentation completeness

6. **Deploy**
   - Merge to `main` via PR
   - **Manual approval required at all stages**

### Branch Strategy

- `main`: Production-ready code, protected
- `<###-feature-name>`: Feature branches (sequential numbering from speckit)
- All branches MUST have associated spec document
- **Manual Control**: User decides when to create branches and merge

### Commit Conventions

**DO NOT COMMIT AND PUSH**
**DO NOT COMMIT AND PUSH**
**DO NOT COMMIT AND PUSH**

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `build`, `ci`, `chore`

**Manual Commit Examples:**
```bash
# User manually reviews and commits
git add <files>
# git commit -m "docs: amend constitution to v2.0.0 (C engineering principles)"
# git push origin main
```

**Prohibited:**
```bash
# DO NOT automatically commit or push
# git commit -m "auto: ..."  # FORBIDDEN
# git push origin main       # FORBIDDEN without user approval
```

### Code Style Guidelines

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 100 characters
- **Brace Style**: K&R (opening brace on same line)
- **Naming**: `snake_case` for functions and variables, `UPPER_SNAKE_CASE` for
  macros and constants, `PascalCase` for struct/typedef names
- **Headers**: Include guards mandatory, alphabetize system includes before
  project includes
- **Functions**: One responsibility per function; max 50 lines (exceptions
  require justification)
- **Comments**: Explain WHY, not WHAT; Doxygen blocks for public APIs

## Governance

**Authority:**
This constitution supersedes all other development practices and guides. In case
of conflict with team conventions, constitution principles take precedence.

**Amendment Process:**
1. Propose amendment via GitHub issue with rationale
2. Architectural review for impact assessment
3. Team discussion and approval (consensus required)
4. Update constitution with version bump (MAJOR.MINOR.PATCH)
5. Propagate changes to all dependent templates and documentation
6. Announce changes to all contributors
7. **Manual Execution**: All constitutional amendments require manual user
   approval and commit

**Versioning Policy:**
- **MAJOR**: Backward incompatible principle removals or redefinitions
- **MINOR**: New principle/section added or materially expanded guidance
- **PATCH**: Clarifications, wording improvements, typo fixes

**Compliance Review:**
- All PRs MUST verify constitution compliance via `/review` command
- Memory safety violations of NON-NEGOTIABLE principles block merge
- Compiler warnings block merge (zero-warning policy)
- **Manual Review**: User MUST verify all compliance checks before merge

**Runtime Guidance:**
- Use `.specify/templates/` for workflow templates
- Use `docs/` for user-facing documentation
- Use `Makefile` as the single source of truth for build commands

**Enforcement:**
- CI checks for compilation, static analysis, memory safety
- Mandatory code review for all changes
- Quarterly constitution review and update cycle
- **Manual Control**: User has final approval on all changes to main branch

---

**Version**: 2.0.0 | **Ratified**: 2026-04-03 | **Last Amended**: 2026-04-26
