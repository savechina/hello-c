#ifndef TODO_SAMPLE_H
#define TODO_SAMPLE_H

/**
 * @brief 终端 TODO 小项目 — 全部 TODO 代码的家 (业务核心 + 教程 demo 入口)
 *
 * todo_main.c (独立二进制 build/bin/todo, 自带 main(), Makefile SOURCES
 * 已排除) 与教程 demo 都从这里调用 — 业务逻辑只写一遍 (KISS, 无重复)。
 *
 * 能力清单:
 *   1. 数据结构   — Task / TodoStore, calloc 堆对象数组贯穿完整生命周期
 *   2. CRUD       — add (realloc 扩容) / list (表格) / toggle / remove
 *                   (swap+缩容) / find_by_id
 *   3. 持久化     — mkstemp 临时文件 + 简易二进制格式
 *                   (magic "CTDL" + version u32 + count u32 + Task 数组)
 *                   load 校验 magic/version/count, 坏数据 → "corrupt, skip"
 *   4. 工程纪律   — 参数校验 (int 错误码), errno, snprintf 路径, fclose 检查,
 *                   free 后置 NULL (NULL-poison)
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TODO_TEXT_MAX 128      /* 任务文本缓冲大小 (含 NUL) */
#define TODO_FILE_MAGIC_LEN 4u /* 文件魔数长度 */
#define TODO_MAX_COUNT 100000u /* count 合法上限 (防损坏文件巨额分配) */
#define TODO_FILE_VERSION 1u   /* 文件格式版本 */

/* CTDL 文件魔数: 4 字节 "CTDL" (C ToDo List)。
 * 裸字节用途 (fwrite/memcmp), 无 NUL 终止; 定义处用逐字符初始化,
 * 避开 clang 21+ 的 -Wunterminated-string-initialization */
extern const char TODO_FILE_MAGIC[TODO_FILE_MAGIC_LEN];

/** 错误码 — 负值为失败, 0 为成功 */
enum TodoCode {
  TODO_OK = 0,
  TODO_ERR_ARG = -1,      /* 参数非法 */
  TODO_ERR_OOM = -2,      /* 内存分配失败 */
  TODO_ERR_IO = -3,       /* 文件 IO 失败 */
  TODO_ERR_NOTFOUND = -4, /* id 不存在 */
  TODO_ERR_CORRUPT = -5   /* 文件损坏/格式不符 */
};

typedef struct Task {
  long id;
  bool done;
  char text[TODO_TEXT_MAX];
  long created_epoch;
} Task;

typedef struct TodoStore {
  Task** items; /* 堆对象指针数组 (calloc/realloc 管理) */
  size_t count;
  size_t cap;
  long next_id;
} TodoStore;

/* ── 工厂/销毁与 CRUD ── */

Task* task_new(const char* text);
void task_destroy(Task** tp);

void todo_init(TodoStore* store);
void todo_clear(TodoStore* store);
int todo_add(TodoStore* store, const char* text);
long todo_find_index(const TodoStore* store, long id);
int todo_toggle(TodoStore* store, long id);
int todo_remove(TodoStore* store, long id);
void todo_list(const TodoStore* store);

/* ── 持久化 (CTDL 二进制) ── */

int todo_save(const TodoStore* store, const char* path);
int todo_load(TodoStore* store, const char* path);

const char* todo_code_name(int rc);

/* ── 教程 demo 入口 (hello 的 main.c dispatch 经 main_awesome_sample 调用) ── */

int main_todo_sample(void);

#endif /* TODO_SAMPLE_H */
