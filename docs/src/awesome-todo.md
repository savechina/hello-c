# 第一个真实项目：终端 TODO 🟡

> "语法 demo 教你句子怎么造，真实项目教你写一篇文章。"

## 开篇故事

你已经学完了变量、指针、结构体、文件 I/O——就像学会了每个单词和语法规则。
但当你坐下来想写一个"能用的程序"时，问题变了：**这些知识要按什么顺序组织起来？**

这一章我们不再做孤立的语法演示，而是从零搭一个**终端 TODO 小项目**：
能增删改查（CRUD）、能把数据存到磁盘、下次启动还在、程序退出时不留一丝垃圾。
它不读键盘——整个流程由程序自己驱动，所以你可以把它当成一个**可重复运行的验收测试**：
`make asan` 跑一遍，内存错误和泄漏当场现形。

你会看到四件事如何咬合在一起：

```
数据结构  →  CRUD  →  磁盘持久化  →  资源纪律
 (怎么存)    (怎么改)    (怎么留住)      (怎么不漏)
```

## 本章适合谁

- 语法都学过，但"合起来写个完整程序"还没练过的人
- 写过 `malloc`/`fopen`，但对"谁分配、谁释放"心里没底的人
- 想知道一个小项目如何组织错误码、参数校验、文件格式的人
- 准备对接 SQLite 章，想先理解"自己造简易存储"有多难的人

## 你会学到什么

- 用 `calloc` + `realloc` 管理一个**贯穿全生命周期**的堆对象数组
- CRUD 四件事的 C 实现：追加扩容、表格化打印、标记完成、交换删除 + 缩容
- 简易**二进制文件格式**：magic + version + count + 数据体，以及三道加载防线
- `mkstemp` 生成可清理的临时文件，`snprintf` 拼路径
- 工程纪律：`int` 错误码、`errno`、`fclose` 检查、`free` 后置 `NULL`、工厂/销毁模式

## 前置要求

- 理解结构体、指针、动态内存（`malloc`/`free`/`realloc`）
- 读过文件 I/O 基础（`fopen`/`fwrite`/`fread`/`fclose`）
- 知道 `errno` 的存在（不熟也没关系，本章会用到）

## 项目蓝图

