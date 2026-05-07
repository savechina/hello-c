# 高级多态：函数指针与分发表 (VTable Pattern)

如果你用过 Python 或 Java，你对 "多态" 的概念应该不会陌生 —— 同一个方法名在不同对象上表现不同。但 C 语言没有类、没有继承、更没有虚函数。那怎么办？

答案就在函数指针 —— C 程序员用了几十年的「手工虚表」模式。

## 开篇故事

想象一个万能遥控器。你按下「播放」键，它不知道自己在控制电视、DVD 还是音响 —— 它只是调用一个函数指针，这个指针在遥控器初始化时就指向了正确的设备控制函数。

```
遥控器 (Shape 接口) ──► function pointer ──► 电视/Circle/Rectangle？
                      运行时才决定
```

遥控器不知道它控制的是什么设备。它只知道每个设备都实现了相同的按钮（`area`、`perimeter`）。这就是多态 —— 同一接口，不同行为。

> "C 没有虚函数表？没关系，自己造一个。"

## 本章适合谁

- 已经掌握 [函数指针](../basic/function_pointers.md) 和 [回调函数](../basic/callbacks.md) 的 C 学习者
- 好奇 C 如何实现面向对象多态效果的系统程序员
- 阅读 Linux 内核代码时看到了 vtable 模式的开发者
- 想在 C 中实现插件化/可扩展架构的工程师

## 你会学到什么

1. 函数指针分发表 —— 用函数指针数组做运行时分发
2. VTable 结构体 —— 模拟 C++ 虚函数表的 Struct 模式
3. 接口模式 —— struct 函数指针 + void* data = "虚拟类"
4. Shape 接口实战 —— Circle、Rectangle、Triangle 的统一多态操作
5. 动态分发 —— 运行时切换 vtable，实现行为替换
6. Error-first：NULL 函数指针 guard 防止 segfault

## 前置要求

- 完成 [函数指针](../basic/function_pointers.md) 章节
- 完成 [回调函数](../basic/callbacks.md) 章节
- 理解 `void*` 的含义

## 第一个例子

```c
typedef int32_t (*op_func_t)(int32_t, int32_t);

static int32_t op_add(int32_t a, int32_t b) { return a + b; }
static int32_t op_sub(int32_t a, int32_t b) { return a - b; }
static int32_t op_mul(int32_t a, int32_t b) { return a * b; }

/* dispatch table: 函数指针数组 */
const op_func_t ops[] = { op_add, op_sub, op_mul };

int32_t x = 20, y = 7;
for (int i = 0; i < 3; i++) {
    printf("ops[%d] = %d\n", i, ops[i](x, y));
}
```

输出：
```
ops[0] = 27   (20 + 7)
ops[1] = 13   (20 - 7)
ops[2] = 140  (20 * 7)
```

这里 `ops` 就是一个 **dispatch table**（分发表）—— 一个函数指针的数组。通过数组索引，运行时决定调用哪个函数。

## 原理解析

### 1. Dispatch Table：函数指针数组

Dispatch table 是最简单的多态形式 —— 一张「查表」：

```
操作码 (opcode) → 查表 → 函数指针 → 调用

  0 → ops[0] → op_add(20, 7)  = 27
  1 → ops[1] → op_sub(20, 7)  = 13
  2 → ops[2] → op_mul(20, 7)  = 140
```

```c
typedef int32_t (*binary_op_t)(int32_t, int32_t);

static int32_t op_add(int32_t a, int32_t b) { return a + b; }
static int32_t op_sub(int32_t a, int32_t b) { return a - b; }
static int32_t op_mul(int32_t a, int32_t b) { return a * b; }

const binary_op_t dispatch[] = { op_add, op_sub, op_mul };

/* 通过查表调用 */
int32_t apply(int32_t op_code, int32_t a, int32_t b) {
    return dispatch[op_code](a, b);  /* dispatch[op_code] 是函数指针 */
}
```

**真实场景**：CPU 的 opcode 分发、正则引擎的指令分发、语言解释器的字节码执行 —— 全部用 dispatch table。

### 2. VTable-like Struct：虚函数表

C++ 的虚函数表其实就是一个 struct 里存着函数指针。我们可以手动实现：

```c
/* VTable 定义 */
typedef double (*shape_area_fn)(const void *self);
typedef double (*shape_perimeter_fn)(const void *self);

typedef struct {
    shape_area_fn     area;
    shape_perimeter_fn perimeter;
} ShapeVTable;

/* Shape 实例：vtable 指针 + 数据指针 */
typedef struct {
    ShapeVTable *vtable;
    void *data;
} Shape;
```

