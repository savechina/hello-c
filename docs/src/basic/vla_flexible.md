# VLA 与柔性数组成员

```admonish note
**本章讲什么** — C99 变长数组(VLA):大小在**运行时**决定的栈数组;柔性数组成员(FAM):结构体末尾"多要一段内存"的堆上变长数组。错误先行:零长度 VLA 是 UB、VLA 不能写初始化列表、FAM 的 malloc 少算 header 直接堆溢出。
**对应源码** — [`src/basic/vla_flexible_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/vla_flexible_sample.c)
```

## 开篇故事

读入用户输入的 N 个整数再求和,你会怎么写?先 `malloc` 再循环?C99 其实给了更轻的选项 —— **变长数组(VLA)**:数组大小可以是一个运行时表达式,像普通局部变量一样在栈上自动分配、自动释放。而当数据天然长在结构体屁股后面(比如"头 + N 个元素"),**柔性数组成员**让一次 `malloc` 就装下整条记录。两者一个管栈、一个管堆,是"定长心智"之外必须掌握的两张牌。

## 本章适合谁

- 学过数组和结构体,想处理"运行时才知道大小"的数据
- 写过 `malloc(sizeof(T) + n * sizeof(...))` 但不确定对不对的人
- 准备读/写 C11/C17 代码库的读者(VLA/FAM 都是标准特性)

## 你会学到什么

1. VLA 的声明语法、`sizeof` 在运行时求值的含义、以及三条硬性约束
2. FAM 的声明规则与安全的 `malloc` 尺寸公式
3. VLA(栈)与 FAM(堆)的选型边界:什么时候用哪个

## 错例:三条不能踩的线

```c
/* ❌ 错例 (仅示意, UB 或编译错) */
int arr[0];            /* C17 起零长度数组非法; 0 长 VLA 是 UB */
int vla[n];            /* n 可能为 0 或负数 → UB, 必须运行时校验 */
int init_vla[n] = {0}; /* VLA 不能有初始化列表 → 约束违规 */
```

### 为什么错

1. **零长度 VLA 是未定义行为**:标准要求 VLA 大小为正
2. **VLA 不能带初始化列表**:编译期无法生成初始化循环
3. **VLA 在栈上**:n 太大会栈溢出 —— 示例里钳制 `n ≤ 64` 就是这个原因

## 正确写法一:VLA(栈上运行时大小)

```c
int vla_sum_n(int n)
{
    if (n < 1) n = 1;          /* 防 UB */
    if (n > 64) n = 64;        /* 防栈溢出 */

    int vla[64];               /* ✅ 固定上限声明 */
    for (int i = 0; i < n; i++) {
        vla[i] = i + 1;        /* 运行时填入 */
    }

    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += vla[i];
    }
    return sum;
}
```

`vla_sizeof_n()` 演示 `sizeof` 对 VLA 是**运行时求值**:

```c
int vla_sizeof_n(int n)
{
    /* ...同样钳制后... */
    int tmp[64];
    return (int)sizeof(tmp);   /* 返回 n * sizeof(int), 不是固定值 */
}
```

## 正确写法二:FAM(堆上一次 malloc)

```c
struct IntRecord {
    int count;
    int values[];   /* ✅ 柔性数组成员: 结构体最后一个成员, 无初始化 */
};