源码：[`src/awesome/todo_sample.c`](https://github.com/savechina/hello-c/blob/main/src/awesome/todo_sample.c)

```
阶段 1  数据结构 + 工厂/销毁   task_new() / task_destroy(), NULL-poison
阶段 2  CRUD                   add → list → toggle → find → remove
阶段 3  磁盘持久化             mkstemp → save → load → corrupt → rm → load
阶段 4  资源纪律               free 后置 NULL, 删除临时文件, 不留垃圾
```

数据结构只有两个：

```c
typedef struct Task {
  long id;             /* 自增主键, 从 1 开始 */
  bool done;           /* 完成标记 */
  char text[128];      /* 定长文本, 避免碎片 */
  long created_epoch;  /* 创建时间 (Unix 时间戳) */
} Task;

typedef struct TodoStore {
  Task** items;   /* 堆对象指针数组 — calloc/realloc 管理 */
  size_t count;   /* 当前元素个数 */
  size_t cap;     /* 已分配容量 */
  long next_id;   /* 下一个可用 id */
} TodoStore;
```

**为什么是 `Task**`（指针数组）而不是 `Task*`（连续数组）？**
因为删除时要 `free` 单个对象——指针数组让"销毁一个"和"整体释放"分成两步，
正好用来演示资源纪律。连续数组更缓存友好，但删除要 `memmove` 挪数据；
两种设计都对，本章选指针数组是为了教学。

---

## 代码讲解 · 一、数据结构（工厂与销毁）

C 没有构造函数/析构函数，但你可以**用约定模拟**：`task_new` 负责分配并初始化，
`task_destroy` 负责释放并把指针"打毒"（poison）。

```c
static Task* task_new(const char* text) {
  Task* t = (Task*)calloc(1, sizeof *t);   /* calloc: 分配 + 清零一步到位 */
  if (t == NULL) {
    return NULL;                            /* 分配失败必须报告, 不能硬解引用 */
  }
  t->created_epoch = (long)time(NULL);
  if (text != NULL) {
    size_t n = strlen(text);
    if (n >= sizeof t->text) {
      n = sizeof t->text - 1;               /* 超长截断, 绝不溢出 */
    }
    char* dst = (char*)memcpy(t->text, text, n);
    if (dst == NULL) {                      /* 防御性检查: 规范保证返回目标指针 */
      free(t);
      return NULL;
    }
    t->text[n] = '\0';
  }
  return t;
}

static void task_destroy(Task** tp) {
  if (tp == NULL || *tp == NULL) {
    return;
  }
  free(*tp);
  *tp = NULL;   /* NULL-poison: 防止 double-free / use-after-free */
}
```

三个细节值得停下来品：

1. **`calloc` 而不是 `malloc`**：结构体里有 `bool` 和 `char[]`，清零意味着
   `text` 天然是空串、`done` 天然是 `false`——少写三行初始化，也少漏一处。
2. **`task_destroy` 接受 `Task**`**：只有拿到"指针的地址"，才能在释放后
   把调用方的变量改成 `NULL`。传 `Task*` 的话，`free` 完你拿不到那个变量。
3. **截断而不是报错**：`text[128]` 是定长设计，超长就截断——但**必须**手动补
   `'\0'`，否则 `printf("%s")` 会越界读。

> **语言中立的视角**：这个 `new/destroy` 配对，就是 Go 的 `New/Close`、
> Rust 的 `Drop`、Java 的 `constructor/finalize` 在 C 里的手工版本。
> 没有语言替你管，所以纪律必须自己立。

---

## 代码讲解 · 二、CRUD（增删改查）

### add：追加 + realloc 扩容

数组不可能一开始就知道要装多少元素。策略：**容量翻倍**，摊还成本 O(1)。

```c
static int todo_reserve(TodoStore* store, size_t want) {
  if (want <= store->cap) {
    return TODO_OK;                          /* 空间够, 直接返回 */
  }
  size_t newcap = (store->cap == 0) ? 4 : store->cap * 2;
  while (newcap < want) {
    newcap *= 2;
  }
  Task** grown;
  if (store->cap == 0) {
    grown = (Task**)calloc(newcap, sizeof *grown);  /* 首次: calloc 清零 */
  } else {
    grown = (Task**)realloc(store->items, newcap * sizeof *grown);
  }
  if (grown == NULL) {
    return TODO_ERR_OOM;   /* 关键: realloc 失败时原块仍然有效, 不能覆盖指针 */
  }
  store->items = grown;
  store->cap = newcap;
  return TODO_OK;
}
```

**`realloc` 的铁律**：

```c
/* ❌ 错误写法: 一旦 realloc 失败返回 NULL, 原指针被覆盖 → 内存泄漏 + 后续 UAF */
store->items = realloc(store->items, new_size);

/* ✅ 正确写法: 用临时变量接返回值, 检查后再赋值 */
Task** grown = realloc(store->items, new_size);
if (grown == NULL) {
  return TODO_ERR_OOM;   /* 原 store->items 完好无损 */
}
store->items = grown;
```

`add` 本身很薄——校验参数、确保空间、建对象、挂上去：

```c
static int todo_add(TodoStore* store, const char* text) {
  if (store == NULL || text == NULL || text[0] == '\0') {
    return TODO_ERR_ARG;             /* 参数校验: 空指针和空串都拒绝 */
  }
  int rc = todo_reserve(store, store->count + 1);
  if (rc != TODO_OK) {
    return rc;                        /* 错误码逐层上抛 */
  }
  Task* t = task_new(text);
  if (t == NULL) {
    return TODO_ERR_OOM;
  }
  t->id = store->next_id++;
  store->items[store->count++] = t;
  return TODO_OK;
}
```

### list：表格化打印

```c
printf("  %-4s %-5s %-19s %s\n", "ID", "done", "created", "text");
for (size_t i = 0; i < store->count; i++) {
  const Task* t = store->items[i];
  char when[24] = "?";
  time_t sec = (time_t)t->created_epoch;
  struct tm* tm = localtime(&sec);
  if (tm == NULL ||
      strftime(when, sizeof when, "%Y-%m-%d %H:%M:%S", tm) == 0) {
    snprintf(when, sizeof when, "?");   /* 时间戳非法时优雅降级 */
  }
  printf("  %-4ld %-5s %-19s %s\n", t->id, t->done ? "[x]" : "[ ]", when,
         t->text);
}
```

`localtime` 可能返回 `NULL`（时间戳超出范围），`strftime` 可能返回 `0`
（缓冲区太小）——两个返回值都检查，打印 `?` 而不是崩溃。

### toggle / find：线性扫描

```c
static long todo_find_index(const TodoStore* store, long id) {
  for (size_t i = 0; i < store->count; i++) {
    if (store->items[i]->id == id) {
      return (long)i;      /* 找到: 返回下标 */
    }
  }
  return -1;               /* 没找到: 返回 -1 (哨兵值) */
}

static int todo_toggle(TodoStore* store, long id) {
  long idx = todo_find_index(store, id);
  if (idx < 0) {
    return TODO_ERR_NOTFOUND;
  }
  store->items[idx]->done = !store->items[idx]->done;
  return TODO_OK;
}
```

数据量小（< 10⁴）时线性扫描足够；真要亿级数据，才轮到哈希表或数据库
——这正是本章结尾"对接 SQLite"的动机。

### remove：交换删除 + 缩容

删除中间元素，最怕 `memmove` 搬运整段数据。**交换法**：拿最后一个元素填坑，
O(1) 挪动（代价是顺序改变——TODO 列表通常不在乎）。

```c
static int todo_remove(TodoStore* store, long id) {
  long idx = todo_find_index(store, id);
  if (idx < 0) {
    return TODO_ERR_NOTFOUND;
  }
  size_t pos = (size_t)idx;
  task_destroy(&store->items[pos]);               /* ① 先销毁被删对象 */
  store->count--;
  store->items[pos] = store->items[store->count]; /* ② 尾部对象填坑 (swap) */
  store->items[store->count] = NULL;              /* ③ 尾槽置空, 不留悬垂指针 */

  if (store->count == 0) {
    free(store->items);                           /* ④ 空了: 整体释放 */
    store->items = NULL;
    store->cap = 0;
  } else if (store->cap > 4 && store->count <= store->cap / 2) {
    Task** shrunk =
        (Task**)realloc(store->items, store->count * sizeof *shrunk);
    if (shrunk != NULL) {                         /* ⑤ 缩容失败可容忍 */
      store->items = shrunk;
      store->cap = store->count;
    }
  }
  return TODO_OK;
}
```

注意 ⑤ 和 `add` 的区别：**扩容失败是致命错误**（加不进去），
**缩容失败只是浪费一点内存**（功能不受影响）——所以一个返回错误码，
一个只打印提示继续跑。错误处理的"严格程度"要匹配后果。

---

## 代码讲解 · 三、磁盘持久化（IO）

### 文件格式

自己定一个最简单的二进制格式：

```
偏移      内容                说明
────────  ──────────────────  ─────────────────────────
0..3      "CTDL"              4 字节魔数 (magic), 标识文件类型
4..7      version (u32)       格式版本, 未来不兼容时 +1
8..11     count (u32)         Task 个数
12..      Task[count]         结构体数组, 顺序存储
```

### save：写入 + fclose 检查

```c
static int todo_save(const TodoStore* store, const char* path) {
  FILE* fp = fopen(path, "wb");
  if (fp == NULL) {
    printf("    [save] 打开失败: %s (errno=%d)\n", strerror(errno), errno);
    return TODO_ERR_IO;
  }
  uint32_t version = TODO_FILE_VERSION;
  uint32_t count = (uint32_t)store->count;
  int ok = fwrite(TODO_MAGIC, 1, TODO_MAGIC_LEN, fp) == TODO_MAGIC_LEN;
  if (ok) {
    ok = fwrite(&version, sizeof version, 1, fp) == 1;
  }
  if (ok) {
    ok = fwrite(&count, sizeof count, 1, fp) == 1;
  }
  for (size_t i = 0; ok && i < store->count; i++) {
    ok = fwrite(store->items[i], sizeof(Task), 1, fp) == 1;
  }
  if (fclose(fp) != 0) {   /* fclose 也必须检查: 缓冲区可能延迟刷盘 */
    ok = 0;
  }
  return ok ? TODO_OK : TODO_ERR_IO;
}
```

**为什么 `fclose` 要检查？** `fwrite` 成功只说明数据进了**用户态缓冲区**，
真正落盘发生在 `fclose`（或 `fflush`）时。磁盘满、网络盘断连……
错误往往在最后一刻才暴露。只查 `fwrite` 不查 `fclose`，等于关门不确认锁没锁上。

### load：三道防线

加载外部数据 = 信任边界。**永远不要假设文件是好的**：

```c
/* 防线 1: magic — 这是我们家的文件吗? */
char magic[TODO_MAGIC_LEN];
if (fread(magic, 1, sizeof magic, fp) != sizeof magic ||
    memcmp(magic, TODO_MAGIC, sizeof magic) != 0) {
  printf("    [load] corrupt, skip — magic 不匹配\n");
  fclose(fp);
  return TODO_ERR_CORRUPT;
}

/* 防线 2: version — 我认识这个格式版本吗? */
uint32_t version = 0;
if (fread(&version, sizeof version, 1, fp) != 1 ||
    version != TODO_FILE_VERSION) {
  printf("    [load] corrupt, skip — version=%u 不支持\n",
         (unsigned)version);
  fclose(fp);
  return TODO_ERR_CORRUPT;
}

/* 防线 3: count — 这个数字合理吗? (防巨额分配) */
uint32_t count = 0;
if (fread(&count, sizeof count, 1, fp) != 1 || count > TODO_MAX_COUNT) {
  printf("    [load] corrupt, skip — count=%u 超出合法范围\n",
         (unsigned)count);
  fclose(fp);
  return TODO_ERR_CORRUPT;
}
```

三道防线的顺序有讲究：**先花小钱验证，再花大钱分配**。
如果先读 `count = 4294967295` 就去 `calloc`，一个 8 字节的坏文件就能让你
申请 34 GB——这就是 `TODO_MAX_COUNT` 上限存在的意义。

读入成功后，还要对每个对象做**防御性规范化**：

```c
t->text[sizeof t->text - 1] = '\0';  /* 强制 NUL 结尾 (坏文件可能没写) */
t->done = t->done ? true : false;    /* 规范化 0/1 (bool 只应有两个值) */
```

### 临时文件：mkstemp + 可清理

演示不能污染系统，所以用 `mkstemp` 生成唯一路径，用完即删：

```c
static int todo_make_temp_path(char* buf, size_t buflen) {
  int n = snprintf(buf, buflen, "/tmp/hello_c_todo_XXXXXX");
  if (n < 0 || (size_t)n >= buflen) {
    return TODO_ERR_ARG;              /* snprintf 返回值: 负=错误, >=size=截断 */
  }
#if TODO_HAVE_POSIX
  int fd = mkstemp(buf);              /* XXXXXX 被原地替换成随机串 */
  if (fd < 0) {
    printf("    [tmp] mkstemp 失败: %s (errno=%d)\n", strerror(errno), errno);
    return TODO_ERR_IO;
  }
  close(fd);                          /* 后续用 fopen 重新打开, 先关掉 fd */
  return TODO_OK;
#else
  n = snprintf(buf, buflen, "hello_c_todo_demo.tr");  /* Windows 回退: 固定名 */
  return (n > 0 && (size_t)n < buflen) ? TODO_OK : TODO_ERR_ARG;
#endif
}
```

`mkstemp` 比 `tmpnam`/`tmpnam` 安全——后者有**竞态条件**（TOCTOU）：
你拿到名字和你创建文件之间，攻击者可以抢先放一个符号链接。
`mkstemp` 把"生成名字"和"创建文件"合成一个原子操作，直接返回已打开的 fd。

平台守卫 `#if !defined(_WIN32)` 的写法：POSIX 头 `<unistd.h>` 在
macOS/Linux/Solaris/FreeBSD 都有，Windows 没有——用编译期分支隔离，
而不是运行时 `if`（后者会让 Windows 编译直接失败）。

---

## 代码讲解 · 四、工程纪律

这一节没有新功能，全是"不出错"的功夫。

### 错误码：用 int 返回，而不是 abort

```c
enum TodoCode {
  TODO_OK = 0,
  TODO_ERR_ARG = -1,      /* 参数非法 */
  TODO_ERR_OOM = -2,      /* 内存分配失败 */
  TODO_ERR_IO = -3,       /* 文件 IO 失败 */
  TODO_ERR_NOTFOUND = -4, /* id 不存在 */
  TODO_ERR_CORRUPT = -5   /* 文件损坏/格式不符 */
};
```

约定俗成：**0 = 成功，非 0 = 失败**（负数是本项目的自定义错误）。
调用方一行就能判断：

```c
int rc = todo_add(&store, "写周报");
if (rc != TODO_OK) {
  /* 处理失败 */
}
```

### errno：错误的"详细原因"

`fopen` 失败只告诉你"失败了"，`errno` 告诉你**为什么**：

```c
if (fp == NULL) {
  printf("打开失败: %s (errno=%d)\n", strerror(errno), errno);
  /* 打开失败: No such file or directory (errno=2) */
}
```

`errno` 是线程局部的全局量，**只在系统调用失败后立刻读取**才有效——
中间插任何可能失败的调用都可能把它覆盖掉。

### NULL-poison：free 后置 NULL

```c
free(*tp);
*tp = NULL;   /* 这一行, 挡掉一整类 bug */
```

置 `NULL` 不能防止"第一次 free"，但能让**第二次 free 变得无害**
（`free(NULL)` 是标准定义的空操作），也让 use-after-free 变成可探测的空指针解引用
而不是"读到脏数据"。配合 ASan，问题当场暴露。

### 资源清理的完整清单

程序退出前，把所有资源过一遍：

```c
static void todo_clear(TodoStore* store) {
  for (size_t i = 0; i < store->count; i++) {
    task_destroy(&store->items[i]);  /* 逐个销毁堆对象 */
  }
  free(store->items);                /* 销毁数组本身 */
  store->items = NULL;               /* NULL-poison */
  store->count = 0;
  store->cap = 0;
  store->next_id = 1;
}
```

顺序不能反：**先释放元素，再释放容器**。反过来的话，元素指针就丢了，
它们占用的内存永远还不回去。

---

## 运行效果

```text
========================================
  终端 TODO 小项目 (Terminal TODO Mini-Project)
========================================

--- 阶段 1: 数据结构 + 工厂/销毁 (资源纪律) ---
  [task_new]  堆对象 0x60e000000040, id=0, text="预习: 结构体与动态数组"
  [task_destroy] free 后指针置 NULL: demo == NULL ? yes

--- 阶段 2: CRUD (增删改查) ---
  [add] 追加任务, 观察 realloc 扩容:
    + #1  "写周报"  (cap 0 -> 4)
    + #5  "给项目写 README"  (cap 4 -> 8)
  ...
  [remove] 删除 #1 (swap: 尾部对象填坑 + 缩容):
    rc=TODO_OK, count=4, cap=4

--- 阶段 3: 磁盘持久化 (二进制格式) ---
  [mkstemp] 临时文件: /tmp/hello_c_todo_aLlgRg
  [save] 写入 4 条 -> magic "CTDL" + ver + count + Task[]
  [corrupt] 用坏数据覆盖文件, 演示三道防线:
    ① magic 不符    -> rc=TODO_ERR_CORRUPT
    ② version 不符  -> rc=TODO_ERR_CORRUPT
    ③ count 超限    -> rc=TODO_ERR_CORRUPT
  [rm] 删除临时文件后再次 load:
    rc=TODO_ERR_IO (errno=2, 文件不存在时优雅失败)

--- 阶段 4: 资源纪律 (cleanup) ---
  [clear] 释放 4 个 Task 堆对象 + 指针数组:
    items == NULL ? yes, count=0, cap=0

文件已清理 ✓ 无临时文件残留, 无内存泄漏。
```

验收命令：

```bash
make build   # 编译通过 (-Wall -Wextra -Werror)
make asan    # ASan+UBSan 跑一遍, 退出码 0 = 无内存错误
```

---

## 常见错误

### ❌ 错误 1: realloc 返回值直接覆盖原指针

```c
p = realloc(p, new_size);   /* 失败时 p 变 NULL, 原内存泄漏 */
```

**修复**：临时变量接返回值，检查成功后再赋值（见 `todo_reserve`）。

### ❌ 错误 2: 只查 fwrite, 不查 fclose

```c
fwrite(data, 1, n, fp);
fclose(fp);                 /* 磁盘满时这里才报错, 被你忽略 */
```

**修复**：`if (fclose(fp) != 0) { /* 报错 */ }`。

### ❌ 错误 3: 信任文件里的 count

```c
fread(&count, 4, 1, fp);
buf = malloc(count * sizeof *buf);   /* count=0xFFFFFFFF → 申请 64 GB */
```

**修复**：先校验上限（`count > TODO_MAX_COUNT` → corrupt），再分配。

### ❌ 错误 4: free 了不置 NULL

```c
free(store->items);
/* ... 200 行之后 ... */
store->items[i] = x;   /* use-after-free, ASan 会抓, 但生产环境靠运气 */
```

**修复**：`free` 后立刻 `= NULL`，让错误变成显眼的空指针崩溃。

### ❌ 错误 5: 先释放容器, 再想释放元素

```c
free(store->items);              /* 元素指针全丢了 */
for (...) task_destroy(...);     /* 悬垂指针, double-free */
```

**修复**：先元素，后容器（见 `todo_clear`）。

---

## 建议延伸：对接 SQLite 章

你刚手写的这套东西——增删改查、文件格式、损坏检测——**正是一个迷你数据库**。
现在回头看 [数据库 (SQLite)](../database.md) 章，会有完全不同的体感：

| 本章手写的 | SQLite 里的对应物 |
|-----------|------------------|
| `CTDL` + version + count 头 | 数据库文件头 (magic "SQLite format 3") |
| `TODO_MAX_COUNT` 上限校验 | `PRAGMA max_page_count` / 页数校验 |
| `todo_find_index` 线性扫描 | B-tree 索引 (O(log n)) |
| `todo_save` 整体重写 | WAL / journal 增量日志 |
| `TODO_ERR_CORRUPT` | `SQLITE_CORRUPT` |
| 单进程、无并发 | 多进程、多连接、锁机制 |

**练习思路**：把本章的 `TodoStore` 换成 SQLite 后端——
`todo_add` 变成 `INSERT`，`todo_list` 变成 `SELECT`，
接口（那几个 `todo_*` 函数签名）一行不改。这就是**抽象层**的价值：
数据结构和持久化是两回事，可以分别替换。

---

## 小结

- **数据结构先行**：`Task`（数据）和 `TodoStore`（容器）分开，容器只管指针数组的生命周期
- **扩容翻倍、缩容减半**：摊还 O(1)；`realloc` 用临时变量接返回值，失败不丢原块
- **交换删除**：尾部填坑 O(1)，代价是乱序——按需取舍
- **文件三道防线**：magic → version → count，**先验证后分配**，坏数据一律 `corrupt, skip`
- **`fclose` 必须检查**：数据可能在关闭时才真正落盘
- **资源纪律**：`free` 后置 `NULL`；先释放元素再释放容器；临时文件用 `mkstemp` + 用完即删
- **错误码 + errno**：`int` 返回值判断"是否失败"，`errno` 解释"为什么失败"

> **我的教训是**：第一次写这个项目时，我在 `remove` 里先 `free(store->items)`
> 再遍历销毁元素——程序跑得好好的，直到 ASan 报 use-after-free。
> 资源纪律不是"记得就行"，而是**固定的清理顺序 + free 后置 NULL + 每次跑 sanitizer**。

## 练习

### 🟢 练习 1：搜索功能

给 `TodoStore` 加一个 `todo_search(store, keyword)`：
遍历所有任务，`strstr` 匹配 `text`，返回匹配的 id 列表。
思考：返回 `long*`（谁分配谁释放？）还是打印出来（谁都不用管）？

### 🟡 练习 2：文件格式升级到 version 2

给 `Task` 加一个 `long due_epoch`（截止时间）字段，把 `TODO_FILE_VERSION` 改成 2。
让 `todo_load` 同时接受 version 1 和 2：读到 v1 时把 `due_epoch` 填 0。
这就是**真实软件升级数据格式**的最小模型——永远要能读旧数据。

### 🔴 练习 3：原子保存（防断电丢数据）

现在的 `todo_save` 直接覆盖目标文件——写到一半断电，数据就毁了。
改成**写临时文件 + rename**：

```c
snprintf(tmp, sizeof tmp, "%s.tmp", path);
/* 写 tmp, fclose 检查通过后 */
rename(tmp, path);   /* 同一文件系统内, rename 是原子的 */
```

验证：写入过程中 `kill -9` 自己，重启后数据必须完整（要么旧版要么新版）。

---

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 增删改查（CRUD） | Create/Read/Update/Delete 四种基本数据操作 |
| 工厂函数（Factory） | 负责分配并初始化对象的函数，如 `task_new` |
| 空指针打毒（NULL-poison） | `free` 后把指针置 `NULL`，防止重复释放 |
| 魔数（Magic Number） | 文件开头的固定字节串，标识文件类型 |
| 容量翻倍（Growth Factor） | 动态数组扩容策略，摊还时间 O(1) |
| 交换删除（Swap-Delete） | 用尾元素填充被删位置，O(1) 删除 |
| 竞态条件（TOCTOU） | Check-Then-Use 的时间窗口攻击面 |
| 信任边界（Trust Boundary） | 外部数据（文件/网络）必须先验证再使用 |
| 原子操作（Atomic Operation） | 要么完全成功、要么完全不发生的操作 |
| 资源纪律（Resource Discipline） | 统一的分配/释放规则，防止泄漏和悬垂指针 |

## 延伸阅读

- [C11 标准 §7.22.3](https://en.cppreference.com/w/c/memory) — `malloc`/`realloc`/`free` 语义
- [mkstemp(3) man page](https://man7.org/linux/man-pages/man3/mkstemp.3.html) — 安全临时文件
- [SQLite 文件格式](https://www.sqlite.org/fileformat.html) — 真实数据库的文件头长什么样
- [Beej's Guide to C](https://beej.us/guide/bgc/) — 动态内存与文件 I/O 章节

## 继续学习

你已经完整走了一遍"从数据结构到磁盘"的小项目闭环——这是所有系统程序的骨架：
**内存里怎么组织 → 磁盘上怎么留住 → 出错时怎么不漏**。

在下一章节中，我们将探索更底层的系统能力。

> 💡 **提示**：本章的 `save/load` 是"把内存原样倒到磁盘"。当你需要按条件查询、
> 并发访问、或数据量超过内存时，就该把这套手工活交给数据库——
> 这正是 [SQLite 章](../database.md) 要讲的。

[← 上一章：资源精选](awesome.md) | [回到目录 → SUMMARY](index.md)
