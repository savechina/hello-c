# 数据库 (Database with SQLite3) 🟡

> "数据是程序的血液——而数据库是让血液有组织流动的心血管系统。"

## 开篇故事

想象你在一座巨大的图书馆里找一本书。如果你一本一本地翻，可能需要几个小时。但如果你告诉图书管理员："我要找评分 80 分以上且名字以 'A' 开头的书"，管理员会直接带你到正确的书架前。

SQLite 就像这样一位图书管理员。你不需要自己动手翻书——你只需要告诉它**你要什么**（SQL 查询），它负责**高效地找到**（数据库引擎）。预编译语句（Prepared Statement）就像是告诉管理员："我以后每次都查这个格式的数据"——管理员会提前优化搜索策略，不仅更快，还能防止坏人用假书名骗你。

## 本章适合谁

- 写过文件读写，但想尝试结构化数据存储的人
- 在 Python/Go 里用过 ORM，想知道底层 C API 怎么工作的人
- 对 SQL 注入攻击好奇，想知道"prepared statement 到底安全在哪里"的人
- 想了解数据库事务（Transaction）概念的人

## 你会学到什么

- SQLite3 完整工作流：`open` → `exec` → `query` → `close`
- 预编译语句（Prepared Statements）的工作原理和 SQL 注入防御
- CRUD 操作：CREATE、READ、UPDATE、DELETE 的 C 语言实现
- 事务控制：BEGIN / COMMIT / ROLLBACK 的用法
- 错误处理：`sqlite3_errmsg` 和返回值检查
- 资源清理：finalize 语句、close 数据库

## 前置要求

- 理解文件 I/O 基础（`fopen`/`fclose`、`fread`/`fwrite`）
- 了解 SQL 基本语法（`SELECT`、`INSERT`、`CREATE TABLE`）
- 掌握指针和错误码返回模式

## 第一个例子

```c
#include <stdio.h>
#include <sqlite3.h>

int main(void) {
    sqlite3 *db;
    /* 打开一个内存数据库 */
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open DB: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    /* 执行 SQL */
    char *errmsg = NULL;
    rc = sqlite3_exec(db, "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT);",
                      NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("Table created!\n");
    }

    /* 关闭数据库 */
    sqlite3_close(db);
    return 0;
}
```

四步走：**打开 → 执行 → 检查错误 → 关闭**。

## 原理解析

### 1. 内存数据库 vs 文件数据库

```c
sqlite3_open(":memory:", &db);    /* 内存数据库 — 程序退出后数据消失，适合测试 */
sqlite3_open("mydb.sqlite", &db); /* 文件数据库 — 数据持久化到磁盘 */
```

**我的理解**：内存数据库就像在桌上摊开笔记本，用完就收走。文件数据库就像把笔记归档到文件柜。内存数据库速度快（不经过磁盘 I/O），适合教程演示、单元测试和临时计算。

### 2. sqlite3_exec：一键执行

```c
int sqlite3_exec(sqlite3 *db,              /* 数据库句柄 */
                 const char *sql,           /* SQL 语句 */
                 int (*callback)(void*,int,char**,char**),  /* 回调函数 */
                 void *arg,                 /* 传给回调的参数 */
                 char **errmsg);            /* 错误信息 */
```

- 返回值：`SQLITE_OK` = 成功，非 0 = 错误
- 回调函数：每找到一行数据就调用一次。设为 `NULL` 表示不需要结果
- `errmsg`：错误信息由 SQLite 分配，必须用 `sqlite3_free()` 释放

```c
/* 用法 1: 不需要查询结果 (CREATE, INSERT, UPDATE, DELETE) */
sqlite3_exec(db, "INSERT INTO t VALUES (1, 'hello');", NULL, NULL, &errmsg);

/* 用法 2: 需要查询结果 */
sqlite3_exec(db, "SELECT * FROM t;", my_callback, NULL, &errmsg);
```

### 3. 回调函数详解

