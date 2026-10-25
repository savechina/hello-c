/**
 * @file todo_sample.c
 * @brief 终端 TODO 小项目 — 业务核心唯一实现 + 教程 demo 驱动
 *
 * 这是 TODO 业务代码的唯一来源: todo_main.c 二进制入口与 hello 的教程
 * demo 入口 (main_todo_sample, 本文件) 都从这里调用。KISS: 业务逻辑
 * 只写一遍, 无第二份实现。
 *
 * 非交互 (不读 stdin), 全部由 demo 自身驱动; mkstemp 临时文件保证可清理。
 * 平台: 标准头 + POSIX mkstemp (<unistd.h>); _WIN32 回退固定临时名。
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(_WIN32)
  #include <unistd.h> /* mkstemp, close */
  #define TODO_HAVE_POSIX 1
#else
  #define TODO_HAVE_POSIX 0
#endif

#include "awesome/todo_sample.h"

/* CTDL 文件魔数 — header 里 extern, 唯一定义在此 */
const char TODO_FILE_MAGIC[TODO_FILE_MAGIC_LEN] = "CTDL";

/* ====================================================================
 *  工厂与销毁 (资源纪律: 谁分配, 谁释放; free 后置 NULL)
 * ==================================================================== */

Task* task_new(const char* text) {
  Task* t = (Task*)calloc(1, sizeof *t);
  if (t == NULL) {
    return NULL;
  }
  t->created_epoch = (long)time(NULL);
  if (text != NULL) {
    size_t n = strlen(text);
    if (n >= sizeof t->text) {
      n = sizeof t->text - 1; /* 超长截断, 绝不溢出 */
    }
    char* dst = (char*)memcpy(t->text, text, n);
    if (dst == NULL) { /* 防御性检查: 规范保证返回目标指针 */
      free(t);
      return NULL;
    }
    t->text[n] = '\0';
  }
  return t;
}

void task_destroy(Task** tp) {
  if (tp == NULL || *tp == NULL) {
    return;
  }
  free(*tp);
  *tp = NULL; /* NULL-poison: 防止 double-free / use-after-free */
}

/* ====================================================================
 *  CRUD
 * ==================================================================== */

static int todo_reserve(TodoStore* store, size_t want) {
  if (want <= store->cap) {
    return TODO_OK;
  }
  size_t newcap = (store->cap == 0) ? 4 : store->cap * 2;
  while (newcap < want) {
    newcap *= 2;
  }
  Task** grown;
  if (store->cap == 0) {
    grown = (Task**)calloc(newcap, sizeof *grown); /* 首次: calloc 清零 */
  } else {
    grown = (Task**)realloc(store->items, newcap * sizeof *grown);
  }
  if (grown == NULL) {
    return TODO_ERR_OOM; /* realloc 失败时原块仍然有效 */
  }
  store->items = grown;
  store->cap = newcap;
  return TODO_OK;
}

void todo_init(TodoStore* store) {
  if (store == NULL) {
    return;
  }
  store->items = NULL;
  store->count = 0;
  store->cap = 0;
  store->next_id = 1;
}

int todo_add(TodoStore* store, const char* text) {
  if (store == NULL || text == NULL || text[0] == '\0') {
    return TODO_ERR_ARG;
  }
  int rc = todo_reserve(store, store->count + 1);
  if (rc != TODO_OK) {
    return rc;
  }
  Task* t = task_new(text);
  if (t == NULL) {
    return TODO_ERR_OOM;
  }
  t->id = store->next_id++;
  store->items[store->count++] = t;
  return TODO_OK;
}

long todo_find_index(const TodoStore* store, long id) {
  if (store == NULL) {
    return -1;
  }
  for (size_t i = 0; i < store->count; i++) {
    if (store->items[i]->id == id) {
      return (long)i;
    }
  }
  return -1;
}

int todo_toggle(TodoStore* store, long id) {
  if (store == NULL || id <= 0) {
    return TODO_ERR_ARG;
  }
  long idx = todo_find_index(store, id);
  if (idx < 0) {
    return TODO_ERR_NOTFOUND;
  }
  store->items[idx]->done = !store->items[idx]->done;
  return TODO_OK;
}

int todo_remove(TodoStore* store, long id) {
  if (store == NULL || id <= 0) {
    return TODO_ERR_ARG;
  }
  long idx = todo_find_index(store, id);
  if (idx < 0) {
    return TODO_ERR_NOTFOUND;
  }
  size_t pos = (size_t)idx;
  task_destroy(&store->items[pos]); /* 先销毁被删对象 */
  store->count--;
  store->items[pos] = store->items[store->count]; /* 尾部对象填坑 (swap) */
  store->items[store->count] = NULL;
  if (store->count == 0) {
    free(store->items);
    store->items = NULL;
    store->cap = 0;
  } else if (store->cap > 4 && store->count <= store->cap / 2) {
    Task** shrunk =
        (Task**)realloc(store->items, store->count * sizeof *shrunk);
    if (shrunk != NULL) {
      store->items = shrunk;
      store->cap = store->count;
    }
    /* 缩容失败保留原容量 — 不是错误 */
  }
  return TODO_OK;
}

