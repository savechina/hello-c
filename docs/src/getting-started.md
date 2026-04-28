# Getting Started

## 安装 C 编译器

首先，你需要安装一个 C 编译器。hello-c 项目支持 **GCC**（Linux/Solaris）和 **Clang**（macOS/FreeBSD）。

### macOS

macOS 通常预装了 Clang。如果没有，可以通过 Xcode Command Line Tools 安装：

```bash
$ xcode-select --install
```

也可以通过 Homebrew 安装 GCC：

```bash
$ brew install gcc
```

验证安装：

```bash
$ clang --version    # 或 gcc --version
```

### Linux

大多数 Linux 发行版都自带 GCC。如果没有，可以使用包管理器安装：

**Ubuntu/Debian:**
```bash
$ sudo apt-get install build-essential
```

**Fedora/Red Hat:**
```bash
$ sudo dnf groupinstall "Development Tools"
```

**Arch Linux:**
```bash
$ sudo pacman -S gcc
```

验证安装：

```bash
$ gcc --version
```

### Windows

推荐使用 MSYS2 + MinGW-w64 或 WSL（Windows Subsystem for Linux）：

**MSYS2:**
```bash
# 安装 MSYS2 后，在 MSYS2 终端中运行：
$ pacman -S mingw-w64-x86_64-gcc
```

**WSL:**
```bash
# 在 WSL 中安装 Linux 发行版（如 Ubuntu）后：
$ sudo apt-get install build-essential
```

## 安装构建工具

hello-c 使用 **GNU Make** 作为构建系统。大多数系统已预装 `make`。

验证安装：

```bash
$ make --version
```

如果没有安装：
- macOS: `$ brew install make`
- Ubuntu: `$ sudo apt-get install make`
- Fedora: 已包含在 `build-essential` 中

## 克隆项目

```bash
$ git clone https://github.com/your-org/hello-c.git
$ cd hello-c
```

你应该会看到以下目录结构：

```
hello-c/
├── Makefile              # 构建脚本
├── include/              # 公共头文件
├── src/                  # 源代码
│   ├── main.c            # 程序入口
│   ├── basic/            # 基础教程代码
│   ├── advance/          # 进阶教程代码
│   └── ...
├── docs/                 # mdBook 文档
│   └── src/              # 文档源文件
└── build/                # 编译输出（git 忽略）
```

## 编译与运行

### 编译项目

```bash
$ make build
```

这将使用 `gcc -Wall -Wextra -Werror -std=c17`（或 clang 等价选项）编译所有 `src/` 下的 `.c` 文件，输出到 `build/bin/hello`。

### 运行教程

```bash
$ make run
```

这将编译并运行 `hello` 程序，依次执行基础入门教程的所有章节。

### 清理构建

```bash
$ make clean
```

删除 `build/` 目录中的所有编译产物。

### 查看帮助

```bash
$ make help
```

显示所有可用的 Make 目标和配置信息。

## 阅读文档

教程文档由 **mdBook** 生成。你可以本地预览：

```bash
$ cd docs
$ mdbook serve --open
```

这将在本地启动一个文档服务器，并自动在浏览器中打开。当你编辑文档时，页面会自动刷新。

安装 mdBook：
```bash
$ cargo install mdbook
# 或使用预编译版本
$ brew install mdbook  # macOS
```

## 编辑器推荐

- **VS Code**: 免费的 C/C++ 插件提供语法高亮、智能补全和调试
- **Neovim/Vim**: 轻量级，配合 LSP 插件体验优秀
- **CLion**: JetBrains 出品，功能完善（付费）
- **Sublime Text**: 简洁快速，适合快速编辑

## 接下来

- 📖 阅读 [Introduction](introduction.md) — 了解 C 语言的特点
- 📖 阅读 [About Hello C](about-hello.md) — 了解这个项目的背景
- 📖 阅读 [基础入门](basic/basic_overview.md) — 开始你的 C 语言学习之旅