```c
int select_callback(void *arg, int ncols, char **values, char **headers) {
    for (int i = 0; i < ncols; i++) {
        printf("%s = %s | ", headers[i], values[i] ? values[i] : "NULL");
    }
    printf("\n");
    return 0;  /* 返回 0 = 继续，非 0 = 停止查询 */
}
```

- `ncols`: 列数
- `values`: 每列的值（字符串），`NULL` 表示 SQL 的 NULL
- `headers`: 每列的名称
- 返回 0 表示"我还要看下一行"，返回非 0 表示"够了，停下"

**局限**：`sqlite3_exec` 把所有值转成字符串，适合简单场景。需要类型安全时，应该用预编译语句。

### 4. 预编译语句 (Prepared Statements)

```c
sqlite3_stmt *stmt = NULL;

/* 第 1 步: 编译 SQL */
int rc = sqlite3_prepare_v2(db,
    "SELECT * FROM students WHERE score >= ?;",
    -1, &stmt, NULL);

/* 第 2 步: 绑定参数 (? → 80) */
sqlite3_bind_int(stmt, 1, 80);  /* 位置从 1 开始，不是 0! */

/* 第 3 步: 执行并取结果 */
while (sqlite3_step(stmt) == SQLITE_ROW) {
    int id    = sqlite3_column_int(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    int score = sqlite3_column_int(stmt, 2);
    printf("id=%d name=%s score=%d\n", id, name, score);
}

/* 第 4 步: 释放 */
sqlite3_finalize(stmt);
```

**为什么叫"预编译"？** SQLite 把 SQL 字符串编译成内部字节码，缓存起来。下次绑定不同参数重复执行时，跳过编译步骤，直接执行字节码。

### 5. 防 SQL 注入

```c
/* ❌ 危险: 字符串拼接 */
char sql[512];
snprintf(sql, sizeof(sql),
    "INSERT INTO students VALUES ('%s', %d);", user_input, score);
sqlite3_exec(db, sql, NULL, NULL, &errmsg);
/* 如果 user_input = "Robert'); DROP TABLE students;--" */
/* 实际执行的 SQL 变成: */
/* INSERT INTO students VALUES ('Robert'); DROP TABLE students;--', 0) */

/* ✅ 安全: 预编译语句 */
sqlite3_prepare_v2(db, "INSERT INTO students VALUES (?, ?);", -1, &stmt, NULL);
sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC);
sqlite3_bind_int(stmt, 2, score);
sqlite3_step(stmt);
sqlite3_finalize(stmt);
/* user_input = "Robert'); DROP TABLE students;--" */
/* 它只会当作一个字符串值存入 name 列——不会执行！ */
```

### 6. 事务 (Transaction)

```c
sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
/* 执行多条 INSERT... */
if (all_ok) {
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);  /* 确认保存 */
} else {
    sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL); /* 撤销所有更改 */
}
```

类比银行转账：从 A 扣 100 元、向 B 加 100 元——要么都成功，要么都失败。不能出现"A 扣了但 B 没收到"的中间状态。

## 常见错误

### ❌ 错误 1: 忘记释放 errmsg

```c
char *errmsg = NULL;
sqlite3_exec(db, "BAD SQL", NULL, NULL, &errmsg);
// errmsg 已被 SQLite 分配内存!
// 忘记 sqlite3_free(errmsg) → 内存泄漏
```

✅ **修复**：永远在收到 errmsg 后 `sqlite3_free(errmsg)`。

### ❌ 错误 2: 绑定时位置从 0 开始

```c
// ❌ 位置从 1 开始！
sqlite3_bind_int(stmt, 0, 80);  /* 错误! 应该是 1 */

// ✅ 正确
sqlite3_bind_int(stmt, 1, 80);  /* 第一个 ? 的位置 = 1 */
```

### ❌ 错误 3: 忘记 finalize 语句

```c
sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
sqlite3_step(stmt);
// 没有 sqlite3_finalize(stmt) → 内存泄漏 + sqlite3_close 可能失败
```

✅ **修复**：每个 `prepare` 必须配对 `finalize`。

