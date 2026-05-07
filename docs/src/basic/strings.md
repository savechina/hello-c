# 字符串 (Strings)

## 开篇故事

想象你在读一本书，但书页上没有页码，也没有目录。你怎么知道这本书什么时候结束？作者在最后一页放了一个特殊符号，看到它，你就知道：故事到此为止。

C 语言的字符串就是这样工作的。它没有内置的"长度"字段，就是一块连续的 `char` 内存，用 `\0`（null 终止符）标记结尾。少了这个标记，字符串就会一直读下去，直到偶然撞见一个 0 字节，读出一堆毫不相关的随机数据。

```c
char greeting[] = "Hello, C!";
// 内存布局：['H','e','l','l','o',',',' ','C','!','\0']
// 编译器自动在末尾加上 '\0'
printf("长度: %zu\n", strlen(greeting));  // 9（不含 \0）
printf("大小: %zu\n", sizeof(greeting));  // 10（含 \0）
```

这就是为什么在 C 语言里处理字符串从来不是一件理所当然的事。空间够不够？`\0` 有没有写？边界有没有守住？每一步都要自己管。C 把控制权全部交给你，也把责任全部交给你。

## 你会学到什么

本章是字符串专题的入口。C 字符串涉及的内容很多，我们把它拆分成了四个子章节，按学习路径排列：

- [**字符串基础**](./string_basics.md) — 字符数组的声明、null terminator 的作用、`char[]` vs `char*` 的区别、ASCII 内存布局、`strlen` 和 `sizeof` 的差异
- [**字符串操作**](./string_operations.md) — 标准库核心函数：`strlen`、`strcpy`、`strcat`、`strcmp` 的用法和注意事项
- [**安全字符串**](./safe_strings.md) — 缓冲区溢出原理、`strncpy` 和 `snprintf` 的正确使用、边界检查、安全编码模式
- [**字符串处理**](./string_processing.md) — `strtok` 分隔、`strstr` 子串搜索、`strchr` 字符查找、字符串解析实战

## 继续学习

- 上一章：[const 正确性](./const_correctness.md)
- 下一章：[字符串基础](./string_basics.md)

---

> 本章完整示例代码位于 [`src/basic/strings_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/strings_sample.c)。
