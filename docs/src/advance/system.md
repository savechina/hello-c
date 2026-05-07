# 系统调用 (System Calls) — 总览

> "操作系统是一个房东——它把钥匙（文件描述符）给你，把门铃（信号）装好，你可以直接开窗看水管（mmap），但如果你不敲门就闯进去，房东会毫不留情地请你在外面。"

## 开篇故事

想象你住在一个大型公寓楼里。大楼管理员（操作系统）管理着一切：水管、电线、门锁。

你不能直接改水管——你得先申请钥匙（打开文件描述符 `open`）。如果有紧急事件（比如火灾报警器响了），管理员会按你的门铃（POSIX 信号 `signal`），你必须放下手里的事去处理。如果你想查看水管布局，不需要跑到地下室——管理员允许你在墙上开窗（`mmap` 内存映射），直接看到水管的样子。甚至你还可以克隆一个自己去帮忙干活（`fork` 子进程），通过一根管子（`pipe`）和分身沟通。

## 本章简介

系统调用是你和操作系统之间的**直接对话**。不需要经过标准库的中间层——`open()` 直接触发系统调用，`read()` 直接和内核交互。掌握系统调用，你就掌握了 Unix/Linux 的"核武器"。

本章分为 **6 个子章节**，每个子章节聚焦一个特定领域，配有完整的源代码和文档。

## 子章节

| # | 子章节 | 难度 | 预计时间 | 链接 | 源代码 |
|---|--------|------|---------|------|--------|
| 1 | 文件与目录操作 | 🟡 | 45 min | [file](system/file.md) | `system_file_sample.c` |
| 2 | POSIX 信号处理 | 🔴 | 45 min | [signal](system/signal.md) | `system_signal_sample.c` |
| 3 | 内存映射 I/O | 🔴 | 45 min | [mmap](system/mmap.md) | `system_mmap_sample.c` |
| 4 | 进程管理 | 🔴 | 50 min | [process](system/process.md) | `system_process_sample.c` |
| 5 | 管道与 IPC | 🔴 | 50 min | [ipc](system/ipc.md) | `system_ipc_sample.c` |
| 6 | CLI 开发模式 | 🟡 | 35 min | [cli](system/cli.md) | `system_cli_sample.c` |

## 前置要求

- 掌握指针、文件 I/O (`fopen`/`fclose`)
- 理解 `errno` 错误码模式
- 了解进程和线程的基本概念
- 能运行 `make build && make run`

## 类比速查

| 概念 | 类比 |
|------|------|
| `open()/close()` | 拿钥匙/还钥匙 |
| `read()/write()` | 进出房间 |
| POSIX 信号 | 门铃 |
| `mmap()` | 在墙上开窗 |
| `fork()` | 细胞分裂（克隆自己） |
| `exec()` | 变身（变成另一个程序） |
| `pipe()` | 传声筒 |
| `socketpair()` | 专用电话 |

## 平台兼容性

所有子章节代码使用条件编译（`#ifdef __APPLE__`、`#ifdef __linux__`）确保跨平台兼容：
- **macOS**: 完全支持
- **Linux**: 完全支持
- **Solaris/FreeBSD**: 部分支持

## 编译和运行

```bash
make build    # 编译所有源文件
make run      # 运行所有系统调用演示
```

## 核心原则

1. **每个 open 配 close** — 文件描述符泄漏是常见 bug
2. **每个 fork 配 wait** — 不回收子进程 = 僵尸进程
3. **信号处理函数简单粗暴** — 只设置标志，不做复杂操作
4. **错误优先学习** — 先看错，再看怎么修

[← 上一章：异步与并发](./async.md) | [下一篇：文件与目录操作 →](system/file.md)
