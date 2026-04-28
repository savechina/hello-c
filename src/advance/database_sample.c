/**
 * @file database_sample.c
 * @brief 数据库 (Database with SQLite3) — Advance tutorial chapter
 *
 * Demonstrates SQLite3 C API:
 *   1. In-memory database (sqlite3_open ":memory:")
 *   2. CREATE TABLE
 *   3. INSERT raw SQL
 *   4. SELECT and iterate results
 *   5. Prepared statements (sqlite3_prepare_v2)
 *   6. Parameter binding (sqlite3_bind_*)
 *   7. Column extraction (sqlite3_column_*)
 *   8. UPDATE & DELETE
 *   9. Error handling (sqlite3_errmsg)
 *  10. Transaction control
 *  11. Resource cleanup
 *
 * Uses in-memory SQLite so no temp files needed.
 * Falls back gracefully if sqlite3.h is not available.
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "advance/database_sample.h"

#if __has_include(<sqlite3.h>)

#include <sqlite3.h>

/* ====================================================================
 *  Global database handle for demo convenience
 * ==================================================================== */

static sqlite3 *g_db = NULL;

/* ====================================================================
 *  Section 1: Open & Close (内存数据库)
 * ==================================================================== */

/**
 * 打开一个内存数据库
 * 类比: 图书馆管理员已经帮你把书架布置好，你随时可以查询
 */
static void database_open_close_sample(void)
{
    printf("  [1] 打开内存数据库:\n");

    int rc = sqlite3_open(":memory:", &g_db);
    if (rc != SQLITE_OK) {
        printf("    [Error] sqlite3_open: %s\n", sqlite3_errmsg(g_db));
        sqlite3_close(g_db);
        g_db = NULL;
        return;
    }

    printf("    内存数据库已打开 (sqlite3_open \":memory:\")\n");
    printf("    数据库句柄: %p\n", (void *)g_db);
}

/* ====================================================================
 *  Section 2: CREATE TABLE
 * ==================================================================== */

/**
 * 创建表 — DDL (Data Definition Language)
 * 类比: 定义书架的格式——每本书有哪些属性可记录
 */
static void database_create_table_sample(void)
{
    printf("\n  [2] 创建表 (CREATE TABLE):\n");

    const char *sql =
        "CREATE TABLE IF NOT EXISTS students ("
        "  id    INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name  TEXT    NOT NULL,"
        "  score INTEGER DEFAULT 0"
        ");";

    char *errmsg = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("    [Error] CREATE TABLE: %s\n", errmsg);
        sqlite3_free(errmsg);
        return;
    }

    printf("    CREATE TABLE students (id, name, score) — 成功\n");
}

/* ====================================================================
 *  Section 3: INSERT raw SQL
 * ==================================================================== */

/**
 * 用原始 SQL 插入数据
 * 类比: 直接把新书放到书架上
 */
static void database_insert_raw_sample(void)
{
    printf("\n  [3] 插入数据 (INSERT raw SQL):\n");

    const char *inserts[] = {
        "INSERT INTO students (name, score) VALUES ('Alice', 95);",
        "INSERT INTO students (name, score) VALUES ('Bob', 82);",
        "INSERT INTO students (name, score) VALUES ('Charlie', 70);",
        NULL
    };

    for (int i = 0; inserts[i] != NULL; i++) {
        char *errmsg = NULL;
        int rc = sqlite3_exec(g_db, inserts[i], NULL, NULL, &errmsg);
        if (rc != SQLITE_OK) {
            printf("    [Error] INSERT #%d: %s\n", i + 1, errmsg);
            sqlite3_free(errmsg);
        } else {
            printf("    INSERT #%d — 成功\n", i + 1);
        }
    }
}

/* ====================================================================
 *  Section 4: SELECT (sqlite3_exec callback)
 * ==================================================================== */

/**
 * sqlite3_exec 的回调函数
 * 每找到一行数据就调用一次
 */