int fam_sum_n(int n)
{
    if (n < 1) n = 1;
    if (n > 64) n = 64;

    struct IntRecord *rec =
        malloc(sizeof(*rec) + (size_t)n * sizeof(rec->values[0]));
    if (rec == NULL) return -1;    /* 每一步 malloc 都要检查 */

    rec->count = n;
    for (int i = 0; i < n; i++) {
        rec->values[i] = i + 1;
    }

    int sum = 0;
    for (int i = 0; i < rec->count; i++) {
        sum += rec->values[i];
    }
    free(rec);                     /* 一次 free 整条记录 */
    return sum;
}
```

### malloc 尺寸公式

```text
sizeof(*rec)                      ← count 字段 + 对齐 padding
+ n * sizeof(rec->values[0])      ← n 个 int
```

漏掉 header、或者把 `sizeof(*rec)` 算成包含 0 元素 FAM 的大小再 `+ (n+1)` —— 都会算错。`offsetof(struct IntRecord, values)` 可以精确验证 header 到 values 的偏移。

## 原理解析

### VLA:FAM 的栈上表亲

| 维度 | VLA | FAM |
|------|-----|-----|
| 存储位置 | 栈 | 堆 |
| 生命周期 | 作用域结束自动释放 | 手动 `free` |
| 大小上限 | 栈空间(通常 1~8MB) | 可用内存 |
| 适合场景 | 小的临时缓冲(本章 ≤64) | 大的、生命周期长的记录 |

### 为什么 FAM 比"指针 + 两次 malloc"好

朴素写法是 `struct { int count; int *p; }` + 第二次 `malloc(n*sizeof(int))`:两次分配 = 两次碎片 + 两次 free + 两次 cache miss。FAM 把 header 和数据放在**同一块连续内存**,一次分配一次释放,CPU 缓存也友好。`smart_pointers_sample.c` 讲的 RAII 风格同样适用:FAM 的所有权规则是"一次 malloc,一次 free,谁分配谁释放"。

## 动手练习

### 🟢 入门:运行时求平均

读入 n(1≤n≤64),用 VLA 存 n 个整数并求平均(整数除法即可)。

<details><summary>点击查看答案</summary>

```c
int avg_of_n(int n)
{
    if (n < 1) n = 1;
    if (n > 64) n = 64;
    int tmp[64], sum = 0;
    for (int i = 0; i < n; i++) tmp[i] = i + 2;  /* 示例数据 */
    for (int i = 0; i < n; i++) sum += tmp[i];
    return sum / n;
}
```

</details>

### 🟡 中级:FAM 记录的克隆

写 `struct IntRecord *fam_clone(const struct IntRecord *src)`:一次 malloc 拷贝 header+数据,返回新指针。

<details><summary>点击查看答案</summary>

```c
struct IntRecord *fam_clone(const struct IntRecord *src)
{
    struct IntRecord *dst =
        malloc(sizeof(*dst) + (size_t)src->count * sizeof(dst->values[0]));
    if (dst == NULL) return NULL;
    dst->count = src->count;
    memcpy(dst->values, src->values,
           (size_t)src->count * sizeof(dst->values[0]));
    return dst;
}
```

</details>

### 🔴 挑战:offsetof 自检

用 `offsetof` 打印 `count` 和 `values` 的偏移,验证 `sizeof(*rec) - offsetof(struct IntRecord, values) == padding 字节数`。

<details><summary>点击查看答案</summary>

```c
printf("offsetof(count)  = %zu\n", offsetof(struct IntRecord, count));
printf("offsetof(values) = %zu\n", offsetof(struct IntRecord, values));
printf("header tail      = %zu\n", sizeof(struct IntRecord) - offsetof(struct IntRecord, values));
/* FAM 结构体的 sizeof 不包含 values, 只到 padding 结束 */
```

</details>

## 故障排查 (FAQ)

**Q:为什么我的 VLA 一跑就崩?**

A:大概率 n 超出栈上限。VLA 大而稀少;超过几 KB 就该用 `malloc`。

**Q:FAM 可以出现在结构体中间吗?**

A:不行。FAM 必须是**最后一个成员**,且结构体至少还有一个具名成员。C99/C11/C17 均如此。

**Q:`sizeof(struct IntRecord)` 算上 FAM 了吗?**

A:没有。`sizeof` 只到 padding 为止,FAM 部分是 0。这正是 `malloc(sizeof(*rec) + n * sizeof(...))` 公式成立的原因。

## 小结

- VLA:运行时大小的栈数组;**大小必须 > 0**、**不能有初始化列表**、**注意栈上限**
- FAM:`int values[]` 结构体尾部;一次 malloc 覆盖 header+数据;`sizeof` 不含 FAM
- 公式:`malloc(sizeof(*rec) + (size_t)n * sizeof(rec->values[0]))` + NULL 检查
- 选型:小而短命 → VLA;大而长命 → FAM

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 变长数组 | Variable-Length Array (VLA) | 大小运行时确定的栈数组(C99) |
| 柔性数组成员 | Flexible Array Member (FAM) | 结构体尾部零长数组成员(C99) |
| 栈溢出 | Stack overflow | 栈空间耗尽,通常直接崩溃 |
| 偏移 | Offset / offsetof | 成员相对结构体起始地址的字节数 |

## 延伸阅读

- [cppreference - VLA](https://en.cppreference.com/w/c/language/array#Variable-length_arrays)
- [cppreference - Flexible array member](https://en.cppreference.com/w/c/language/struct#Flexible_array_members)
- [C17 standard §6.7.6.2](https://port70.net/~nsz/c/c17/n1570.html#6.7.6.2)

## 继续学习

- [上一章](./arrays.md):数组
- [下一章](./enums.md):枚举

---

> 本章代码位于 [`src/basic/vla_flexible_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/vla_flexible_sample.c),测试位于 [`test/basic/test_vla_flexible_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_vla_flexible_sample.c)。
