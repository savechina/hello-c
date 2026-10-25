# Unix 环境与内核 (Unix & Kernel) 🔴

## 开篇故事

前一章的「系统编程」教会了你进程、信号、mmap——都是 POSIX 的通用面。但 Unix 家族
四个兄弟（macOS、Linux、FreeBSD、illumos）对**同一件事**给出了四个不同的答案：
事件通知用 kqueue 还是 epoll？挂载点怎么枚举？内核状态怎么从 userland 读取？

本章集群带你走进这些「同一问题、多套答案」的领域，并给出用户态 C 程序员
真正能落地的答案。想写内核驱动？先从 userland 内核接口走起。

## 本章章节

| 章节 | 难度 | 内容 |
|------|------|------|
| [Unix 环境编程](env.md) | 🟡 | environ、getopt、进程身份、passwd、sysconf |
| [多平台事件模型](event-models.md) | 🔴 | kqueue（macOS/FreeBSD）、epoll（Linux）、ports（illumos）、poll（兜底） |
| [VFS 与挂载点](vfs.md) | 🟡 | getmntinfo/getmntent/mnttab、statvfs |
| [内核接口探测](kernel-interface.md) | 🔴 | sysctl、kstat、/proc 与 /sys——驱动教学的 userland 替代 |
| [真实项目: 终端 TODO](todo-project.md) | 🟡 | 数据结构 CRUD + 二进制持久化 + 资源纪律 |

## 为什么没有驱动章节？

内核驱动（macOS DriverKit/KPI、FreeBSD kernel module、illumos driver）需要
内核态工具链与独立构建系统，无法在本仓「一键 make」编译运行——违背教程
「每章均可运行」的约定。因此我们用能运行的 [内核接口探测](kernel-interface.md)
替代，并在该章给出各自官方驱动教程的指引。

## 平台兼容性

本集群所有 sample 均支持 macOS / Linux / FreeBSD / illumos 四平台编译；
平台专属 API 均以 `#if defined(...)` 分叉，兜底路径保证「总有输出、退出码 0」。

> **验证状态**：macOS（clang `-Werror` + ASan）已实测；Linux / FreeBSD / illumos
> 分支按 API 文档人工编写，待 CI 矩阵落实后自动验证（见 kernel-interface.md 的驱动指引）。

## 下一步

← [Web 开发](../web.md) | [阶段复习](../review.md) →