static int select_callback(void *arg, int ncols, char **values, char **headers)
{
    (void)arg;
    (void)headers;

    for (int i = 0; i < ncols; i++) {
        const char *val = (values[i] != NULL) ? values[i] : "NULL";
        if (i > 0) printf(", ");
        printf("%s", val);
    }
    printf("\n");
    return 0;  /* 返回 0 = 继续，非 0 = 停止 */
}

/**
 * 查询所有记录 — sqlite3_exec + 回调
 */
static void database_select_all_sample(void)
{
    printf("\n  [4] 查询全部 (SELECT *):\n");

    int rc = sqlite3_exec(g_db, "SELECT id, name, score FROM students;",
                          select_callback, NULL, NULL);
    if (rc != SQLITE_OK) {
        printf("    [Error] SELECT: %s\n", sqlite3_errmsg(g_db));
    }
}

/* ====================================================================
 *  Section 5: Prepared Statements (sqlite3_prepare_v2)
 * ==================================================================== */

/**
 * 预编译语句 — 安全、可复用
 * 类比: 提前告诉图书馆"我要一个按分数排序的书架"，而不是每次手动翻
 *
 * 关键 API:
 *   - sqlite3_prepare_v2 — 将 SQL 文本编译为字节码
 *   - sqlite3_bind_text / sqlite3_bind_int — 绑定参数值
 *   - sqlite3_step — 逐行执行
 *   - sqlite3_column_text / sqlite3_column_int — 提取列值
 *   - sqlite3_finalize — 释放语句对象
 */
static void database_prepared_statement_sample(void)
{
    printf("\n  [5] 预编译语句 (Prepared Statement):\n");

    const char *sql = "SELECT id, name, score FROM students WHERE score >= ?;";
    sqlite3_stmt *stmt = NULL;

    /* 编译 SQL */
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("    [Error] prepare: %s\n", sqlite3_errmsg(g_db));
        return;
    }

    /* 绑定参数: ? = 80 (查找分数 >= 80 的学生) */
    sqlite3_bind_int(stmt, 1, 80);
    printf("    绑定参数: score >= 80\n");
    printf("    结果:\n");

    /* 逐行读取 */
    int row = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id    = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        int score = sqlite3_column_int(stmt, 2);

        printf("      row %d: id=%d, name=%s, score=%d\n", ++row, id, name, score);
    }

    if (rc != SQLITE_DONE) {
        printf("    [Error] step: %s\n", sqlite3_errmsg(g_db));
    }

    /* 释放语句 */
    sqlite3_finalize(stmt);
    printf("    预编译语句已释放 (finalize)\n");
}

/* ====================================================================
 *  Section 6: INSERT via prepared statement (安全插入)
 * ==================================================================== */

/**
 * 用预编译语句插入 — 防止 SQL 注入
 * 类比: 填写表格而不是口述指令——格式固定，不会出错
 */
static void database_prepared_insert_sample(void)
{
    printf("\n  [6] 预编译插入 (Prepared INSERT):\n");

    const char *sql = "INSERT INTO students (name, score) VALUES (?, ?);";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("    [Error] prepare: %s\n", sqlite3_errmsg(g_db));
        return;
    }

    /* 插入: David, 88 */
    sqlite3_bind_text(stmt, 1, "David", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, 88);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        printf("    插入 David (88) — 成功\n");
    } else {
        printf("    [Error] INSERT step: %s\n", sqlite3_errmsg(g_db));
    }

    sqlite3_finalize(stmt);

    /* 插入: 坏名字 "Robert'); DROP TABLE students;--" */
    rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("    [Error] prepare: %s\n", sqlite3_errmsg(g_db));
        return;
    }

    sqlite3_bind_text(stmt, 1, "Robert'); DROP TABLE students;--", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, 0);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        printf("    SQL 注入尝试被安全阻止（名字被当成字符串处理）\n");
    }

    sqlite3_finalize(stmt);
}

