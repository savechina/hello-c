/**
 * @file todo_main.c
 * @brief 终端 TODO 应用 — 独立二进制入口 (build/bin/todo, 与 hello 分离)
 *
 * 薄层 (KISS): 只做三件事 — argv 解析、帮助体系、数据文件路径解析;
 * 业务逻辑全部来自共享核心 awesome/todo_core (业务只写一遍)。
 * Makefile 的 SOURCES 已排除本文件 (防双 main 冲突)。
 *
 * 用法:
 *   todo                 列出全部任务 (默认命令)
 *   todo add <text>      新增任务
 *   todo done <id>       标记完成/未完成 (toggle)
 *   todo rm   <id>       删除任务
 *   todo clear           清空并删除数据文件 (幂等)
 *   todo --help | help [cmd] | --version
 *
 * 存储: $TODO_FILE, 缺省 $HOME/.hello-c-todo.dat (CTDL 二进制格式,
 * 与教程 demo awesome/todo_sample.c 共享同一份实现)。
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>

#include "awesome/todo_sample.h"

#define TODO_APP_VERSION "0.1.0"

/* ── 帮助体系 (clap 风格: --help/-h/--version/-V/help <cmd>) ── */

static void todo_print_usage(void) {
  printf("A terminal TODO app in pure C — 终端 TODO 小应用\n\n");
  printf("Usage: todo [OPTIONS] [COMMAND]\n\n");
  printf("Commands:\n");
  printf("  list       Show all tasks (default when no command is given)\n");
  printf("  add <text> Add a new task\n");
  printf("  done <id>  Toggle a task between [open] and [done]\n");
  printf("  rm <id>    Remove a task\n");
  printf("  clear      Delete the data file (reset everything)\n");
  printf("  help [cmd] Show help for a command\n\n");
  printf("Options:\n");
  printf("  -h, --help       Print help\n");
  printf("  -V, --version    Print version\n\n");
  printf("Environment:\n");
  printf("  TODO_FILE  Path of the data file (default: "
         "$HOME/.hello-c-todo.dat)\n\n");
  printf("Examples:\n");
  printf("  todo add \"买牛奶\"      # 新增任务\n");
  printf("  todo list              # 列表\n");
  printf("  todo done 1            # 完成 #1\n");
  printf("  TODO_FILE=/tmp/t.dat todo add test\n");
}

static void todo_print_cmd_help(const char* cmd) {
  if (strcmp(cmd, "add") == 0) {
    printf("Add a new task\n\n");
    printf("Usage: todo add <text>\n");
    printf(
        "  <text>  任务内容 (空格要整体加引号; 最大 %d 字节, 自动 NUL 截断)\n",
        TODO_TEXT_MAX - 1);
  } else if (strcmp(cmd, "done") == 0) {
    printf("Toggle a task's done state\n\n");
    printf("Usage: todo done <id>\n");
    printf("  <id>    任务 ID (todo list 第一列; 必须 > 0)\n");
    printf("          已完成会退回 [open] — toggle 语义\n");
  } else if (strcmp(cmd, "rm") == 0) {
    printf("Remove a task\n\n");
    printf("Usage: todo rm <id>\n");
    printf("  <id>    任务 ID (todo list 第一列)\n");
  } else if (strcmp(cmd, "clear") == 0) {
    printf("Delete the data file\n\n");
    printf("Usage: todo clear\n");
    printf("  文件不存在 (ENOENT) 视为成功 — 幂等\n");
  } else if (strcmp(cmd, "list") == 0) {
    printf("Show all tasks (default command)\n\n");
    printf("Usage: todo list\n");
  } else {
    printf("  [错误] 无命令帮助 \"%s\"。可用: list/add/done/rm/clear\n", cmd);
    exit(1);
  }
}

/* ── 数据文件路径: $TODO_FILE → $HOME 默认 ── */

static int todo_path(char* buf, size_t bufsz) {
  const char* env = getenv("TODO_FILE");
  if (env != NULL && env[0] != '\0') {
    int n = snprintf(buf, bufsz, "%s", env);
    return (n >= 0 && (size_t)n < bufsz) ? 0 : -1;
  }
#if !defined(_WIN32)
  const char* home = getenv("HOME");
  if (home != NULL && home[0] != '\0') {
    int n = snprintf(buf, bufsz, "%s/.hello-c-todo.dat", home);
    return (n >= 0 && (size_t)n < bufsz) ? 0 : -1;
  }
#endif
  int n = snprintf(buf, bufsz, "hello-c-todo.dat"); /* Windows 兜底: 当前目录 */
  return (n > 0 && (size_t)n < bufsz) ? 0 : -1;
}

/* ── 命令实现 (薄壳 — 业务调用全部来自 todo_core) ── */

