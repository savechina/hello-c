# 不透明指针 (Opaque Pointers & RAII Patterns)

## 开篇故事

想象一家酒店的保险箱 (safe deposit box)。你走进前台, 服务员给你一个编号, 你用这个编号存取物品。你**不知道**保险箱长什么样、里面装了什么、钥匙怎么工作——你只拿到一把「钥匙」(指针), 用这把钥匙存取东西。当你退房 (作用域结束) 时, 保险箱自动上锁并清零。

这就是 C 语言中的**不透明指针 (opaque pointer)** 设计: 调用者拿到一个指针, 但看不到它指向什么样的结构体和内部字段。所有操作通过工厂函数和 API 完成——你永远不会直接触碰内部数据。

> 💡 **RAII 是什么**？RAII = Resource Acquisition Is Initialization（资源获取即初始化）—— 这是 C++/Rust 中一种"在构造函数里分配资源、在析构函数里自动释放"的模式。C 语言没有构造函数/析构函数，但可以用宏和 `goto` 来**模拟**类似效果：离开作用域时自动 cleanup。

## 本章适合谁

- 已掌握 void\* 泛型编程和手动内存管理
- 正在编写 C 语言库或模块, 需要隐藏内部实现
- 好奇 C 语言能否模拟 RAII (资源获取即初始化) 模式
- 想用 C 实现工厂模式 (Factory Pattern) 的开发者

## 你会学到什么

1. 不透明指针 (Opaque Pointer) 的完整实现方法
2. 工厂模式 (Factory Pattern) —— `create → use → destroy` 三步走
3. RAII-style 宏 —— C 语言模拟自动资源管理
4. void\* 通用容器的设计与实现
5. 公开结构体 vs 不透明结构体的 ABI 兼容性
6. 信息隐藏 (Information Hiding) 的核心价值

## 前置要求

- 已掌握 [void\* 泛型编程](../basic/void_generic.md) —— 类型擦除
- 已掌握 [内存管理](../basic/memory_mgmt.md) —— malloc/free 基础
- 理解 [头文件与模块系统](../basic/headers.md) —— 接口与实现分离

## 第一个例子

```c
/* ---- 头文件 (public API) ---- */
typedef struct MyBuffer MyBuffer;  /* 不完整类型! */

MyBuffer *mybuffer_create(void);
void      mybuffer_destroy(MyBuffer *buf);
int       mybuffer_push(MyBuffer *buf, uint8_t byte);

/* ---- main.c ---- */
MyBuffer *buf = mybuffer_create();
mybuffer_push(buf, 42);
mybuffer_destroy(buf);
/* buf 内部结构不可见 */
```

这段代码中, `main.c` **只能**调用 `mybuffer_create/destroy/push` 三个函数。
`MyBuffer` 结构体的定义在 `.c` 文件中, 调用者无法 `buf->len = 0`——甚至连 `sizeof(MyBuffer)` 都不知道。

这就是信息隐藏 (Information Hiding) 的力量: 接口稳定, 实现可以任意修改。

> 📌 **回顾之前学的**: 信息隐藏（Information Hiding）——通过 `typedef struct X X;` 声明不完整类型，迫使调用者只能通过公开 API 访问，无法直接修改内部字段。详见 [void* 泛型编程](../basic/void_generic.md) 和 [头文件与模块系统](../basic/headers.md)。

## 原理解析

### 1. 内存泄漏 — 原始指针的问题 (Error-First)

```c
/* ❌ 典型泄漏 */
void buggy_function(void) {
    char *buf = malloc(256);
    if (buf == NULL) return;
    strncpy(buf, "data", 255);
    /* ... 使用 buf ... */
    /* 忘记 free(buf)! 每次调用泄漏 256 字节 */
}
```

**修复**: 每条执行路径必须有 free:

```c
/* ✅ 配对 free */
void safe_function(void) {
    char *buf = malloc(256);
    if (buf == NULL) return;
    /* ... */
    free(buf);
    buf = NULL;
}
```