```
Shape 实例内存布局:

  Shape ┌──────────────┐
       │ vtable ──────┼──► ShapeVTable ┌───────────────┐
       │ data    ─────┼──► 具体数据     │ area = circle │
                       │                │ perimeter =   │
                       │                │   circle_peri │
                       └──────────────┘ └───────────────┘
```

关键洞察：**所有 Circle 实例共享同一个 `circle_vtable`**（静态分配）。数据 (`data`) 才是每个实例独有的。

### 3. Interface Pattern：struct + void*

把 VTable 和 Interface 组合起来，就是 C 的「虚拟类」：

```c
/* Interface 定义 */
typedef struct {
    TransformSetup    setup;
    TransformDispatch dispatch;
    TransformCleanup  cleanup;
} TransformInterface;

/* 使用 */
ScaleOffsetData so_data = { 0 };
iface.setup(&so_data);
double result = iface.dispatch(&so_data, 5.0);
iface.cleanup(&so_data);
```

这叫做 **Interface Pattern** —— 一组函数指针定义了「行为契约」，`void* data` 承载状态。调用者只通过接口函数操作数据，不关心具体实现。

**类比**：
- Python: `class` + `def method(self)`
- C++: `virtual` 方法
- Rust: `trait`
- Go: `interface`
- C: `struct { fp*... }; + void* data`

### 4. Shape 实战：Circle, Rectangle, Triangle

```
Python:                    C 手写 vtable:

class Shape:              typedef struct { ShapeVTable *vt; void *data; } Shape;
    def area(self): ...   ShapeVTable circle_vtable = {.area = circle_area, ...};
                          Circle c = {.base.vt = &circle_vtable, ...};

class Circle(Shape):
    def area(self): ...
```

```c
/* Circle 实现 */
typedef struct { double radius; } CircleData;

static double circle_area(const void *data) {
    const CircleData *d = data;
    return 3.14159 * d->radius * d->radius;
}

static ShapeVTable circle_vtable = {
    .area = circle_area,
    .perimeter = circle_perimeter,
};

/* 通过接口调用 */
Shape shape = { .vtable = &circle_vtable, .data = &circle_data };
double a = shape.vtable->area(shape.data);  /* 动态分发！ */
```

### 5. 动态 Dispatch：运行时切换 vtable

VTable 不一定要静态的。你可以运行时更换行为：

```c
DynamicFunc linear =   { "linear",   linear_eval };
DynamicFunc const42 =  { "const42",  const_eval };

printf("%s(3.0) = %.2f\n", linear.label, linear.evaluate(3.0));

/* 运行时切换 */
linear = const42;  /* 行为改变了！ */
printf("%s(3.0) = %.2f\n", linear.label, linear.evaluate(3.0));

/* 输出:
   linear(3.0) = 7.00
   const42(3.0) = 42.00
*/
```

这在插件系统、运行时配置切换、策略模式中非常常见。

### Memory 对比：C struct vtable vs Python 对象

```
C (手动 vtable):                    Python (自动 vtable):

  Shape (16 字节):                   Circle 对象 (100+ 字节):
  ┌─────────┐                         ┌────────────────┐
  │vt* (8B) │────────► 静态 vtable   │ob_refcnt (8B)  │
  │data* (8B)│───────► 实例数据      │ob_type (8B) ───┼──► type object
  └─────────┘                       │ radius (8B)     │  │ __dict__
  N 个实例共享 1 个 vtable           └────────────────┘  │ __method__
                                         ┌────────────────┤
                                         │area* (8B) ─────┘
                                         │peri* (8B)     │
                                         └────────────────┘
  C 的开销: 16B → 多态
  Python 的开销: 100+B + 对象元数据
```

## 常见错误

### ❌ 错误 1：NULL 函数指针 → segfault

```c
ShapeVTable *vt = NULL;
double a = vt->area(data);  /* ❌ Segmentation fault! */
```

编译器不报错（类型匹配），运行时空指针解引用 → SIGSEGV。

✅ **修复**: 每次通过接口调用前检查

```c
double shape_area(Shape *s) {
    if (s == NULL || s->vtable == NULL || s->vtable->area == NULL) {
        fprintf(stderr, "Error: NULL function pointer!\n");
        return 0.0;
    }
    return s->vtable->area(s->data);
}
```

> "在 C 里，函数指针可能是 NULL。调用前检查，就像过马路前看左右。"

### ❌ 错误 2：vtable 指针悬挂（dangling）

