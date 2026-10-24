# TODOS

## T-001: Expand regression tests across ~50 basic/ chapters

- **What:** Add Unity regression tests for the untested `src/basic/*_sample.c` chapters (memory, strings, structs, IO, etc.), mirroring the `test/basic/test_variables_sample.c` pattern with `test-asan` gate drivers.
- **Why:** AGENTS.md historically noted near-zero test coverage; this branch closed the gap for calc/raii/async_thread/variables only. The remaining basic/ chapters run under `make asan` but have no suite-level regression tests.
- **Pros:** Full behavioral coverage of the tutorial corpus; AI makes this 10–100× cheaper than human effort; catches demo regressions per chapter.
- **Cons:** Multi-commit campaign (~50 files + Makefile test growth); reopens the single-landing structure accepted in D3; needs per-chapter test-depth decisions.
- **Context:** Motivation: D4 offered this as option C (completeness 10/10) and the user chose B instead — rejected for this pass, backlog only. Current state: 6 test binaries, `test/` mirrors `src/` layout, TEST_LINK_OBJS links all objects except main.o (no Makefile edits needed per new test). Where to start in 3 months: pick highest-traffic chapters (memory_mgmt, strings, structs) first, reuse `test_variables_sample.c` as the template.
- **Depends on / blocked by:** none (standalone campaign; independent of the Section 1–4 fixes).

## T-002: Expand `make analyze` (-fanalyzer) beyond `src/hello.c`

- **What:** Change the `analyze` target (`Makefile:318-329`) to run `-fanalyzer` over each file in `$(SOURCES)` instead of the single hardcoded `src/hello.c`, keeping the existing "compiler lacks -fanalyzer → skip" probe.
- **Why:** AGENTS.md presents `make analyze` as this project's leak / double-free / uninitialized-value gate, but the recipe compiles exactly one file: `-I$(INCLUDE_DIR) $(addprefix -I,$(INC_DIRS)) -c src/hello.c -o /dev/null`. That is 1 of 76 source files, while the manual memory management this tutorial teaches lives in the 24 `src/advance/` chapters (`raii_sample.c`, `smart_pointers_sample.c`, `iterators_sample.c`, `memory_mgmt_sample.c`). On macOS the target skips entirely because Apple clang has no `-fanalyzer`, so the documented gate never runs on the maintainer's own machine.
- **Pros:** The leak and uninitialized-value checks finally reach the chapters where those bugs actually occur. The target already probes and skips cleanly, so widening the file list is a small loop change with no new dependencies. The CI `analyze` step in `.github/workflows/test.yml` starts checking real code immediately.
- **Cons:** `-fanalyzer` is slow and noisy across 76 files, so the first run will surface a batch of pre-existing findings that need triage one by one. Once in CI it adds minutes per push, and until triage is done it keeps the job red. Some findings will be intentional tutorial code (for example the UAF demos in `scope_sample.c`) and need a suppression list rather than a code change.
- **Context:** Motivation: the 2026-10-10 engineering review of `e8442ce..65a9bce` found the gate's real coverage contradicts its documentation, but deliberately did not fix it inside a test/docs change. Treat it as its own triage campaign. Where to start in 3 months: run it once with `make analyze CC=gcc-14` locally, bucket the findings into real bugs vs intentional demos, add suppressions for the latter, then widen the file list.
- **Depends on / blocked by:** No hard prerequisite. Sequence it after `.github/workflows/test.yml` lands, otherwise CI goes red on findings nobody has triaged yet.

## T-003: extern "C" linkage + recursion-vs-iteration comparison

- **What:** Two deliberately deferred teaching gaps: (a) an `extern "C"` / `#ifdef __cplusplus` linkage-guard chapter — add it only when a C++ consumer or a header-sharing sibling (hello-python/ruby/go) actually requires it; (b) a recursion-vs-iteration counterpart demo — iterative factorial/fibonacci, a tail-recursion note (why C compilers rarely guarantee TCO), and a stack-cost comparison.
- **Why:** The 2026-10-24 chapter inventory (F9) found `extern "C"` / `__cplusplus` absent from all of `src/`, deliberately skipped by D4 as niche for a pure-C tutorial; the recursion topic remains ~30% covered — recursion is taught, but there is no iterative counterpart, no tail-recursion discussion, and no cost comparison.
- **Pros:** The linkage guard is ~10 lines and unblocks real C/C++ interop the day a sibling project needs it; the iterative counterparts make the recursion chapter comparison-complete and teach the stack-overflow trade-off with runnable numbers.
- **Cons:** `extern "C"` has no pure-C payoff until a consumer exists (dead weight in a beginner corpus); the recursion extension reopens a basic/ chapter and needs stack-depth demos that behave differently across compilers/platforms.
- **Context:** Chapter inventory evidence at `~/.gstack/projects/savechina-hello-c/main-eng-review-20261024-103506.md` (F9: absent #10 + partial "recursion-vs-iteration"). Where to start in 3 months: check whether hello-python/ruby/go or any C++ consumer has appeared; if yes, land (a) as a headers-chapter fold, and land (b) independently in `docs/src/basic/recursion.md` + `src/basic/recursion_sample.c`.
- **Depends on / blocked by:** none (standalone backlog; independent of the Section 1–4 fixes and the T5–T10 chapter round).