static int cmd_add(TodoStore* s, const char* text, const char* path) {
  int rc = todo_add(s, text);
  if (rc != TODO_OK) {
    fprintf(stderr, "  [错误] add 失败: %s\n", todo_code_name(rc));
    return 1;
  }
  long idx = todo_find_index(s, s->next_id - 1);
  if (idx >= 0)
    printf("  新增 #%ld: %s\n", s->next_id - 1, s->items[idx]->text);
  return todo_save(s, path);
}

static int cmd_toggle(TodoStore* s, long id, const char* path) {
  int rc = todo_toggle(s, id);
  if (rc != TODO_OK) {
    fprintf(stderr, "  [错误] #%ld %s\n", id, todo_code_name(rc));
    return 1;
  }
  long idx = todo_find_index(s, id);
  if (idx >= 0)
    printf("  #%ld → %s\n", id, s->items[idx]->done ? "[done]" : "[open]");
  return todo_save(s, path);
}

static int cmd_remove(TodoStore* s, long id, const char* path) {
  long idx = todo_find_index(s, id);
  char text[TODO_TEXT_MAX] = "?";
  if (idx >= 0)
    snprintf(text, sizeof text, "%s", s->items[idx]->text); /* 拷贝防悬垂 */
  int rc = todo_remove(s, id);
  if (rc != TODO_OK) {
    fprintf(stderr, "  [错误] #%ld %s\n", id, todo_code_name(rc));
    return 1;
  }
  printf("  删除 #%ld: %s\n", id, text);
  return todo_save(s, path);
}

/* ── 入口: 只在 build/bin/todo 中被链接 (SOURCES 已排除, 防双 main) ── */

int main(int argc, char* argv[]) {
  char path[512];
  if (todo_path(path, sizeof(path)) != 0) {
    fprintf(stderr, "[错误] TODO_FILE 路径过长\n");
    return 1;
  }

  /* 全局旗标先行 — clap 亦按 parse order 处理 --help/-V */
  if (argc >= 2 &&
      (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
    todo_print_usage();
    return 0;
  }
  if (argc >= 2 &&
      (strcmp(argv[1], "-V") == 0 || strcmp(argv[1], "--version") == 0)) {
    printf("todo %s\n", TODO_APP_VERSION);
    return 0;
  }
  if (argc >= 2 && strcmp(argv[1], "help") == 0) {
    if (argc >= 3) {
      todo_print_cmd_help(argv[2]);
    } else {
      todo_print_usage();
    }
    return 0;
  }

  TodoStore store;
  todo_init(&store);

  /* 数据文件不存在 (ENOENT) = 空列表, 不是错误; 其余错误照样报 */
  (void)errno;
  int rc_load = todo_load(&store, path);
  bool file_missing =
      (rc_load == TODO_ERR_IO && access(path, F_OK) != 0 && errno == ENOENT);
  if (rc_load != TODO_OK && !file_missing) {
    fprintf(stderr, "  [提示] 从 %s 读取失败: %s, 数据视为空\n", path,
            todo_code_name(rc_load));
  }
  if (file_missing) {
    store.count = 0;
  }

  int status = 0;
  if (argc < 2 || strcmp(argv[1], "list") == 0) {
    todo_list(&store);
  } else if (strcmp(argv[1], "add") == 0) {
    if (argc < 3 || argv[2][0] == '\0') {
      fprintf(stderr, "  [错误] 用法: todo add <text>\n");
      status = 1;
    } else {
      status = cmd_add(&store, argv[2], path);
    }
  } else if (strcmp(argv[1], "done") == 0) {
    long id = (argc >= 3) ? strtol(argv[2], NULL, 10) : 0;
    if (id <= 0) {
      fprintf(stderr, "  [错误] 用法: todo done <id>\n");
      status = 1;
    } else {
      status = cmd_toggle(&store, id, path);
    }
  } else if (strcmp(argv[1], "rm") == 0) {
    long id = (argc >= 3) ? strtol(argv[2], NULL, 10) : 0;
    if (id <= 0) {
      fprintf(stderr, "  [错误] 用法: todo rm <id>\n");
      status = 1;
    } else {
      status = cmd_remove(&store, id, path);
    }
  } else if (strcmp(argv[1], "clear") == 0) {
    if (remove(path) != 0 && errno != ENOENT) {
      fprintf(stderr, "  [错误] 删除 %s: %s\n", path, strerror(errno));
      status = 1;
    } else {
      printf("  已清空 (数据文件 %s 已删除)\n", path);
    }
  } else {
    fprintf(stderr, "  [错误] 未知命令 \"%s\"。Try 'todo --help'\n", argv[1]);
    status = 1;
  }

  todo_clear(&store);
  return status;
}