/* ====================================================================
 *  Section 7: UPDATE & DELETE
 * ==================================================================== */

/**
 * UPDATE — 更新已有记录
 */
static void database_update_sample(void)
{
    printf("\n  [7] 更新数据 (UPDATE):\n");

    const char *sql = "UPDATE students SET score = 99 WHERE name = 'Alice';";
    char *errmsg = NULL;

    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("    [Error] UPDATE: %s\n", errmsg);
        sqlite3_free(errmsg);
        return;
    }

    printf("    UPDATE Alice score → 99 — 成功\n");
    printf("    受影响的行数: %d\n", sqlite3_changes(g_db));
}

/**
 * DELETE — 删除记录
 */
static void database_delete_sample(void)
{
    printf("\n  [8] 删除数据 (DELETE):\n");

    /* 先查看删除前的状态 */
    printf("    删除前的数据:\n");
    sqlite3_exec(g_db, "SELECT id, name, score FROM students;",
                 select_callback, NULL, NULL);

    const char *sql = "DELETE FROM students WHERE name = 'Robert'); DROP TABLE students;--';";
    char *errmsg = NULL;

    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("    [Note] DELETE 了之前 SQL 注入的脏数据: %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("    DELETE — 成功\n");
    }

    printf("    删除后的数据:\n");
    sqlite3_exec(g_db, "SELECT id, name, score FROM students;",
                 select_callback, NULL, NULL);
}

/* ====================================================================
 *  Section 8: 错误处理 (sqlite3_errmsg)
 * ==================================================================== */

/**
 * 故意制造错误，展示如何获取错误信息
 */
static void database_error_handling_sample(void)
{
    printf("\n  [9] 错误处理 (Error Handling):\n");

    /* 错误 1: 查询不存在的表 */
    char *errmsg = NULL;
    int rc = sqlite3_exec(g_db, "SELECT * FROM nonexistent;", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("    错误 (不存在的表): %s\n", errmsg);
        sqlite3_free(errmsg);
    }

    errmsg = NULL;
    /* 错误 2: 违反约束 (name NOT NULL) */
    rc = sqlite3_exec(g_db,
        "INSERT INTO students (name, score) VALUES (NULL, 100);",
        NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("    错误 (NOT NULL 违反): %s\n", errmsg);
        sqlite3_free(errmsg);
    }

    /* 错误 3: 用 sqlite3_errmsg 直接获取 */
    errmsg = NULL;
    rc = sqlite3_exec(g_db, "BLEEP BLOOP;", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("    sqlite3_errmsg: %s\n", sqlite3_errmsg(g_db));
        sqlite3_free(errmsg);
    }
}

/* ====================================================================
 *  Section 9: Transaction (事务)
 * ==================================================================== */

/**
 * 事务控制 — 确保数据一致性
 * 类比: 银行转账——扣款和存款要么一起成功，要么一起失败
 */
static void database_transaction_sample(void)
{
    printf("\n  [10] 事务控制 (Transaction):\n");

    /* 开始事务 */
    char *errmsg = NULL;
    int rc = sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("    [Error] BEGIN: %s\n", errmsg);
        sqlite3_free(errmsg);
        return;
    }

    /* 在同一事务中插入两条记录 */
    rc = sqlite3_exec(g_db,
        "INSERT INTO students (name, score) VALUES ('Eve', 77);",
        NULL, NULL, NULL);

    if (rc == SQLITE_OK) {
        rc = sqlite3_exec(g_db,
            "INSERT INTO students (name, score) VALUES ('Frank', 65);",
            NULL, NULL, NULL);
    }

    if (rc == SQLITE_OK) {
        /* 全部成功 — 提交 */
        rc = sqlite3_exec(g_db, "COMMIT;", NULL, NULL, &errmsg);
        if (rc == SQLITE_OK) {
            printf("    事务提交成功 (COMMIT) — Eve(77), Frank(65)\n");
        }
    } else {
        /* 有错误 — 回滚 */
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        printf("    事务回滚 (ROLLBACK)\n");
    }
    if (errmsg != NULL) {
        sqlite3_free(errmsg);
    }
}