核心问题: C 语言**不会自动释放堆内存**。每个 `malloc` 都必须配对一个 `free`。

### 2. 不透明指针 — 隐藏实现

C 语言实现信息隐藏的套路:

```c
/* ---- public.h (用户可见的头文件) ---- */

/* 不完整类型 (incomplete type) — 只声明, 不定义结构体 */
typedef struct Database Database;

/* 工厂函数: 创建 */
Database *database_create(const char *path);

/* 操作函数: 只能通过 API 访问 */
int database_insert(Database *db, const char *key, const char *value);

/* 清理函数: 销毁 */
void database_destroy(Database *db);

/* ---- database.c (内部实现, 用户看不到) ---- */

/* 真实结构体定义: 只有 .c 文件可见 */
struct Database {
    char   path[512];
    int    fd;
    char **keys;
    char **values;
    size_t count;
    size_t capacity;
};

Database *database_create(const char *path) {
    Database *db = malloc(sizeof(Database));
    if (db == NULL) return NULL;
    strncpy(db->path, path, sizeof(db->path) - 1);
    db->path[sizeof(db->path) - 1] = '\0';
    db->count = 0;
    db->capacity = 64;
    db->keys = calloc(db->capacity, sizeof(char *));
    db->values = calloc(db->capacity, sizeof(char *));
    return db;
}

void database_destroy(Database *db) {
    if (db == NULL) return;
    for (size_t i = 0; i < db->count; i++) {
        free(db->keys[i]);
        free(db->values[i]);
    }
    free(db->keys);
    free(db->values);
    free(db);
}
```

**调用者视角**:
```c
Database *db = database_create("mydb.dat");  /* ✅ */
database_insert(db, "name", "Alice");       /* ✅ */
database_destroy(db);                        /* ✅ */

/* db->count = 0; */  /* ❌ 编译错误! 看不到结构体 */
/* sizeof(Database); */ /* ❌ 编译错误! incomplete type */
```

### 3. 工厂模式 (Factory Pattern)

工厂模式的标准三部曲:

```
┌───── create ───────┐  ┌──── use ──────┐  ┌── destroy ──┐
│                    │  │               │  │             │
v                    v  v               v  v             v
NULL ──→ 有效指针 ──→ 调用 getter ──→ 调用 setter ──→ NULL
  ^                    │              │            │
  └────────────────────┴──────────────┴────────────┘
       (内部状态被隐藏, 调用者只能通过函数操作)
```

```c
/* Step 1: create */
MyObj *obj = myobj_create(arg1, arg2);
if (obj == NULL) {
    /* 处理创建失败 */
    return -1;
}

/* Step 2: use (只能通过 API) */
myobj_set_something(obj, value);
int result = myobj_get_something(obj);

/* Step 3: destroy (清理所有资源) */
myobj_destroy(obj);
obj = NULL;  /* 防止悬垂指针 */
```

### 4. RAII-style 宏 — C 语言的自动资源管理

RAII (Resource Acquisition Is Initialization) 是 C++/Rust 的自动资源管理模式。在 C 中, 可以用 `for` 循环模拟:

```c
/* 文件 RAII 宏 */
#define WITH_FILE(fp, path, mode) \
    for (FILE *fp = fopen(path, mode); \
         fp != NULL; \
         fclose(fp), fp = NULL)

/* 使用方式 */
WITH_FILE(f, "data.txt", "r") {
    char buf[256];
    while (fgets(buf, sizeof(buf), f)) {
        printf("%s", buf);
    }
}  /* ← for 循环结束, 执行 fclose(f), f = NULL */
```

**工作原理**: `for` 循环只执行一次 (第二次检查 `fp != NULL` 为假时退出), 退出时执行第三个表达式 `fclose(fp), fp = NULL`:

```
初始化: FILE *fp = fopen(...)  → fp = 文件句柄
检查:   fp != NULL              → true, 进入循环
循环体: { ... 使用 fp ... }
迭代:   fclose(fp), fp = NULL   → 自动关闭文件!
检查:   fp != NULL              → false, 退出循环
```