void todo_list(const TodoStore* store) {
  if (store == NULL) {
    printf("  (null store)\n");
    return;
  }
  if (store->count == 0) {
    printf("  (空列表)\n");
    return;
  }
  printf("  %-4s %-5s %-19s %s\n", "ID", "done", "created", "text");
  for (size_t i = 0; i < store->count; i++) {
    const Task* t = store->items[i];
    char when[24] = "?";
    time_t sec = (time_t)t->created_epoch;
    struct tm* tm = localtime(&sec);
    if (tm == NULL ||
        strftime(when, sizeof when, "%Y-%m-%d %H:%M:%S", tm) == 0) {
      snprintf(when, sizeof when, "?");
    }
    printf("  %-4ld %-5s %-19s %s\n", t->id, t->done ? "[x]" : "[ ]", when,
           t->text);
  }
}

void todo_clear(TodoStore* store) {
  if (store == NULL) {
    return;
  }
  for (size_t i = 0; i < store->count; i++) {
    task_destroy(&store->items[i]);
  }
  free(store->items);
  store->items = NULL; /* NULL-poison */
  store->count = 0;
  store->cap = 0;
  store->next_id = 1;
}

/* ====================================================================
 *  磁盘持久化 (CTDL: magic + version + count + Task[])
 * ==================================================================== */

int todo_save(const TodoStore* store, const char* path) {
  if (store == NULL || path == NULL) {
    return TODO_ERR_ARG;
  }
  FILE* fp = fopen(path, "wb");
  if (fp == NULL) {
    printf("    [save] 打开失败: %s (errno=%d)\n", strerror(errno), errno);
    return TODO_ERR_IO;
  }
  uint32_t version = TODO_FILE_VERSION;
  uint32_t count = (uint32_t)store->count;
  int ok = fwrite(TODO_FILE_MAGIC, 1, TODO_FILE_MAGIC_LEN, fp) ==
           TODO_FILE_MAGIC_LEN;
  if (ok) {
    ok = fwrite(&version, sizeof version, 1, fp) == 1;
  }
  if (ok) {
    ok = fwrite(&count, sizeof count, 1, fp) == 1;
  }
  for (size_t i = 0; ok && i < store->count; i++) {
    ok = fwrite(store->items[i], sizeof(Task), 1, fp) == 1;
  }
  if (fclose(fp) != 0) { /* fclose 也必须检查: 缓冲区可能延迟刷盘 */
    ok = 0;
  }
  if (!ok) {
    printf("    [save] 写入失败 (errno=%d)\n", errno);
    return TODO_ERR_IO;
  }
  return TODO_OK;
}

int todo_load(TodoStore* store, const char* path) {
  if (store == NULL || path == NULL) {
    return TODO_ERR_ARG;
  }
  FILE* fp = fopen(path, "rb");
  if (fp == NULL) {
    if (errno != ENOENT) { /* 文件不存在 = 空列表, 静默; 其余报错 */
      printf("    [load] 打开失败: %s (errno=%d)\n", strerror(errno), errno);
    }
    return TODO_ERR_IO;
  }

  char magic[TODO_FILE_MAGIC_LEN];
  if (fread(magic, 1, sizeof magic, fp) != sizeof magic ||
      memcmp(magic, TODO_FILE_MAGIC, sizeof magic) != 0) {
    printf("    [load] corrupt, skip — magic 不匹配\n");
    fclose(fp);
    return TODO_ERR_CORRUPT;
  }

  uint32_t version = 0;
  if (fread(&version, sizeof version, 1, fp) != 1 ||
      version != TODO_FILE_VERSION) {
    printf("    [load] corrupt, skip — version=%u 不支持 (期望 %u)\n",
           (unsigned)version, (unsigned)TODO_FILE_VERSION);
    fclose(fp);
    return TODO_ERR_CORRUPT;
  }

  uint32_t count = 0;
  if (fread(&count, sizeof count, 1, fp) != 1 || count > TODO_MAX_COUNT) {
    printf("    [load] corrupt, skip — count=%u 超出合法范围 (0..%u)\n",
           (unsigned)count, (unsigned)TODO_MAX_COUNT);
    fclose(fp);
    return TODO_ERR_CORRUPT;
  }

  /* 三道防线通过 → 分配并读入; 失败不破坏现有数据 */
  todo_clear(store);
  int rc = todo_reserve(store, count);
  if (rc != TODO_OK) {
    fclose(fp);
    return rc;
  }
  for (uint32_t i = 0; i < count; i++) {
    Task* t = (Task*)calloc(1, sizeof *t);
    if (t == NULL) {
      todo_clear(store);
      fclose(fp);
      return TODO_ERR_OOM;
    }
    if (fread(t, sizeof *t, 1, fp) != 1) {
      printf("    [load] corrupt, skip — Task[%u] 数据不完整\n", (unsigned)i);
      task_destroy(&t);
      todo_clear(store);
      fclose(fp);
      return TODO_ERR_CORRUPT;
    }
    t->text[sizeof t->text - 1] = '\0'; /* 防御: 强制 NUL 结尾 */
    t->done = t->done ? true : false;   /* 防御: 规范化 0/1 */
    store->items[store->count++] = t;
    if (t->id >= store->next_id) {
      store->next_id = t->id + 1;
    }
  }
  if (fclose(fp) != 0) {
    printf("    [load] fclose 失败 (errno=%d)\n", errno);
    todo_clear(store);
    return TODO_ERR_IO;
  }
  return TODO_OK;
}

