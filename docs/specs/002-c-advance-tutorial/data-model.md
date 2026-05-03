# Data Model: C Advance Tutorial Subcommands

## Entities & Relationships

### Entity: CLI Subcommand

| Attribute | Type | Description |
|-----------|------|-------------|
| name | string | Subcommand name: `basic`, `advance`, `list`, `all`, `help` |
| handler | function pointer | `void (*handler)(void)` — points to coordinator function |
| description_cn | string | Chinese description for `list` output |
| description_en | string | English technical description (bilingual) |

### Entity: Topic Chapter

| Attribute | Type | Description |
|-----------|------|-------------|
| name | string | Topic directory name: `advance/error_handling_sample` |
| module | string | Parent module: `advance`, `basic`, `module1`, `module2` |
| entry_point | string | `main_<module>_sample(void)` function name |
| has_subcommands | bool | Whether parent module has subcommand support |

## Relationships

```
[main.c] --dispatches--> [CLI Subcommand]
[CLI Subcommand] --calls--> [Coordinator Function]
[Coordinator Function] --calls--> [Topic Chapter Entry Points]
[Topic Chapter] --exists in--> [src/ directories]
```

## State Transitions

**CLI Dispatch Flow**:
```
[Start] → Parse argv[1]
  ├── "basic" → main_basic_sample() → all basic _sample() functions
  ├── "advance" → main_advance_sample() → all advance _sample() functions
  ├── "list" → print available topics (basic + advance + modules)
  ├── "all" / no args → main_hello() → ALL chapters (current behavior)
  └── "help" / unknown → print usage + exit
```

## Data Volume / Scale

- **Subcommands**: 4 (basic, advance, list, all/help)
- **Basic chapters**: ~38 files (after restructure) in `src/basic/`
- **Advance chapters**: 12 files in `src/advance/`
- **Modules**: 2 example modules (`module1/`, `module2/`)

## Validation Rules

- All subcommand handlers MUST be `void func(void)` signature
- `list` output MUST be bilingual (Chinese + English terms)
- Unknown subcommand MUST print usage and return 1
- `argv[1]` NULL check: treat as "all" (backward compatible)