**扩展: 内存 RAII**:
```c
#define WITH_MALLOC(ptr, type, count) \
    for (type *ptr = calloc(count, sizeof(type)); \
         ptr != NULL; \
         (free(ptr), ptr = NULL))

WITH_MALLOC(arr, int32_t, 10) {
    arr[0] = 100;
    /* ... */
}  /* ← 自动 free(arr) */
```

### 5. void\* 通用容器 — 类型擦除 + 回调

不透明指针 + void\* 可以构建泛型容器:

```c
typedef struct {
    void **items;
    size_t count;
    void (*free_item)(void *);  /* 自定义清理函数 */
} GenericArray;

GenericArray *arr = generic_array_create(10, free);
int *p = malloc(sizeof(int));
*p = 42;
generic_array_push(arr, p);
/* ... */
generic_array_destroy(arr);
/* destroy 内部调用:
   for (i) arr->free_item(arr->items[i]);  // free 每个元素
   free(arr->items);                        // free 数组本身
   free(arr);                               // free 容器
*/
```

### 6. 公开 vs 不透明 — ABI 兼容性

| 场景 | 公开结构体 | 不透明结构体 |
|------|-----------|-------------|
| 调用者能看到字段 | ✅ `obj->x = 5` | ❌ 编译错误 |
| 修改内部结构 | 调用者代码全要改 | 调用者代码不变 |
| 二进制兼容 (ABI) | ❌ 改 struct = 新编译 | ✅ 改内部不影响 API |
| 适用场景 | 数据传递 (POD 类型) | 库/模块的核心接口 |

> 「好的 API 设计不是「能做什么」, 而是「不能做什么」。限制调用者的操作, 才能保护你的不变量。」

## 常见错误

### 错误 1: 工厂创建后忘记 destroy

```c
/* ❌ 泄漏 */
MyObj *obj = myobj_create();
/* ... 使用 ... */
/* 忘了 myobj_destroy(obj)! */

/* ✅ 修复: 配对 create/destroy */
MyObj *obj = myobj_create();
/* ... */
myobj_destroy(obj);
obj = NULL;
```

### 错误 2: 在 for RAII 宏内部 return

```c
/* ❌ return 跳过 cleanup */
WITH_MALLOC(buf, char, 64) {
    if (condition) return;  /* ← return 跳过 for 的迭代表达式! */
    /* buf 泄漏! */
}

/* ✅ 修复: 用 goto 替代 return, 或不用 RAII 宏 */
```

**RAII 宏的限制**: 在 `for` 循环体内使用 `return`、`goto` 跳出循环会**跳过 cleanup**。只适用于**不会提前返回**的场景。

### 错误 3: 不完整类型声明错误

```c
/* ❌ 错误方式: 头文件中暴露部分结构 */
struct MyObj { int visible_field; };
typedef struct MyObj MyObj;

/* ✅ 正确方式: 完全隐藏 */
typedef struct MyObj MyObj;  /* 只有 typedef, 没有 struct 定义 */
```

### 错误 4: destroy 后不置 NULL

```c
/* ❌ 悬垂指针 */
MyObj *obj = myobj_create();
myobj_destroy(obj);
myobj_set_something(obj, val);  /* ❌ 使用已释放的指针 → UB */

/* ✅ 修复: destroy 后置 NULL */
myobj_destroy(obj);
obj = NULL;
/* myobj_set_something(obj, val); → 函数内会检查 NULL, 安全返回 */
```

## 动手练习

### 🟢 练习 1: 简单工厂模式

实现一个简单的 `Counter` 结构体, 包含 `int value`。提供 `counter_create()`、`counter_inc()`、`counter_get()`、`counter_destroy()` 四个函数。

<details><summary>点击查看答案</summary>