/* ====================================================================
 *  Section 10: Column extraction (列值提取)
 * ==================================================================== */

/**
 * 细粒度列提取 — sqlite3_column_* 系列
 */
static void database_column_extraction_sample(void)
{
    printf("\n  [11] 列值提取 (Column Extraction):\n");

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db,
        "SELECT id, name, score FROM students;", -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        printf("    [Error] prepare: %s\n", sqlite3_errmsg(g_db));
        return;
    }

    /* 打印列名 */
    int ncols = sqlite3_column_count(stmt);
    printf("    列数: %d\n", ncols);
    for (int i = 0; i < ncols; i++) {
        printf("    列 %d 名称: %s\n", i, sqlite3_column_name(stmt, i));
    }

    /* 逐行提取 */
    printf("\n    明细:\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        /* 每种类型都试一下 */
        int64_t rowid = sqlite3_column_int64(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        int score = sqlite3_column_int(stmt, 2);

        /* 类型检测 */
        int type = sqlite3_column_type(stmt, 2);
        const char *type_str = "UNKNOWN";
        switch (type) {
            case SQLITE_INTEGER: type_str = "INTEGER"; break;
            case SQLITE_TEXT:    type_str = "TEXT";    break;
            case SQLITE_FLOAT:   type_str = "FLOAT";   break;
            case SQLITE_NULL:    type_str = "NULL";    break;
            default: break;
        }

        printf("      id=%" PRId64 ", name=%s, score=%d (type=%s)\n",
               rowid, name, score, type_str);
    }

    sqlite3_finalize(stmt);
}

/* ====================================================================
 *  Section 11: 资源清理 (Resource Cleanup)
 * ==================================================================== */

/**
 * 关闭数据库 — 必须调用
 * 类比: 图书馆打烊，关灯锁门
 */
static void database_cleanup_sample(void)
{
    printf("\n  [12] 资源清理 (Cleanup):\n");

    if (g_db != NULL) {
        int rc = sqlite3_close(g_db);
        if (rc == SQLITE_OK) {
            printf("    数据库已关闭 (sqlite3_close)\n");
        } else {
            printf("    [Note] sqlite3_close 返回 %d — 可能有未释放的语句\n", rc);
        }
        g_db = NULL;
    }
}

/* ====================================================================
 *  Coordinator entry
 * ==================================================================== */

#else /* __has_include(<sqlite3.h>) */

/*  sqlite3.h 不存在时的 fallback */

#endif /* __has_include(<sqlite3.h>) */

int main_database_sample(void)
{
    printf("========================================\n");
    printf("  数据库 (Database with SQLite3)\n");
    printf("========================================\n\n");

#if __has_include(<sqlite3.h>)

    /* Phase 1: Open & Schema */
    database_open_close_sample();
    database_create_table_sample();

    /* Phase 2: Insert & Query */
    database_insert_raw_sample();
    database_select_all_sample();

    /* Phase 3: Prepared Statements */
    database_prepared_statement_sample();
    database_prepared_insert_sample();

    /* Phase 4: UPDATE, DELETE, Error Handling */
    database_update_sample();
    database_delete_sample();
    database_error_handling_sample();

    /* Phase 5: Transactions & Columns */
    database_transaction_sample();
    database_column_extraction_sample();

    /* Phase 6: Cleanup */
    database_cleanup_sample();

#else

    printf("  [跳过] SQLite3 未安装\n");
    printf("  安装方法:\n");
    printf("    macOS:  brew install sqlite3\n");
    printf("    Ubuntu: sudo apt install libsqlite3-dev\n");
    printf("    安装后重新编译本教程即可运行此演示。\n");
    printf("    编译时需要链接 -lsqlite3 (更新 Makefile LDFLAGS)。\n");

#endif

    printf("\n数据库演示完毕。\n");
    return 0;
}