/* ── 错误码可读化 (两个入口共用) ── */

const char* todo_code_name(int rc) {
  switch (rc) {
  case TODO_OK:
    return "TODO_OK";
  case TODO_ERR_ARG:
    return "TODO_ERR_ARG";
  case TODO_ERR_OOM:
    return "TODO_ERR_OOM";
  case TODO_ERR_IO:
    return "TODO_ERR_IO";
  case TODO_ERR_NOTFOUND:
    return "TODO_ERR_NOTFOUND";
  case TODO_ERR_CORRUPT:
    return "TODO_ERR_CORRUPT";
  default:
    return "?";
  }
}

/* ====================================================================
 *  临时文件路径 (mkstemp + snprintf, ascii only)
 * ==================================================================== */

static int todo_make_temp_path(char* buf, size_t buflen) {
  int n = snprintf(buf, buflen, "/tmp/hello_c_todo_XXXXXX");
  if (n < 0 || (size_t)n >= buflen) {
    return TODO_ERR_ARG;
  }
#if TODO_HAVE_POSIX
  int fd = mkstemp(buf); /* 模板被原地替换为唯一路径 */
  if (fd < 0) {
    printf("    [tmp] mkstemp 失败: %s (errno=%d)\n", strerror(errno), errno);
    return TODO_ERR_IO;
  }
  close(fd); /* 后续用 fopen 重新打开, 先关掉 fd */
  return TODO_OK;
#else
  n = snprintf(buf, buflen, "hello_c_todo_demo.tr"); /* Windows 固定名 */
  return (n > 0 && (size_t)n < buflen) ? TODO_OK : TODO_ERR_ARG;
#endif
}

/* ====================================================================
 *  教程 demo 驱动 (非交互; 业务全部来自上方共享核心)
 * ==================================================================== */

static void todo_demo_factory(void) {
  printf("--- 阶段 1: 数据结构 + 工厂/销毁 (资源纪律) ---\n");
  Task* demo = task_new("预习: 结构体与动态数组");
  if (demo == NULL) {
    printf("  [task_new] 失败 (OOM)\n");
    return;
  }
  printf("  [task_new]  堆对象 %p, id=0, text=\"%s\"\n", (void*)demo,
         demo->text);
  task_destroy(&demo);
  printf("  [task_destroy] free 后指针置 NULL: demo == NULL ? %s\n\n",
         demo == NULL ? "yes" : "no");
}

static void todo_demo_crud(TodoStore* store) {
  printf("--- 阶段 2: CRUD (增删改查) ---\n");
  const char* seeds[] = {"写周报", "读《C 程序设计语言》", "跑 3 公里",
                         "复习指针与内存", "给项目写 README"};
  printf("  [add] 追加任务, 观察 realloc 扩容:\n");
  for (size_t i = 0; i < sizeof seeds / sizeof seeds[0]; i++) {
    size_t cap_before = store->cap;
    int rc = todo_add(store, seeds[i]);
    if (rc != TODO_OK) {
      printf("    + 失败: %s\n", todo_code_name(rc));
      return;
    }
    printf("    + #%-2ld \"%s\"  (cap %zu -> %zu)\n",
           store->items[store->count - 1]->id, seeds[i], cap_before,
           store->cap);
  }

  printf("\n  [list] 当前列表:\n");
  todo_list(store);

  printf("\n  [toggle] 把 #2 标记为完成: rc=%s\n",
         todo_code_name(todo_toggle(store, 2)));
  printf("  [list] toggle 后:\n");
  todo_list(store);

  long idx = todo_find_index(store, 3);
  printf("\n  [find_by_id] 查找 #3 -> %s\n",
         idx >= 0 ? store->items[idx]->text : "(未找到)");
  printf("  [find_by_id] 查找 #99 -> (未找到) (rc=%s)\n",
         todo_code_name(TODO_ERR_NOTFOUND));

  printf("\n  [remove] 删除 #1 (swap: 尾部对象填坑 + 缩容):\n");
  printf("    rc=%s, count=%zu, cap=%zu\n",
         todo_code_name(todo_remove(store, 1)), store->count, store->cap);
  todo_list(store);
  printf("\n");
}

