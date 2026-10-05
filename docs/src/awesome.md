# 资源精选 (Awesome)

```admonish note
**本章讲什么** — `src/awesome/` 模块：项目预留的 C 学习资源精选章节，目前是一个最小模块骨架（scaffold），本章如实记录它现在的样子和扩展方式。
**对应源码** — [`src/awesome/awesome.c`](https://github.com/savechina/hello-c/blob/main/src/awesome/awesome.c)
```

## 开篇故事

每个成熟语言的生态都有自己的 "awesome 列表"——社区精选的学习资源集合。本项目为 C 语言也预留了这样一个位置：`src/awesome/`。

**需要先说清楚**：这个模块目前只是一块骨架，还没有收录任何资源条目。本章不会编造一份资源清单，而是讲清楚三件事——模块现在长什么样、怎么运行、以及如果你想往里加内容，该按什么约定做。

## 模块现状

整个模块只有两个文件，共 11 行代码。

头文件 `src/awesome/awesome.h`（完整内容）：

```c
#ifndef AWESOME_H
#define AWESOME_H
int main_awesome_sample(void);
#endif
```

源文件 `src/awesome/awesome.c`（完整内容）：

```c
#include <stdio.h>
#include "awesome.h"

int main_awesome_sample() {
    printf("Hello from awesome!\n");
    return 0;
}
```

和项目里所有模块一样，它遵循同一套约定：

- 对外暴露唯一入口 `main_awesome_sample(void)`（模块名在前、`_sample` 在后）
- 头文件用 `#ifndef AWESOME_H / #define AWESOME_H / #endif` 保护
- 系统头在前、项目头在后

## 如何运行

入口由 `src/main.c` 的 argv 分发调用（收到 `awesome` 子命令时）：

```bash
make build
./build/bin/hello awesome
```

输出：

```
Hello from awesome!
```

> 📌 也可以用 `./build/bin/hello list` 查看全部可用子命令——`awesome` 会出现在列表里（README 的项目模块表中，它与 `advance` 一样标注为 *coming soon*）。

## 如何扩展这个模块

想往资源精选里加内容时，按项目既有模式操作：

1. **资源写进 `awesome.c`** —— 把 `main_awesome_sample()` 里的占位输出替换成你的内容（例如按类别打印资源清单，或组织成结构体数组 + 循环输出）
2. **需要新函数就加 `static` 辅助函数** —— 和 `src/algo/algo.c` 的做法一致：算法逻辑各是一个 `static` 函数，入口统一调度
3. **声明留在 `awesome.h`** —— 只暴露 `main_awesome_sample`，内部实现不进头文件
4. **入口签名保持不变** —— `main.c` 只认 `int main_awesome_sample(void)`，改名会让分发失效

不需要改 `Makefile`：构建系统用 `**.c` 通配自动收集源文件，新文件加入 `src/awesome/` 即参与编译。

## 小结

- `src/awesome/` 是预留的 **C 学习资源精选**章节，当前为最小骨架：`awesome.c` + `awesome.h` 共 11 行
- 唯一入口 `main_awesome_sample(void)`，由 `./build/bin/hello awesome` 触发
- 扩展方式与项目其他模块完全一致：入口 + `static` 辅助函数 + 头文件保护，构建自动收录

**核心术语**：Scaffold（骨架代码）/ Entry Point（模块入口）/ Header Guard（头文件保护）

> 「骨架先立起来，内容慢慢填——这是教程仓库组织章节的常见做法：结构稳定了，扩展只是往固定的位置放东西。」

## 继续学习

- [算法示例](./algo/algo.md)：一个"填满了内容"的模块长什么样——入口调度 + 多个 `static` 函数的完整范例
- [头文件与模块](./basic/headers.md)：头文件保护、声明与定义分离的原理
- [项目结构与约定](./advance/tools.md)：模块入口命名规范与构建系统如何自动收录源文件