```c
void init_shape(Shape *s) {
    ShapeVTable local_vtable = {.area = my_area};
    s->vtable = &local_vtable;  /* ❌ 函数返回后 local_vtable 被销毁 */
}

/* 调用者 */
Shape s;
init_shape(&s);
s.vtable->area(s.data);  /* ❌ 使用悬挂指针 → UB */
```

✅ **修复**: vtable 必须是静态的或 heap 分配的

```c
static ShapeVTable my_vtable = {.area = my_area};  /* 静态: 程序生命周期内有效 */
s->vtable = &my_vtable;  /* ✅ 安全 */
```

### ❌ 错误 3：vtable 不匹配

```c
CircleData circle = { .radius = 5.0 };
Shape s = { .vtable = &rect_vtable, .data = &circle };  /* ❌ mismatch! */
double a = s.vtable->area(s.data);  /* 结果未定义 */
```

✅ **修复**: 初始化时严格配对

```c
Shape s = { .vtable = &circle_vtable, .data = &circle };  /* ✅ 正确配对 */
```

### ❌ 错误 4：忘记 NULL dispatch table 越界

```c
op_func_t dispatch[] = { op_add, op_sub };
int32_t result = dispatch[5](10, 3);  /* ❌ 越界访问 → UB */
```

✅ **修复**: 检查索引范围

```c
int32_t num_ops = sizeof(dispatch) / sizeof(dispatch[0]);
if (op_code >= 0 && op_code < num_ops) {
    result = dispatch[op_code](a, b);
}
```

## 动手练习

### 🟢 入门：扩展 dispatch table

扩展上面的 dispatch table，添加 `op_div` 和 `op_mod`，用 for 循环遍历调用所有 5 个操作。

<details><summary>点击查看答案</summary>

```c
static int32_t op_div(int32_t a, int32_t b) {
    return (b != 0) ? (a / b) : 0;
}

static int32_t op_mod(int32_t a, int32_t b) {
    return (b != 0) ? (a % b) : 0;
}

const binary_op_t dispatch[] = { op_add, op_sub, op_mul, op_div, op_mod };
int32_t num_ops = (int32_t)(sizeof(dispatch) / sizeof(dispatch[0]));

for (int32_t i = 0; i < num_ops; i++) {
    printf("ops[%d] = %d\n", i, dispatch[i](20, 7));
}
```

</details>

### 🟡 中级：实现 Shape 接口 (Triangle)

在 Circle 和 Rectangle 之外，实现 Triangle（海伦公式）并加入多态数组统一遍历。

<details><summary>点击查看答案</summary>

```c
typedef struct { double a, b, c; } TriData;

static double tri_area(const void *data) {
    const TriData *d = data;
    double s = (d->a + d->b + d->c) / 2.0;
    return sqrt(s * (s - d->a) * (s - d->b) * (s - d->c));
}

static double tri_perimeter(const void *data) {
    const TriData *d = data;
    return d->a + d->b + d->c;
}

static ShapeVTable tri_vtable = {
    .area = tri_area,
    .perimeter = tri_perimeter
};

TriData tri = { 3.0, 4.0, 5.0 };
Shape shapes[3];
shapes[2] = (Shape){ .vtable = &tri_vtable, .data = &tri };

for (int i = 0; i < 3; i++) {
    printf("area=%.4f perimeter=%.4f\n",
           shapes[i].vtable->area(shapes[i].data),
           shapes[i].vtable->perimeter(shapes[i].data));
}
```

</details>

### 🔴 挑战：策略模式 + NULL Guard

实现一个 `Calculator` 接口，支持运行时切换运算策略（add/mul/div），包含完整的 NULL guard 和错误报告。

<details><summary>点击查看答案</summary>

```c
typedef struct {
    const char *name;
    int32_t (*apply)(int32_t, int32_t);
} Strategy;

static int32_t str_add(int32_t a, int32_t b) { return a + b; }
static int32_t str_mul(int32_t a, int32_t b) { return a * b; }

static int32_t safe_apply(Strategy *st, int32_t a, int32_t b) {
    if (st == NULL) {
        fprintf(stderr, "Error: NULL strategy!\n");
        return -1;
    }
    if (st->apply == NULL) {
        fprintf(stderr, "Error: strategy '%s' has NULL apply!\n",
                st->name ? st->name : "(unnamed)");
        return -1;
    }
    return st->apply(a, b);
}

Strategy strategies[] = {
    { "add", str_add },
    { "mul", str_mul },
    { "broken", NULL }
};

for (int i = 0; i < 3; i++) {
    printf("strategies[%d] → %d\n", i, safe_apply(&strategies[i], 6, 7));
}

/* 输出:
   strategies[0] → 13
   strategies[1] → 42
   Error: strategy 'broken' has NULL apply!
   strategies[2] → -1
*/
```