```c
/* counter.h */
typedef struct Counter Counter;
Counter *counter_create(void);
void     counter_inc(Counter *c);
int      counter_get(const Counter *c);
void     counter_destroy(Counter *c);

/* counter.c */
struct Counter {
    int value;
};

Counter *counter_create(void) {
    Counter *c = malloc(sizeof(Counter));
    if (c) c->value = 0;
    return c;
}

void counter_inc(Counter *c) {
    if (c) c->value++;
}

int counter_get(const Counter *c) {
    if (c) return c->value;
    return 0;
}

void counter_destroy(Counter *c) {
    free(c);
}
```

</details>

### 🟡 练习 2: RAII 文件读取宏

写一个 `WITH_FILE_READ(var, path)` 宏, 等价于 `for (FILE *var = fopen(path, "r"); var; fclose(var), var = NULL)`, 然后读取文件内容。

<details><summary>点击查看答案</summary>

```c
#define WITH_FILE_READ(var, path) \
    for (FILE *var = fopen(path, "r"); \
         var != NULL; \
         fclose(var), var = NULL)

void read_demo(void) {
    WITH_FILE_READ(f, "/tmp/test.txt") {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            printf("%s", line);
        }
    }  /* 自动 fclose */
}
```

</details>

### 🔴 练习 3: 引用计数不透明指针

实现 `Resource` 的引用计数: `resource_create()` 返回 ref=1 的资源。`resource_add_ref()` 增加引用, `resource_release()` 减少引用, ref=0 时真正 free。

<details><summary>点击查看答案</summary>

```c
typedef struct Resource Resource;

Resource *resource_create(void);
void      resource_add_ref(Resource *r);
void      resource_release(Resource *r);

struct Resource {
    int ref_count;
    char *data;
};

Resource *resource_create(void) {
    Resource *r = malloc(sizeof(Resource));
    if (r) {
        r->ref_count = 1;
        r->data = NULL;
    }
    return r;
}

void resource_add_ref(Resource *r) {
    if (r) r->ref_count++;
}

void resource_release(Resource *r) {
    if (r && --r->ref_count == 0) {
        free(r->data);
        free(r);
    }
}
```

</details>

## 故障排查 (FAQ)

**Q: 不透明指针能用在结构体内嵌吗？**

A: 不能直接内嵌。不透明指针的本质是「调用者不知道类型大小」, 所以只能作为**指针**传递。如果需要内嵌, 必须用「公开结构体」或把嵌套改为间接指针。

```c
typedef struct Inner Inner;  /* 不透明 */

/* ❌ 错误: 不能知道 struct Inner 的大小 */
struct Outer { Inner inner; };

/* ✅ 正确: 用指针 */
struct Outer { Inner *inner; };
```

**Q: RAII 宏真的安全吗？**

A: 基本安全但有边界情况:
- ✅ 在 `for` 循环体内正常执行: 自动 cleanup
- ❌ 在 `for` 循环体内 `return`: **跳过 cleanup** — 泄漏!
- ⚠️ 在 `for` 循环体内 `goto` 到外部: 同样跳过 cleanup
- ✅ 在 `for` 循环体内 `break`: 正常 cleanup (break 会执行迭代表达式)

**Q: 不透明指针比公开结构体慢吗？**

A: 不慢。**函数调用开销**在大多数情况下可以忽略。而且不透明指针允许你修改内部实现而不需要重新编译调用者的代码——这比微观优化重要得多。

**Q: C 语言有比 RAII 和工厂模式更好的资源管理方式吗？**

A: 对于简单场景, `goto cleanup` 模式也是一种有效选择:

```c
int process(void) {
    Resource *r1 = NULL;
    Resource *r2 = NULL;

    r1 = resource_create();
    if (r1 == NULL) goto cleanup;

    r2 = resource_create();
    if (r2 == NULL) goto cleanup;

    /* ... 使用 r1, r2 ... */

    resource_destroy(r2);
    r2 = NULL;

cleanup:
    resource_destroy(r1);  /* 保证释放 */
    return 0;
}
```

## 知识扩展 (选学)

### PIMPL 模式 (C++ 的私有实现)

C++ 中的 PIMPL (Pointer to Implementation) 与 C 的不透明指针本质相同:

```cpp
/* Widget.h */
class Widget {
private:
    struct Impl;  // 不完整类型
    Impl *pImpl;  // 不透明指针
};

/* Widget.cpp */
struct Widget::Impl {
    int secret_data;  // 完全隐藏
};
```

C 语言不透明指针 = PIMPL 的 C 版本。

### 双不透明指针 (Double Opaque)

有些 API 甚至隐藏了创建/销毁函数:

```c
/* 只暴露一个函数指针表 */
typedef struct {
    int (*add)(void *, int);
    void (*release)(void *);
} TableOps;

/* 调用者拿到 void* + ops 表, 完全不知道内部结构 */
void *obj = module_create_object(&ops);
ops.add(obj, 42);
ops.release(obj);
```

Go 语言接口、Rust trait object 本质上都是这种模式 (虚表 dispatch)。

### 常见标准库中的不透明指针

| 类型 | 来源 | 工厂函数 |
|------|------|----------|
| `FILE*` | stdio.h | `fopen()` → `fclose()` |
| `DIR*` | dirent.h | `opendir()` → `closedir()` |
| `sqlite3*` | sqlite3.h | `sqlite3_open()` → `sqlite3_close()` |
| `pthread_mutex_t*` | pthread.h | `pthread_mutex_init()` → `pthread_mutex_destroy()` |

## 小结

本章的核心要点:

- **不透明指针** — 头文件只 `typedef struct X X`, 不暴露内部结构, 实现信息隐藏
- **工厂模式** — 三部曲: `create` → `use` (通过 API) → `destroy`
- **RAII-style 宏** — 用 `for` 循环模拟自动 cleanup, 适合不会提前 return 的场景
- **void\* 通用容器** — 类型擦除 + 回调释放, 构建泛型数据结构
- **ABI 兼容性** — 不透明结构体修改内部不影响调用者, 公开结构体则会破坏 ABI
- **RAII 限制** — for 宏中 `return`/`goto` 会跳过 cleanup — 需要额外注意

> 「C 语言没有类的概念, 但有不透明指针。没有析构函数, 但有 destroy 模式。没有泛型, 但有 void\*。掌握这些模式, C 同样可以优雅和健壮。」

## 术语表

| 英文 | 中文 |
|------|------|
| Opaque pointer | 不透明指针 |
| Incomplete type | 不完整类型 |
| Information hiding | 信息隐藏 |
| Factory pattern | 工厂模式 |
| RAII | 资源获取即初始化 |
| ABI compatibility | 二进制接口兼容性 |
| Type erasure | 类型擦除 |
| Opaque pointer | 透明指针 (opaque = 不透明, 翻译习惯) |
| PIMPL (Pointer to Implementation) | 私有实现模式 |
| Cleanup callback | 清理回调函数 |

## 延伸阅读

- [cppreference: Incomplete types](https://en.cppreference.com/w/c/language/type#Incomplete_types) — C 语言中的不完整类型定义
- [Linux Kernel: include/linux/types.h](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/linux/types.h) — 内核中大量使用不透明指针 (如 `struct file*`)
- [SQLite API: sqlite3_open](https://sqlite.org/c3ref/sqlite3.html) — 标准库不透明指针实例
- [Bjarne Stroustrup: PIMPL idiom](https://www.stroustrup.com/pimpl.html) — C++ 的 PIMPL 模式与 C 不透明指针对比

## 继续学习

你已经掌握了 C 语言中高级结构设计的核心模式: 不透明指针、工厂模式、RAII 宏。这些是构建健壮 C 库的基础。

- [上一章](./atomic-types.md): 原子类型与内存序
- [下一章 →](./iterators.md): 数据结构遍历
- 前置基础: [void* 泛型编程](../basic/void_generic.md)

> 💡 **提示**: 检查你正在用或正在写的 C 代码 — 哪些地方可以用不透明指针来隐藏内部实现？有没有未配对的 create/destroy？RAII 宏能否简化资源管理？
