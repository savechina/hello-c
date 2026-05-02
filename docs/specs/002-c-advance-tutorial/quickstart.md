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

## Add Test with Unity & CMock

1. Download Unity v2.6.1 to `test/vendor/unity/` (3 files: unity.c, unity.h, unity_internals.h)
2. Download CMock v2.6.0 to `test/vendor/cmock/` (Ruby-based mock generator)
3. Create `src/advance/calc.c` + `calc.h` with `calc_add()`, `calc_multiply()`, `calc_is_valid()`
4. Create `test/advance/test_calc_add.c` with Unity test cases (mirrors `src/advance/` layout)
5. Update Makefile: add `test/` directory with wildcard `**/*.c` matching, `make test` target
6. Verify: `make test` runs all tests with colored output

```bash
# Download Unity
mkdir -p test/vendor/unity
curl -L -o unity.zip https://github.com/ThrowTheSwitch/Unity/archive/refs/tags/v2.6.1.zip
unzip -j unity.zip "Unity-2.6.1/src/*" -d test/vendor/unity/
rm unity.zip

# Download CMock
mkdir -p test/vendor/cmock
curl -L -o cmock.zip https://github.com/ThrowTheSwitch/CMock/archive/refs/tags/v2.6.0.zip
unzip -j cmock.zip "CMock-2.6.0/lib/*" -d test/vendor/cmock/
rm cmock.zip

# Run tests
make test
```

## Quality Checklist

- [ ] `make build` zero warnings (note: pre-existing `operators_sample.c` error unrelated)
- [ ] `valgrind --leak-check=full` zero leaks
- [ ] `cppcheck --enable=all src/advance/` passes
- [ ] mdBook has all 15 mandatory sections
- [ ] First-person voice, real-world analogy opening
- [ ] Bilingual Chinese + English terms
- [x] `make test` passes with zero failures (2 tests for `calc_add()`)
- [ ] Unity tests cover `calc_add()`, `calc_multiply()`, `calc_is_valid()`
- [ ] CMock v2.6.0 available in `test/vendor/cmock/`
