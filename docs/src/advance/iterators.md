# 数据结构遍历 (Data Structure Traversal & Iterators)

> "数据是安静的——它不会自己走来。你得拿着一个「小指针」，一步步走到它面前。"——我发现

---

## 开篇故事

想象你在读一本书。你不会一下子跳到第 50 页——你的手指放在当前页，`next()` 翻到下一页，`has_next()` 检查后面是否还有章节。这就是**迭代器 (Iterator)** 的本质：一个「当前位置」的记录器，告诉你下一步去哪、还有没有下一步。

```
书的迭代器                    C 链表的迭代器
┌─────────┐                  ┌──────┬──────┐
│ 第 1 页  │ ← 当前页          │ Data │ Next │──→ ┌──────┬──────┐
├─────────┤                   └──────┴──────┘      │ Data │ Next │──→ ...
│ has_next│ → 有                          └──────┴──────┘
│ next()  │ → 翻到第 2 页                    │ Data │ NULL │ ← 结尾
└─────────┘                                   └──────┴──────┘
```

C 语言没有像 Python `for x in list` 那样的魔法——你需要手动管理「手指」。本章教你如何在单向链表、双向链表、动态数组和二叉树中安全地遍历数据，以及如何在 C 中实现迭代器模式。

## 本章适合谁

- 写过 C 的数组，想学更灵活的数据结构
- 听说过「链表」「二叉树」但没亲手实现过
- 好奇 Python `for x in ...` 在 C 语言中如何实现
- 想了解「迭代器模式」这种设计模式

## 你会学到什么

1. **单向链表 (Singly Linked List)** —— 节点和 next 指针
2. **双向链表 (Doubly Linked List)** —— prev + next，可以从两边遍历
3. **动态数组 (Dynamic Array)** —— `realloc` 自动扩容
4. **二叉树遍历 (Binary Tree Traversal)** —— 前序、中序、后序
5. **迭代器模式 (Iterator Pattern)** —— 在 C 中封装遍历逻辑
6. **常见陷阱** —— 遍历时修改/删除节点导致的段错误

## 前置要求

- 已掌握：指针、结构体、`malloc`/`free`
- 已掌握：函数指针（用于回调和比较函数）
- 了解数组的基本概念

## 第一个例子：单向链表

最简单的链表——每个节点包含数据和指向下一个节点的指针：

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Node {
    int32_t data;
    struct Node *next;
} Node;

/* 创建新节点 */
static Node *node_new(int32_t value) {
    Node *n = malloc(sizeof(Node));
    if (n == NULL) return NULL;
    n->data = value;
    n->next = NULL;
    return n;
}

/* 遍历链表 */
static void list_print(Node *head) {
    for (Node *cur = head; cur != NULL; cur = cur->next) {
        printf("%d → ", (int)cur->data);
    }
    printf("NULL\n");
}