### ❌ 错误 4: 用 printf 拼接 SQL

```c
// ❌ 字符串拼接 = SQL 注入
char sql[256];
sprintf(sql, "INSERT INTO t VALUES ('%s');", user_input);

// ✅ 预编译 + bind
sqlite3_prepare_v2(db, "INSERT INTO t VALUES (?);", -1, &stmt, NULL);
sqlite3_bind_text(stmt, 1, user_input, -1, SQLITE_STATIC);
```

## 动手练习

### 🟢 练习 1: 创建和查询

创建一个学生表（id, name, grade），插入 3 条记录，用 `sqlite3_exec` 回调打印全部结果。

<details>
<summary>点击查看答案</summary>

```c
sqlite3 *db;
sqlite3_open(":memory:", &db);

sqlite3_exec(db, "CREATE TABLE students (id INTEGER PRIMARY KEY, name TEXT, grade INTEGER);",
             NULL, NULL, NULL);
sqlite3_exec(db, "INSERT INTO students (name, grade) VALUES ('Alice', 95);", NULL, NULL, NULL);
sqlite3_exec(db, "INSERT INTO students (name, grade) VALUES ('Bob', 82);", NULL, NULL, NULL);
sqlite3_exec(db, "SELECT id, name, grade FROM students;", select_callback, NULL, NULL);

sqlite3_close(db);
```
</details>

### 🟡 练习 2: 预编译插入

用预编译语句批量插入 10 条记录（名字: S01-S10, 分数: 随机 60-100），然后查询分数 ≥ 80 的学生。

<details>
<summary>点击查看答案</summary>

```c
sqlite3_stmt *stmt;
sqlite3_prepare_v2(db,
    "INSERT INTO students (name, score) VALUES (?, ?);", -1, &stmt, NULL);

for (int i = 1; i <= 10; i++) {
    char name[8];
    snprintf(name, sizeof(name), "S%02d", i);
    int score = 60 + (rand() % 41);  /* 60-100 */
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, score);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);  /* 重置，准备下一次绑定 */
}
sqlite3_finalize(stmt);
```
</details>

### 🔴 练习 3: 事务 + 错误回滚

在一个事务中插入 5 条记录，第 3 条失败时执行 ROLLBACK，验证数据被完整撤销。

<details>
<summary>点击查看答案</summary>

```c
sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

int ok = 1;
for (int i = 1; i <= 5; i++) {
    if (i == 3) {  /* 第 3 条故意失败 */
        char *errmsg;
        sqlite3_exec(db, "INSERT INTO nonexistent VALUES (1);", NULL, NULL, &errmsg);
        sqlite3_free(errmsg);
        ok = 0;
        break;
    }
    char sql[128];
    snprintf(sql, sizeof(sql), "INSERT INTO students (name) VALUES ('S%02d');", i);
    sqlite3_exec(db, sql, NULL, NULL, NULL);
}

if (ok) {
    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
} else {
    sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
    printf("回滚成功，无数据写入\n");
}
```
</details>

## 故障排查

### Q: sqlite3.h 找不到

```
fatal error: 'sqlite3.h' file not found
```

**原因**：系统没有安装 SQLite3 开发包。
**解决**：
```bash
macOS:  brew install sqlite3
Ubuntu: sudo apt install libsqlite3-dev
```
然后更新 Makefile 的 `LDFLAGS` 加 `-lsqlite3`。

### Q: sqlite3_close 返回 SQLITE_BUSY

```c
int rc = sqlite3_close(db);  // 返回 1 (SQLITE_BUSY)
```

**原因**：还有未关闭的预编译语句（`stmt`）。
**解决**：确保所有 `sqlite3_prepare_v2` 都配对 `sqlite3_finalize`。

### Q: prepared statement 的 reset 和 finalize 区别？

- **`sqlite3_reset(stmt)`**: 重置语句到初始状态，可以重新绑定参数再次执行。语句仍然存在。
- **`sqlite3_finalize(stmt)`**: 彻底销毁语句，释放内存。不能再使用。