</details>

## 故障排查 (FAQ)

**Q: C 的 VTable 和 C++ 的 vtable 一样吗？**

A: 原理相同 —— 都是「每实例存一个指向函数指针表的指针」。区别在于：C++ 编译器自动生成虚表和虚函数分发的代码；C 需要你手动写。效果一样，只是 C 需要更多「体力活」。

**Q: 每个实例都存 vtable 指针，内存浪费吗？**

A: 每个实例只存一个指针（8 字节），vtable 本身是静态共享的（一份）。如果你创建 1000 个 Circle，vtable 只有 1 份（约 32 字节），1000 个数据实例各 8 字节 vtable 指针。总开销很小。

**Q: 为什么 data 用 `void*` 而不是具体类型？**

A: `void*` 是 C 的泛型 —— 它可以指向任何类型。VTable 函数通过 `void*` 接收数据，再 cast 回具体类型。这是 C 实现泛型多态的标准模式。

**Q: 函数指针数组越界会怎样？**

A: Undefined Behavior。编译器可能不会警告（数组大小可能在编译时未知），运行时访问越界索引可能返回垃圾指针 → 任意代码执行。必须检查索引边界。

## 知识扩展

### Linux 内核的 VTable

Linux 内核中的 `struct file_operations` 就是 VTable：

```c
/* Linux kernel: fs.h */
struct file_operations {
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *);
    int (*mmap) (struct file *, struct vm_area_struct *);
    /* ... 更多操作 */
};

/* 每个设备驱动提供自己的 file_operations */
static struct file_operations mydev_fops = {
    .read  = mydev_read,
    .write = mydev_write,
    .mmap  = mydev_mmap,
};
```

VFS 层统一调用 `fop->read(file, buf, count, pos)`，具体执行哪个函数取决于你 `open()` 了哪个设备文件。

### Rust Trait Object vs C VTable

```rust
// Rust trait
trait Shape {
    fn area(&self) -> f64;
}

struct Circle { r: f64 }
impl Shape for Circle {
    fn area(&self) -> f64 { 3.14 * self.r * self.r }
}

// 动态分发
let s: &dyn Shape = &Circle { r: 5.0 };
println!("{}", s.area());  // 运行时 dispatch
```

C 的 VTable 就是 Rust `dyn Shape` 的手动等价实现 —— `vtable` 指针指向具体的方法实现。

## 小结

- **Dispatch Table** = 函数指针数组，通过索引查表调用
- **VTable-like Struct** = 手动实现虚函数表，每个实例 `vt* + data*`
- **Interface Pattern** = `struct { fp*... };` + `void* data` = C 的虚拟类
- **动态 Dispatch** = 运行时切换 vtable，改变行为
- **NULL Guard** = 调用前检查 `vtable` 和 `vtable->func` 是否为 NULL
- **共享 vtable** = 所有同类实例共享一份 vtable（静态分配）
- VTable 开销: 每实例 +1 指针（8B），每类 1 份 vtable（约 32-64B）

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 虚函数表 | Virtual Table (vtable) | 存函数指针的表，实现运行时多态 |
| 分发表 | Dispatch Table | 函数指针数组，通过索引选择函数 |
| 接口 | Interface | 一组函数指针定义的「行为契约」 |
| 动态分发 | Dynamic Dispatch | 运行时决定调哪个函数 |
| 策略模式 | Strategy Pattern | 运行时切换算法/行为 |
| void* 泛型 | Generic void* | 无类型指针，C 的泛型实现方式 |
| 悬挂指针 | Dangling Pointer | 指向已销毁内存的指针 |
| 未定义行为 | Undefined Behavior (UB) | 标准不规定结果的错误行为 |

## 延伸阅读

- [Linux 内核: struct file_operations](https://www.kernel.org/doc/html/latest/filesystems/vfs.html)
- [Linux Device Drivers: VFS and File Operations](https://lwn.net/Kernel/LDD3/)
- [Beej's Guide: C VTables](https://beej.us/guide/bgc/)
- [设计模式: Strategy Pattern](https://refactoring.guru/design-patterns/strategy)

## 继续学习

- [上一章](../basic/callbacks.md)：回调函数与多态（基础版）
- [下一章](./database.md)：数据库（SQLite3）
- 实践：用 VTable 模式实现一个简单的事件系统