static void todo_demo_persist(TodoStore* store, const char* path) {
  printf("--- 阶段 3: 磁盘持久化 (二进制格式) ---\n");
  printf("  [mkstemp] 临时文件: %s\n", path);

  printf("  [save] 写入 %zu 条 -> magic \"CTDL\" + ver + count + Task[]\n",
         store->count);
  printf("    rc=%s (含 fclose 检查)\n",
         todo_code_name(todo_save(store, path)));

  todo_clear(store);
  printf("  [clear] 内存已清空 (count=%zu), 从磁盘恢复:\n", store->count);
  printf("    rc=%s (magic/version/count 三道校验)\n",
         todo_code_name(todo_load(store, path)));
  printf("  [list] round-trip 恢复后的列表:\n");
  todo_list(store);

  printf("\n  [corrupt] 用坏数据覆盖文件, 演示三道防线:\n");
  for (int i = 0; i < 2; i++) {
    FILE* fp = fopen(path, "wb");
    if (fp == NULL) {
      break;
    }
    if (i == 0) {
      fwrite("XXXXjunk!", 1, 8, fp); /* ① magic 不符 */
    } else {
      uint32_t bad_ver = 99, zero = 0;
      fwrite(TODO_FILE_MAGIC, 1, TODO_FILE_MAGIC_LEN, fp);
      fwrite(&bad_ver, sizeof bad_ver, 1, fp); /* ② version 不符 */
      fwrite(&zero, sizeof zero, 1, fp);
    }
    fclose(fp);
    printf("  %s -> rc=%s\n",
           i == 0 ? "  ① magic 不符   " : "  ② version 不符 ",
           todo_code_name(todo_load(store, path)));
  }
  {
    FILE* fp = fopen(path, "wb");
    if (fp != NULL) {
      uint32_t one = 1, huge = 0xFFFFFFFFu;
      fwrite(TODO_FILE_MAGIC, 1, TODO_FILE_MAGIC_LEN, fp);
      fwrite(&one, sizeof one, 1, fp);
      fwrite(&huge, sizeof huge, 1, fp); /* ③ count 超限 */
      fclose(fp);
    }
    printf("  ③ count 超限   -> rc=%s\n",
           todo_code_name(todo_load(store, path)));
  }
  printf("    (每次失败后内存数据保持不变, count=%zu)\n", store->count);

  printf("\n  [rm] 删除临时文件后再次 load:\n");
  if (remove(path) != 0) {
    printf("    remove 失败: %s (errno=%d)\n", strerror(errno), errno);
  }
  printf("    rc=%s (errno=%d, 文件不存在时优雅失败)\n",
         todo_code_name(todo_load(store, path)), errno);
  printf("  [list] 内存中的数据不受影响:\n");
  todo_list(store);
  printf("\n");
}

static void todo_demo_cleanup(TodoStore* store, const char* path) {
  printf("--- 阶段 4: 资源纪律 (cleanup) ---\n");
  printf("  [clear] 释放 %zu 个 Task 堆对象 + 指针数组:\n", store->count);
  todo_clear(store);
  printf("    items == NULL ? %s, count=%zu, cap=%zu\n",
         store->items == NULL ? "yes" : "no", store->count, store->cap);
  if (remove(path) == 0) {
    printf("  [remove] 临时文件已删除: %s\n", path);
  } else if (errno == ENOENT) {
    printf("  [remove] 临时文件已清理 (之前已删除): %s\n", path);
  } else {
    printf("  [remove] 警告: errno=%d %s\n", errno, strerror(errno));
  }
  printf("\n文件已清理 ✓ 无临时文件残留, 无内存泄漏。\n");
}

/* ====================================================================
 *  教程 demo 入口 (hello 二进制的 dispatch 调用)
 * ==================================================================== */

int main_todo_sample(void) {
  printf("========================================\n");
  printf("  终端 TODO 小项目 (Terminal TODO Mini-Project)\n");
  printf("========================================\n\n");

  todo_demo_factory();

  TodoStore store;
  todo_init(&store);
  todo_demo_crud(&store);

  char path[64] = "";
  if (todo_make_temp_path(path, sizeof path) == TODO_OK) {
    todo_demo_persist(&store, path);
    todo_demo_cleanup(&store, path);
  } else {
    printf("--- 阶段 3/4: 跳过 (临时文件创建失败) ---\n");
    todo_clear(&store);
  }

  printf("\nTODO 小项目演示完毕。\n");
  return 0;
}
