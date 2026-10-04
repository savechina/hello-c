# TODOS

## T-001: Expand regression tests across ~50 basic/ chapters

- **What:** Add Unity regression tests for the untested `src/basic/*_sample.c` chapters (memory, strings, structs, IO, etc.), mirroring the `test/basic/test_variables_sample.c` pattern with `test-asan` gate drivers.
- **Why:** AGENTS.md historically noted near-zero test coverage; this branch closed the gap for calc/raii/async_thread/variables only. The remaining basic/ chapters run under `make asan` but have no suite-level regression tests.
- **Pros:** Full behavioral coverage of the tutorial corpus; AI makes this 10–100× cheaper than human effort; catches demo regressions per chapter.
- **Cons:** Multi-commit campaign (~50 files + Makefile test growth); reopens the single-landing structure accepted in D3; needs per-chapter test-depth decisions.
- **Context:** Motivation: D4 offered this as option C (completeness 10/10) and the user chose B instead — rejected for this pass, backlog only. Current state: 6 test binaries, `test/` mirrors `src/` layout, TEST_LINK_OBJS links all objects except main.o (no Makefile edits needed per new test). Where to start in 3 months: pick highest-traffic chapters (memory_mgmt, strings, structs) first, reuse `test_variables_sample.c` as the template.
- **Depends on / blocked by:** none (standalone campaign; independent of the Section 1–4 fixes).
