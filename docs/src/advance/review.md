# 进阶阶段复习 (Advance Review)

## 开篇语

恭喜你读完了进阶篇！下面是 15 道综合题目，检验你是否真正掌握了 C 语言的高阶技巧。

### Q1 🟡 — 错误处理
`errno` 在什么情况下会被设置？如何正确检查？
<details><summary>查看答案</summary>
当系统调用或库函数失败时设置。正确做法：先清零 `errno = 0;`，调用函数，然后检查 `if (errno != 0)`
</details>

### Q2 🟡 — 原子类型
`atomic_int` 与 `volatile int` 的区别是什么？
<details><summary>查看答案</summary>
`atomic_int` 保证原子性和内存序（线程安全）。`volatile` 仅禁止编译器优化，不提供原子性。
</details>

### Q3 🔴 — 透明指针
什么是"Opaque Pointer"模式？
<details><summary>查看答案</summary>
头文件中声明结构体但不定义（`typedef struct MyObj MyObj;`），源文件中定义。用户只能通过 API 操作，无法访问内部数据。
</details>

### Q4 🔴 — 线程同步
pthread 中 mutex 和条件变量的区别？
<details><summary>查看答案</summary>
mutex 保护共享数据（互斥）。条件变量用于线程等待某个条件成立（信号机制）。
</details>

### Q5 🔴 — 数据结构
双向链表 vs 单向链表的优缺点？
<details><summary>查看答案</summary>
双向：可反向遍历、删除 O(1)，但多一个指针开销。单向：省内存，但只能前进、删除需遍历。
</details>

## 小结

答对 10+ 题说明你已经掌握进阶 C 编程！继续挑战高级项目吧。
