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

## T-004: Mechanical enforcement of the naming policy (symbol-surface gate)

- **What:** Add `-Wmissing-prototypes -Wmissing-declarations` to the Makefile CFLAGS line plus a scripted `nm` allowlist check that asserts the exported (global) symbol surface of `build/bin/hello` contains only `main*` entries and the documented cross-TU test APIs (`rs_*`, `fp_*`, `vp_*`, `pf_*`, `calc_*`); run both in the existing CI workflow as a naming-policy gate.
- **Why:** The NAMING POLICY (`src/AGENTS.md`, confirmed 2026-10-31) is currently enforced only by review discipline — as codex outside-voice Q1 flagged, `-Werror` build + Unity suites + asan/tsan cannot catch a new accidental global helper. The 2026-10-31 diff spent 262 renames + 1 public-symbol cleanup reaching policy compliance; without a mechanical gate the next chapter can silently regress it. Evidence: `~/.gstack/projects/savechina-hello-c/main-eng-review-20261031-182712.md`.
- **Pros:** Converts a docs-only policy into a build-time failure; makes "one public symbol per file" auditable per commit; compiler flags are free at run time; nm script is ~20 lines of shell.
- **Cons:** First run will flag teaching-code quirks (deliberate unsafe demos, `logging_sample.c` counter-examples) needing an explicit allowlist; maintainer must keep the allowlist in sync when adding chapters; flags slightly increase CI compile time.
- **Context:** Motivation: eng-review 2026-10-31 O7/outside-voice Q1 (codex/deepseek-flash, 155k-token pass). Current state: gates = make build/test/asan/tsan/analyze; no symbol-surface check. Where to start in 3 months: add the two CFLAGS first, run locally, bucket findings into policy violations vs documented exceptions, then land the nm allowlist script as `ci/nm_allowlist.sh` and wire it into `.github/workflows/test.yml`.
- **Depends on / blocked by:** none hard; sequenced after T-005 (macOS runner) so the gate covers both Apple and Ubuntu branches from day one.

## T-005: macOS runner job in CI (verify the Apple-only code branches)

- **What:** Add a `runs-on: macos-latest` job to `.github/workflows/test.yml` (currently Ubuntu-only at :27/:58) running `make build && make test && make test-asan`; the job exists to exercise code paths that cannot run on Linux — the `__has_include(<uchar.h>)` fallback typedef in `src/basic/datatype_sample.c:16-29`, Apple long-double ABI text, clang-specific diagnostic pragmas.
- **Why:** CI covers exactly 1 of the 4 platforms the README markets. The 2026-10-31 datatype diff happens to expand precisely an Apple-only branch (hand-typedef char16_t/char32_t fallback), which CI never compiles; eng-review 2026-10-31 D6=A kept that typede; a future edit can break Apple builds silently. This complements the design doc's `/office-hours` 2026-10-24 Approach-B full matrix (freebsd/omnios VM jobs) — T-005 is the cheap middle step that covers the maintainer's own platform first.
- **Pros:** The Apple-only fallback gets compiled under CI on every push; closes the biggest platform blind spot with one YAML stanza; macOS runners are already used by the repo's docs workflow so credentials/quotas exist.
- **Cons:** macOS runner minutes cost ~10× Linux; failures on Apple clang versions contributors don't have locally can stall PRs; adds one more job to maintain in the workflow.
- **Context:** Motivation: eng-review 2026-10-31 O8/outside-voice Q3. Current state: `test.yml` Ubuntu-only. Where to start in 3 months: copy the existing Linux job block, switch `runs-on`, drop `-D__PLATFORM__LINUX` for `__PLATFORM__MAC` definition, then run once manually via workflow_dispatch to calibrate the asan gate on Darwin (`detect_leaks=0` already handled by the Makefile).
- **Depends on / blocked by:** none hard; should land before T-004 so the symbol-surface gate is calibrated on both platforms.
