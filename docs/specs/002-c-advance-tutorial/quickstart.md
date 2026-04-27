# Quickstart: C Advance Tutorial Development

**Feature**: 002-c-advance-tutorial

## Prerequisites

- GCC 12+ or Clang 15+
- POSIX pthreads (macOS/Linux)
- `make`, `mdbook`, `valgrind`, `cppcheck`

## Build Advance Tutorial

```bash
# Build everything (basic + advance)
make build

# Run advance section only
make advance

# Run single advance chapter
make sample CHAPTER=error_handling
```

## Add New Advance Chapter

1. Create mdBook doc: `docs/src/advance/<topic>.md`
2. Create source: `src/advance/<topic>_sample.{c,h}`
3. Add `main_<topic>_sample()` to `advance.h`
4. Add call to `advance.c`
5. Update `SUMMARY.md`: add to Advance section
6. Verify: `make build` and `make sample CHAPTER=<topic>`

## Quality Checklist

- [ ] `make build` zero warnings
- [ ] `valgrind --leak-check=full` zero leaks
- [ ] `cppcheck --enable=all src/advance/` passes
- [ ] mdBook has all 15 mandatory sections
- [ ] First-person voice, real-world analogy opening
- [ ] Bilingual Chinese + English terms