int main(void) {
    /* 创建链表 */
    Node *head = node_new(10);
    head->next = node_new(20);
    head->next->next = node_new(30);

    list_print(head);  /* 输出: 10 → 20 → 30 → NULL */

    /* 清空内存 */
    for (Node *cur = head; cur != NULL; ) {
        Node *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    return 0;
}
```

核心思想：**`cur = cur->next` 就是 `next()`；`cur != NULL` 就是 `has_next()`**。

## 原理解析

### 1. 单向链表 (Singly Linked List)

链表把数据分散在堆上，每个节点用 `next` 指针串起来：

```
  head
    │
    ▼
┌─────────┐    ┌─────────┐    ┌─────────┐
│ 10 │ →┼──→ │ 20 │ →┼──→ │ 30 │ NULL│
└─────────┘    └─────────┘    └─────────┘

特点:
- 添加节点: O(1)（已知前驱指针）
- 查找节点: O(n)（必须从头开始找）
- 删除节点: O(1)（已知前驱指针）
- 内存: 每个节点多一个指针的开销 (通常 8 字节)
```

**插入节点**：把新节点「插」到链中，关键是**先让新节点的 next 指向后面的节点，再让前面的节点指向新节点**——顺序不能错！

```c
/* 在给定节点后面插入 */
static void insert_after(Node *prev, Node *new_node) {
    if (prev == NULL || new_node == NULL) return;
    new_node->next = prev->next;  /* 新节点先连后面的 */
    prev->next = new_node;         /* 前面的再连新节点 */
}
```

如果顺序反过来——`prev->next = new_node` 先执行——`prev` 原来的后继就丢失了！后面的节点变成孤儿。

**删除节点**：同样需要前驱指针：

```c
/* 删除给定的节点（需要传入前驱） */
static void delete_after(Node *prev) {
    if (prev == NULL || prev->next == NULL) return;
    Node *to_remove = prev->next;
    prev->next = to_remove->next;  /* 跳过要删除的 */
    free(to_remove);                /* 释放内存 */
}
```

### 2. 双向链表 (Doubly Linked List)

每个节点有 `prev` 和 `next` 两个指针，可以前向和后向遍历：

```
  head                                     tail
    │                                        ▲
    ▼                                        │
┌──────┬──────┐    ┌──────┬──────┐    ┌──────┬──────┐
│ NULL │ →┼──→ │ ← │ →┼──→ │ ← │ →┼──→ │ ←│ NULL │
└──────┴──────┘    └──────┴──────┘    └──────┴──────┘
```

```c
typedef struct DNode {
    int32_t data;
    struct DNode *prev;
    struct DNode *next;
} DNode;
```

双向链表的优势：
- **可以从后往前遍历** —— `for (cur = tail; cur != NULL; cur = cur->prev)`
- **删除节点不需要前驱** —— 通过 `node->prev` 自己找到前驱
- 代价：每个节点多一个 `prev` 指针（多 8 字节开销）

### 3. 动态数组 (Dynamic Array)

动态数组 = 普通数组 + 自动扩容。当你加元素时，如果满了就换一块更大的内存：

```c
typedef struct {
    int32_t *data;
    size_t  size;     /* 当前元素数量 */
    size_t  capacity; /* 当前分配的容量 */
} DynArray;

/* 初始化：capacity = 4, size = 0 */
static void dynarray_init(DynArray *arr) {
    arr->capacity = 4;
    arr->size = 0;
    arr->data = malloc(arr->capacity * sizeof(int32_t));
}

/* 自动扩容核心逻辑 */
static void dynarray_push(DynArray *arr, int32_t value) {
    if (arr->size >= arr->capacity) {
        size_t new_cap = arr->capacity * 2;          /* 翻倍扩容 */
        int32_t *tmp = realloc(arr->data, new_cap * sizeof(int32_t));
        if (tmp == NULL) return;                      /* realloc 失败 */
        arr->data = tmp;
        arr->capacity = new_cap;
    }
    arr->data[arr->size++] = value;
}
```

扩容策略的关键：**每次翻倍**（而不是每次 +1）——这样摊还时间复杂度是 O(1)。

```
扩容过程:

  容量=4, 大小=4          容量=8, 大小=5
  ┌──┬──┬──┬──┐          ┌──┬──┬──┬──┬──┬──┬──┬──┐
  │10│20│30│40│          │10│20│30│40│50│  │  │  │
  └──┴──┴──┴──┘          └──┴──┴──┴──┴──┴──┴──┴──┘
    realloc 翻倍!           新元素追加到尾端
```

### 4. 二叉树遍历 (Binary Tree Traversal)

二叉树的每个节点最多有两个子节点：左子树和右子树。深度优先遍历有三种方式：

```
        1
       / \
      2   3
     / \   \
    4   5   6

前序 (Pre-order):   ROOT → LEFT → RIGHT   ⇒  1 2 4 5 3 6
中序 (In-order):    LEFT → ROOT → RIGHT   ⇒  4 2 5 1 3 6
后序 (Post-order):  LEFT → RIGHT → ROOT   ⇒  4 5 2 6 3 1
```

```c
typedef struct TreeNode {
    int32_t data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

static void preorder(TreeNode *node) {
    if (node == NULL) return;
    printf("%d ", (int)node->data);       /* 先访问根 */
    preorder(node->left);                  /* 再左 */
    preorder(node->right);                 /* 后右 */
}
```

**为什么递归能工作？** 因为递归调用天然利用函数调用栈（Call Stack）——你不需要自己管理一个栈，操作系统帮你做了。

### 5. 迭代器模式 (Iterator Pattern)

迭代器模式将「如何遍历」封装成三个函数：

```c
/* C 中的迭代器结构 */
typedef struct {
    const DynArray *arr;
    size_t  index;
} ArrayIterator;

static void iter_init(ArrayIterator *iter, const DynArray *arr) {
    iter->arr = arr;
    iter->index = 0;
}

static int iter_has_next(const ArrayIterator *iter) {
    return iter->index < iter->arr->size;
}

static int32_t iter_next(ArrayIterator *iter) {
    return iter->arr->data[iter->index++];
}
```

使用方式：
```c
ArrayIterator it;
iter_init(&it, &arr);
while (iter_has_next(&it)) {
    int32_t val = iter_next(&it);
    printf("%d ", (int)val);
}
```

这就像你读书时的「手指」——`index` 是手指的位置，`has_next()` 是检查后面有没有页，`next()` 是翻到下一页。

## 典型模式：安全遍历 + 删除节点

**这是最容易踩坑的地方。** 在遍历时删除节点，需要保存「下一个节点的指针」再删除当前节点：

```c
/* ✅ 正确的删除遍历时节点的方式 */
for (Node *cur = head, *next; cur != NULL; cur = next) {
    next = cur->next;   /* ← 先保存下一个！ */
    if (cur->data == 20) {
        /* 从链中移除 cur — 需要前驱指针 */
        free(cur);
    }
}
```

**错误做法**：
```c
/* ❌ 错误 —— cur 被 free 后 cur->next 变成野指针！ */
for (Node *cur = head; cur != NULL; cur = cur->next) {
    if (cur->data == 20) {
        free(cur);   /* ← 然后 cur->next 已经无效了 */
    }
}
```

这就是「遍历中修改/删除」的通用模式——先保存下一个再当前操作。

## 常见错误

### 错误 1：遍历时越过链表末尾

```c
/* ❌ 漏了 NULL 检查 — 段错误! */
Node *cur = head;
printf("%d\n", cur->data);   /* 如果 head 是 NULL 呢？ */

/* ✅ 始终在循环条件里检查 */
for (Node *cur = head; cur != NULL; cur = cur->next) {
    printf("%d ", (int)cur->data);
}
```

### 错误 2：free 后继续使用指针

```c
/* ❌ 已释放的指针不能再访问 */
Node *cur = head;
free(cur);
printf("%d\n", cur->data);  /* ❌ Use-after-free! 数据可能是垃圾 */

/* ✅ free 后立刻置 NULL */
free(cur);
cur = NULL;
```

### 错误 3：动态数组 realloc 后丢失原指针

```c
/* ❌ realloc 失败时原指针丢失 — 内存泄漏! */
arr->data = realloc(arr->data, new_size * sizeof(int32_t));  /* 如果失败，原 data 丢了 */

/* ✅ 用临时变量保存 realloc 结果 */
int32_t *tmp = realloc(arr->data, new_size * sizeof(int32_t));
if (tmp == NULL) { /* 处理错误，原 data 还在 */ }
arr->data = tmp;
```

### 错误 4：遍历时删除节点（不保存 next）

```c
/* ❌ cur 被 free 后不能再访问 cur->next */
for (Node *cur = head; cur != NULL; cur = cur->next) {
    if (needs_delete(cur)) {
        free(cur);  /* 下一轮循环 cur->next = ??? */
    }
}
```

## 动手练习

### 🟢 入门：链表求和

创建一个包含 {10, 20, 30, 40, 50} 的链表，遍历并计算所有节点的 `data` 之和。

<details><summary>点击查看答案</summary>

```c
int64_t sum = 0;
for (Node *cur = head; cur != NULL; cur = cur->next) {
    sum += cur->data;
}
printf("sum = %" PRId64 "\n", sum);  /* 150 */
```

</details>

### 🟡 中级：动态数组 + 迭代器

创建一个动态数组，push 10 个整数，然后用迭代器模式遍历打印。

<details><summary>点击查看答案</summary>

```c
DynArray arr;
dynarray_init(&arr);
for (int i = 0; i < 10; i++) {
    dynarray_push(&arr, (int32_t)(i + 1) * 10);
}

ArrayIterator it;
iter_init(&it, &arr);
printf("迭代器输出: ");
for (; iter_has_next(&it); ) {
    printf("%d ", (int)iter_next(&it));
}
printf("\n");

free(arr.data);
```

</details>

### 🔴 挑战：二叉树中序遍历（非递归）

用**栈**（不是递归）实现二叉树的中序遍历。

<details><summary>点击查看答案</summary>

```c
/* 用数组模拟栈 */
typedef struct {
    TreeNode *nodes[64];
    int top;
} Stack;

void inorder_iterative(TreeNode *root) {
    Stack stk = { .top = 0 };
    TreeNode *cur = root;

    while (cur != NULL || stk.top > 0) {
        /* 一直往左走，把路径上的节点入栈 */
        while (cur != NULL) {
            stk.nodes[stk.top++] = cur;
            cur = cur->left;
        }
        /* 出栈一个，访问，然后往右 */
        cur = stk.nodes[--stk.top];
        printf("%d ", (int)cur->data);
        cur = cur->right;
    }
}
```

</details>

## 故障排查 (FAQ)

**Q：链表和数组，什么时候用哪个？**

A：数组——适合随机访问（`arr[i]`，O(1)）和连续存储；链表——适合频繁插入/删除（O(1)）但不需要随机访问。动态数组是两者的折中。

**Q：`realloc` 失败会怎样？**

A：返回 `NULL`，但**原来的内存不会被释放**。所以用临时变量接收 realloc 结果很重要，否则原指针丢失 = 内存泄漏。

**Q：二叉树的前序、中序、后序有什么区别？**

A：区别在于「访问根节点」的时机：
- 前序：先访问根 → 复制树、生成前缀表达式
- 中序：中间访问根 → 二叉搜索树会得到有序序列
- 后序：最后访问根 → 删除树、生成后缀表达式

**Q：迭代器模式有什么意义？直接用 `for` 循环不行吗？**

A：迭代器封装了遍历的细节。对调用者来说，`iter_has_next()` + `iter_next()` 就是全部——不需要知道底层是链表、数组还是树。当数据结构变化时，调用者代码不需要改。

**Q：双向链表和单向链表怎么选？**

A：需要反向遍历时用双向链表；否则单向链表更省内存。大多数场景单向链表够用。

## 知识扩展 (选学)

### 跳表 (Skip List)

跳表是链表的「加速版」——在每个节点上增加「层」，高层指针跳过多个节点，实现 O(log n) 的平均查找：

```
高层:     ┌────────────────────────────┐
          │   ┌─────────────┐          │
中  层:   ├───┤   ┌────┐    ├───► ...
          │   │   │    │    │
低  层:   1───►2───►3───►4───►5───►6
```

### 哈希表 (Hash Table)

哈希表是 C 中最常用的查找数据结构——用 hash 函数把 key 映射到数组索引：

```c
typedef struct HashEntry {
    char *key;
    int32_t value;
    struct HashEntry *next;  /* 解决冲突：链地址法 */
} HashEntry;
```

### 泛型迭代器

可以用函数指针实现泛型的迭代器——遍历任意类型的数据：

```c
/* 回调函数：每遍历一个元素时调用 */
typedef void (*VisitFunc)(void *element, void *user_data);

void list_foreach(DNode *head, VisitFunc visit, void *user_data) {
    for (DNode *cur = head; cur != NULL; cur = cur->next) {
        visit(cur, user_data);
    }
}
```

## 小结

- **单向链表**：每个节点有一个 `next` 指针，O(n) 查找，O(1) 插入/删除
- **双向链表**：每个节点有 `prev` + `next`，支持双向遍历
- **动态数组**：`realloc` 自动扩容（翻倍策略），摊还 O(1) append
- **二叉树遍历**：前序(根→左→右)、中序(左→根→右)、后序(左→右→根)
- **迭代器模式**：`has_next()` + `next()` 封装遍历逻辑
- **安全删除**：遍历时删除节点——先保存 `next` 再操作

## 术语表

| 英文 | 中文 |
|------|------|
| Singly Linked List | 单向链表 |
| Doubly Linked List | 双向链表 |
| Dynamic Array | 动态数组 |
| Binary Tree | 二叉树 |
| Iterator Pattern | 迭代器模式 |
| Pre-order (Traversal) | 前序遍历 |
| In-order (Traversal) | 中序遍历 |
| Post-order (Traversal) | 后序遍历 |
| Node | 节点 |
| Head / Tail | 头 / 尾 |
| Capacity | 容量 |
| Amortized Complexity | 摊还时间复杂度 |
| Hash Table | 哈希表 |
| Skip List | 跳表 |

## 延伸阅读

- [cppreference - Linked Lists](https://en.wikipedia.org/wiki/Linked_list) — 链表结构完整参考
- [Binary Tree Traversals (Inorder, Preorder and Postorder)](https://www.geeksforgeeks.org/tree-traversals-inorder-preorder-and-postorder/) — 二叉树遍历图解
- [《算法导论》第 10-12 章](https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/) — 数据结构经典教材

## 继续学习

本章你理解了 C 语言中四种核心数据结构的遍历方式，以及迭代器模式的实现。这些数据结构是算法和系统设计的基础。下一步，你可以探索排序算法（利用遍历）、哈希表实现，或更高级的数据结构（红黑树、B 树）。

- [上一章](./async.md)：异步与线程
- [下一章](./data_structures.md)：高级数据结构
