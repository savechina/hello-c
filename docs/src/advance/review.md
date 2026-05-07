# 进阶阶段复习 (Advance Review)

## 开篇语

恭喜你读完了进阶篇！下面是 10 道综合题目，检验你是否真正掌握了 C 语言的高阶技巧。

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

### Q3 🔴 — 不透明指针
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

### Q6 🔴 — 线程池
为什么使用线程池而不是每次都 pthread_create？
<details><summary>查看答案</summary>
创建线程有开销（几微秒到几十微秒）。线程池复用已有线程，避免频繁创建/销毁，特别适合大量短任务场景。
</details>

### Q7 🟡 — I/O 多路复用
select() 和 epoll() 的主要区别是什么？
<details><summary>查看答案</summary>
select() 有 fd 数量限制（FD_SETSIZE），每次需遍历所有 fd（O(n)）。epoll() 无 fd 限制，仅返回就绪的 fd（O(1) 检测）。
</details>

### Q8 🟡 — 进程管理
fork() 之后父子进程共享哪些内容？不共享哪些内容？
<details><summary>查看答案</summary>
共享：代码段（只读）、打开的文件描述符。不共享：栈、堆（写时复制）、数据段（写时复制）。
</details>

### Q9 🔴 — 分发表（VTable）
如何用 C 语言实现类似 C++ 虚函数表的多态？
<details><summary>查看答案</summary>
在 struct 中嵌入函数指针数组（vtable），每个"子类"有各自的 vtable。通过 vtable 索引调用函数，运行时决定具体行为。
</details>

### Q10 🟡 — 系统调用
open()/read()/write() 与 fopen()/fread()/fwrite() 的区别是什么？
<details><summary>查看答案</summary>
open/read/write 是系统调用，直接操作文件描述符。fopen/fread/fwrite 是标准库函数，内部调用系统调用并增加缓冲层。
</details>

## 小结

答对 7+ 题说明你已经掌握进阶 C 编程！继续挑战高级项目吧。