**规则**：循环中 `reset`，用完 `finalize`。

## 知识扩展

### 1. `SQLITE_STATIC` vs `SQLITE_TRANSIENT`

```c
sqlite3_bind_text(stmt, 1, "hello", -1, SQLITE_STATIC);   /* SQLite 不会拷贝字符串 */
sqlite3_bind_text(stmt, 1, dynamic_buf, -1, SQLITE_TRANSIENT);  /* SQLite 会拷贝一份 */
```

**原则**：字符串在你控制的内存中（如局部变量）用 `STATIC`；可能被修改或释放的用 `TRANSIENT`。

### 2. SQLite 的其他 API

- `sqlite3_get_table`: 一次性获取结果到二维数组（简单查询方便，大数据集会占内存）
- `sqlite3_changes(db)`: 返回上一次 INSERT/UPDATE/DELETE 影响的行数
- `sqlite3_last_insert_rowid(db)`: 返回最后一次 INSERT 的自增 ID

### 3. 生产级建议

- 使用 WAL（Write-Ahead Logging）模式提升并发读性能
- 大事务用 `BEGIN IMMEDIATE` 避免写冲突
- 考虑用 ORM 层（如 SQLiteCpp）减少手写 SQL 的错误

## 小结

- **四步走**：`open` → `exec/prepare` → `finalize` → `close`
- **预编译语句**是防 SQL 注入的核心机制——永远不要字符串拼接用户输入
- **errormsg** 必须 `sqlite3_free`，stmt 必须 `finalize`
- **事务**确保多条操作的原子性——要么全成功，要么全失败

> **我的教训是**：第一次写 SQLite C API 时，我忘记 `finalize` 和 `free`，导致内存泄漏。记住：SQLite 分配的内存（errmsg、stmt）和系统分配的内存一样，需要你来管理。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 内存数据库（In-Memory Database） | 数据存储在内存中，程序退出后消失 |
| 预编译语句（Prepared Statement） | 将 SQL 编译为字节码，支持参数绑定 |
| SQL 注入（SQL Injection） | 通过拼接恶意输入篡改 SQL 语句的攻击 |
| 绑定参数（Bind Parameter） | 用 `?` 占位符 + `sqlite3_bind_*` 传递值 |
| 事务（Transaction） | 一组原子操作，要么全部成功，要么全部回滚 |
| 回调函数（Callback Function） | 每找到一行数据时被调用的函数 |
| 资源释放（Finalize/Close） | 清理 SQLite 分配的内存和句柄 |
| 错误信息（Error Message） | `sqlite3_errmsg()` 返回的人类可读错误 |
| 自增主键（Auto-Increment） | `AUTOINCREMENT` 列自动分配递增值 |
| 写入日志（WAL） | Write-Ahead Logging，提升并发性能的模式 |

## 延伸阅读

- [SQLite 官方文档](https://www.sqlite.org/docs.html) — C API 参考手册
- [SQLite3 C API 教程](https://www.sqlite.org/c3ref/funclist.html) — 按函数分类的接口
- [SQL 注入攻防](https://cheatsheetseries.owasp.org/cheatsheets/SQL_Injection_Prevention_Cheat_Sheet.html) — OWASP 安全指南
- [Beej's Guide to SQLite](https://beej.us/guide/bgsql/) — 简明 SQLite 入门

## 继续学习

你已经学会了 SQLite3 C API 的核心工作流。这是 C 程序中最常用的嵌入式数据库——几乎所有嵌入式设备和桌面软件都用它（甚至浏览器、手机 App 的底层）。

在下一章节中，我们将探索操作系统级别的能力：**系统调用**——直接与操作系统对话。

> 💡 **提示**：打开 SQLite3 数据库时，你是否注意到它和文件 I/O 很像？`open → read/write → close` 的模式贯穿整个 C 标准库和 POSIX API。

[← 上一章：工具链](./tools.md) | [下一章 → 系统调用](./system.md)
