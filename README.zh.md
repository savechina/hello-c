# Hello C

[English](README.md) | 简体中文

[![Docs](https://img.shields.io/badge/docs-online-blue)](https://savechina.github.io/hello-c/)
[![License](https://img.shields.io/badge/license-Apache%202.0-green)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/savechina/hello-c?style=social)](https://github.com/savechina/hello-c)

[**Hello C 教程**](https://renyan.org/hello/c) | [**GitHub Pages**](https://savechina.github.io/hello-c/)

一份面向初学者的 C 语言编程教程，采用**错误优先学习法**和**中英双语**教学。从变量到内存管理，涵盖 27 个渐进式章节——所有代码均可运行，附带多平台系统信息演示和完整的测试套件。

## 📖 在线教程

- 🌐 **官方网站**：[renyan.org/hello/c](https://renyan.org/hello/c)
- 📚 **GitHub Pages**：[savechina.github.io/hello-c](https://savechina.github.io/hello-c/)

## ✨ 项目特点

- **🎯 错误优先学习** — 每个概念先展示错误写法，再看编译器报错，最后给出正确写法。通过"犯错"来理解规则存在的意义。
- **🌏 双语设计** — 中文讲解搭配英文代码术语和技术词汇。专为想要阅读真实 C 代码的中文学习者打造。
- **🧠 认知辅助** — 内存示意图直观展示栈/堆布局，Python/C 对比帮助有高阶语言背景的读者快速迁移，每章配有三级练习题。
- **🖥️ 多平台系统信息** — 单文件 [`sysinfo.c`](src/sysinfo.c) 通过 `#ifdef` 迷宫检测 macOS、Linux、Solaris 和 FreeBSD——条件编译的真实应用范例。
- **🧪 Unity + CMock 测试** — 每个模块都有对应的单元测试，使用 Unity 测试框架和 CMock 生成的模拟对象。
- **📚 mdBook 文档** — 完整教程以 [mdBook](https://rust-lang.github.io/mdBook/) 站点发布，自动部署到 GitHub Pages。
- **🔧 C17 严格编译** — 使用 `-std=c17 -Wall -Wextra -Werror`，从一开始就培养安全、可移植的 C 编程习惯。

## 🚀 快速开始

```bash
git clone https://github.com/savechina/hello-c.git
cd hello-c
make run     # 编译并按顺序运行所有教程章节
make test    # 编译并运行 Unity 测试
```

## 📦 模块介绍

### 基础篇 (27 章)

按难度分级的 C 语言核心语法与概念：

| 难度 | 章节 | 主题 |
|------|------|------|
| 🟢 简单 (8) | [变量](docs/src/basic/variables.md)、[数据类型](docs/src/basic/datatype.md)、[函数](docs/src/basic/functions.md)、[运算符](docs/src/basic/operators.md)、[数组](docs/src/basic/arrays.md)、[流程控制](docs/src/basic/control_flow.md)、[循环](docs/src/basic/loops.md)、[头文件与模块](docs/src/basic/headers.md) | 类型、函数、流程控制 |
| 🟡 中等 (7) | [指针](docs/src/basic/pointers.md)、[字符串](docs/src/basic/strings.md)、[结构体](docs/src/basic/structs.md)、[枚举](docs/src/basic/enums.md)、[作用域](docs/src/basic/scope.md)、[类型定义](docs/src/basic/typedef.md)、[联合体](docs/src/basic/unions.md) | 内存、数据结构、作用域 |
| 🔴 困难 (12) | [内存管理](docs/src/basic/memory_mgmt.md)、[文件 I/O](docs/src/basic/file_io.md)、[函数指针](docs/src/basic/function_pointers.md)、[递归](docs/src/basic/recursion.md)、[位运算](docs/src/basic/bit_ops.md)、[预处理器](docs/src/basic/preprocessor.md)、[条件编译](docs/src/basic/conditional_comp.md)、[命令行参数](docs/src/basic/cli_args.md)、[安全字符串](docs/src/basic/safe_strings.md)、[可变参数函数](docs/src/basic/variadic_functions.md)、[回调函数](docs/src/basic/callbacks.md)、[调试](docs/src/basic/debugging.md) | 高级 C 概念 |

**完整章节列表**：[basic_overview.md](docs/src/basic/basic_overview.md) · [C 术语表（46 条）](docs/src/basic/basic_glossary.md) · [基础复习（20 题）](docs/src/basic/review_basic.md)

### Sysinfo — 多平台系统信息

| 文件 | 内容 |
|------|------|
| [`sysinfo.c`](src/sysinfo.c) | 327 行代码通过 `#ifdef` 检测 macOS、Linux、Solaris、FreeBSD |
| [`sysinfo.h`](src/sysinfo.h) | 公开 API：`get_system_info()`、`main_sysinfo()` |

### 进阶篇

高级 C 语言主题 — *即将推出*。

### 算法篇

算法演示 — *即将推出*。

### 资源篇

精选 C 语言编程资源 — *即将推出*。

## 🛠️ 技术栈

| 类别 | 选型 |
|------|------|
| **语言标准** | C17 (`-std=c17`) |
| **编译器** | GCC 12+ / Clang 15+ |
| **构建工具** | GNU Make，out-of-tree 构建 (`build/obj/`, `build/bin/`) |
| **测试框架** | Unity v2.6.1 + CMock v2.6.0 |
| **文档** | mdBook → GitHub Pages |
| **支持平台** | macOS、Linux、Solaris、FreeBSD |

## 📋 项目结构

```
hello-c/
├── src/
│   ├── main.c              # 唯一入口
│   ├── hello.c/.h          # 演示协调器
│   ├── sysinfo.c/.h        # 多平台操作系统检测
│   ├── basic/              # 27 个教程章节 (_sample.c + _sample.h)
│   ├── advance/            # 进阶主题（占位）
│   ├── algo/               # 算法演示
│   └── awesome/            # 精选资源
├── test/
│   ├── vendor/             # Unity v2.6.1, CMock v2.6.0
│   ├── basic/              # 基础模块单元测试
│   └── advance/            # 进阶模块单元测试
├── include/                # 项目级头文件 (global.h)
├── docs/                   # mdBook 文档
├── Makefile                # GNU Make, C17, out-of-tree 构建
└── build/                  # 构建产物 (git 忽略)
```

## 📝 许可证

Apache 2.0 — 详见 [LICENSE](LICENSE)。

## 🔗 相关项目

- [hello-rust](https://github.com/savechina/hello-rust) — Rust 基础教程（结构参考）
- [hello-python](https://github.com/savechina/hello-python) — Python 基础教程
- [hello-go](https://github.com/savechina/hello-go) — Go 基础教程
- [hello-swift](https://github.com/savechina/hello-swift) — Swift 基础教程
- [hello-ruby](https://github.com/savechina/hello-ruby) — Ruby 基础教程
