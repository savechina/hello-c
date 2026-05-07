# HTTP 服务器 (Web Server with POSIX Sockets)

> "一个 HTTP 服务器就像一位电话接线员——铃声响了（客户端连接），接起来（accept），听对方说什么（recv），然后回答（send），最后挂断（close）。就这么简单——只是用的是协议，不是说话。"

## 开篇故事

想象你在一家老式酒店当接线员。客人拿起电话（**客户端连接**），你说"喂，你好"（**accept 连接**），客人说"我要一份早餐"（**发送 HTTP 请求**），你说"好的，稍等"然后端上早餐（**发送 HTTP 响应**），最后挂断电话（**关闭连接**）。

现代 Web 服务器——Nginx、Apache——原理和接线员一样。只是它们不打电话，而是用 TCP 协议。客人不再说话，而是发送一段特定格式的文本（HTTP 请求文本）。你也不用端早餐，而是用特定格式的文本回复（HTTP 响应）。

本章将用 C 语言从零实现一个最简 HTTP 1.0 服务器——不用任何框架，只用 POSIX sockets。

## 本章适合谁

- 用过浏览器访问网站，但好奇"服务器到底在幕后做了什么"的人
- 想过"如果我自己写一个 Web 服务器会是什么样"的人
- 学过 TCP/UDP 网络编程，想在 HTTP 层面实践的人
- 想理解 Nginx 等现代服务器底层原理的人

## 你会学到什么

- Socket 生命周期：`socket` → `bind` → `listen` → `accept` → `close`
- HTTP 请求格式：请求行 + 请求头 + 空行 + 可选 body
- HTTP 响应格式：状态行 + 响应头 + 空行 + body
- Content-Type 头部的作用
- 错误处理：bind 失败、端口冲突、socket 泄漏
- 用 curl 测试你的服务器

## 前置要求

- 理解文件 I/O（socket 也是 fd）
- 掌握 C 字符串操作（解析请求需要）
- 了解 TCP 连接的基本概念（三次握手）

## 第一个例子

```c
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(void) {
    /* 1. 创建 socket — 装电话 */
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    /* 2. 绑定地址 — 指定号码 */
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    bind(fd, (struct sockaddr *)&addr, sizeof(addr));

    /* 3. 监听 — 开始接听 */
    listen(fd, 5);

    /* 4. 接受连接 — 接电话 */
    int client = accept(fd, NULL, NULL);

    /* 5. 发送响应 — 给答案 */
    const char *resp = "HTTP/1.0 200 OK\r\n\r\nHello World!";
    send(client, resp, strlen(resp), 0);

    /* 6. 关闭 — 挂断 */
    close(client);
    close(fd);
    return 0;
}
```

六步走：**装电话 → 选号码 → 开始接听 → 接电话 → 给答案 → 挂断**。

## 原理解析

### 1. Socket 生命周期

```
  ┌─ socket()     创建一个套接字（拿到 fd）
  │
  ├─ setsockopt() 设置选项（如 SO_REUSEADDR）
  │
  ├─ bind()       绑定 IP 和端口
  │
  ├─ listen()     开始监听连接
  │
  ├─ accept()     等待并接受一个连接
  │
  ├─ recv()       读取客户端数据（HTTP 请求）
  │
  ├─ send()       发送数据给客户端（HTTP 响应）
  │
  └─ close()      关闭连接
```

**类比**：
- `socket()` = 去电信公司申请安装电话
- `bind()` = 分配一个电话号码（IP+Port）
- `listen()` = 让电话处于待接听状态
- `accept()` = 听到铃声，接起电话
- `recv()` = 听对方说话
- `send()` = 回应对方
- `close()` = 挂断电话

### 2. HTTP 请求格式

客户端发送的请求长这样：

```
GET /index.html HTTP/1.1\r\n     ← 请求行: METHOD PATH VERSION
Host: localhost:8080\r\n          ← 请求头: 键值对
User-Agent: curl/7.79.1\r\n       ← 请求头
\r\n                               ← 空行（结束标记）
[可选 body]                        ← POST 方法可能有 body
```

- **请求行**：`METHOD PATH VERSION`
  - METHOD: `GET`、`POST` 等
  - PATH: `/index.html`、`/`、`/api/data`
  - VERSION: `HTTP/1.0`、`HTTP/1.1`
- **请求头**：`Key: Value` 格式，每行一个
- **空行**: `\r\n` 表示头部结束，后面是 body

**我的理解**：HTTP 请求就是一封"信"。请求行是信封上的地址（我要去哪、什么方式），请求头是寄件人信息，空行之后是信的内容。

### 3. HTTP 响应格式

服务器回复的响应长这样：

```
HTTP/1.0 200 OK\r\n              ← 状态行: VERSION STATUS_CODE REASON
Content-Type: text/html\r\n      ← 响应头: 告诉客户端内容格式
Content-Length: 123\r\n          ← 响应头: 告诉客户端 body 长度
\r\n                               ← 空行（结束标记）
<!DOCTYPE html>...                ← 响应体: 实际内容
```

常见状态码：
| 状态码 | 含义 |
|--------|------|
| 200 | OK（成功） |
| 404 | Not Found（找不到资源） |
| 500 | Internal Server Error（服务器错误） |

### 4. Content-Type 头部

告诉浏览器（客户端）"我给你的东西是什么格式"：

| Content-Type | 用途 |
|--------------|------|
| `text/html; charset=utf-8` | HTML 网页 |
| `text/plain; charset=utf-8` | 纯文本 |
| `application/json` | JSON 数据 |
| `image/png` | PNG 图片 |
| `application/octet-stream` | 二进制文件（通用下载） |

**关键规则**：如果 Content-Type 不对，浏览器可能无法正确显示。返回 HTML 却标注 `text/plain`，浏览器会直接显示源码而不是渲染页面。

### 5. 错误处理

```c
/* bind 失败 — 端口已被占用 */
if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "bind: %s (端口可能已被占用)\n", strerror(errno));
    close(fd);
    exit(1);
}

/* SO_REUSEADDR — 防止 "Address already in use" */
int opt = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

**我的理解**：不设置 `SO_REUSEADDR` 时，服务器退出后端口可能占用几秒（TIME_WAIT 状态），重启会报错"Address already in use"。设了之后可以立即重用。

## 常见错误

### ❌ 错误 1: 忘记 close 客户端 socket

```c
int client = accept(server_fd, NULL, NULL);
send(client, response, len, 0);
// 忘记 close(client) → fd 泄漏
// 每次连接都泄漏一个 fd，最终系统耗尽
```

✅ **修复**：每个 `accept` 必须配对 `close`。

### ❌ 错误 2: 不检查 bind 返回值

```c
// ❌ bind 失败但没处理——后续 listen 也失败
bind(fd, (struct sockaddr *)&addr, sizeof(addr));
listen(fd, 10);

// ✅ 检查并打印错误
if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(fd);
    exit(1);
}
```

### ❌ 错误 3: 响应缺少空行

```c
// ❌ 没有 \r\n 分隔头部和 body
const char *resp = "HTTP/1.0 200 OK\r\nContent-Length: 5Hello!";
// 客户端不知道 body 从哪开始

// ✅ 正确格式：头部和 body 之间必须有 \r\n
const char *resp = "HTTP/1.0 200 OK\r\nContent-Length: 5\r\n\r\nHello!";
```

### ❌ 错误 4: 字符串拼接构造响应

```c
// ❌ 用 sprintf 拼接响应（缓冲区溢出风险）
char resp[256];
sprintf(resp, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\n\r\n%s",
        strlen(body), body);
// 如果 body 很长？sprintf 不检查边界！

// ✅ 用 snprintf
snprintf(resp, sizeof(resp), "HTTP/1.0 200 OK\r\nContent-Length: %zu\r\n\r\n%s",
         strlen(body), body);
```

## 动手练习

### 🟢 练习 1: 最小 HTTP 服务器

写一个最小服务器，只用 `socket() → bind() → listen() → accept() → send() → close()`，返回 `Hello World!`。用 `curl http://localhost:8080` 测试。

<details>
<summary>点击查看答案</summary>

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(8080)};
addr.sin_addr.s_addr = htonl(INADDR_ANY);
bind(fd, (struct sockaddr *)&addr, sizeof(addr));
listen(fd, 1);
int client = accept(fd, NULL, NULL);
const char *resp = "HTTP/1.0 200 OK\r\n\r\nHello World!";
send(client, resp, strlen(resp), 0);
close(client);
close(fd);
```
</details>

### 🟡 练习 2: 解析请求路径

读取客户端请求第一行，提取 PATH，根据不同路径返回不同内容：
- `/` → "首页"
- `/hello` → "你好世界"
- 其他 → "404 Not Found"

<details>
<summary>点击查看答案</summary>

```c
char buf[1024];
recv(client, buf, sizeof(buf) - 1, 0);
buf[sizeof(buf) - 1] = '\0';

char method[16], path[256], version[16];
sscanf(buf, "%15s %255s %15s", method, path, version);

const char *body;
if (strcmp(path, "/") == 0) {
    body = "<h1>首页</h1>";
} else if (strcmp(path, "/hello") == 0) {
    body = "你好世界!";
} else {
    body = "404 Not Found";
}

char resp[2048];
snprintf(resp, sizeof(resp),
    "HTTP/1.0 200 OK\r\nContent-Length: %zu\r\n\r\n%s",
    strlen(body), body);
send(client, resp, strlen(resp), 0);
```
</details>

### 🔴 练习 3: 返回真实的 HTML 页面

让服务器读取 `index.html` 文件内容，设置 `Content-Type: text/html`，发送给客户端。在浏览器中访问 `http://localhost:8080` 看到渲染后的页面。

<details>
<summary>点击查看答案</summary>

```c
FILE *fp = fopen("index.html", "r");
if (fp == NULL) {
    const char *err = "404 File not found";
    char resp[1024];
    snprintf(resp, sizeof(resp),
        "HTTP/1.0 404 Not Found\r\n\r\n%s", err);
    send(client, resp, strlen(resp), 0);
} else {
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *html = malloc((size_t)size + 1);
    fread(html, 1, (size_t)size, fp);
    html[size] = '\0';
    fclose(fp);

    char header[256];
    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n",
        size);
    send(client, header, (size_t)hlen, 0);
    send(client, html, (size_t)size, 0);
    free(html);
}
```
</details>

## 故障排查

### Q: bind 返回 "Address already in use"

端口正在被另一个程序占用。解决方案：

1. 换端口：`htons(8081)`
2. 查占用进程：`lsof -i :8080`，然后 `kill` 占用进程
3. 加 `SO_REUSEADDR`：让系统可以立刻重用端口

### Q: curl 连接后超时

服务器没有发送 `\r\n\r\n`（头部结束标记），curl 一直等待头部结束。

**检查**：响应字符串里确保有 `\r\n\r\n` 分隔头部和 body。

### Q: 浏览器显示源码而不是渲染页面

Content-Type 设置错误。应该设为 `text/html`：

```c
"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html>...</html>"
```

### Q: accept 后客户端没数据

`recv()` 返回 0 或 -1。0 表示客户端已关闭连接，-1 检查 `errno`。

## 知识扩展

### 1. 现代服务器如何同时处理多个连接

本章是"一次只处理一个连接"。实际服务器用多进程、线程池或 I/O 多路复用：

- **多进程**：`fork()` 每个连接一个进程（Apache 传统模式）
- **I/O 多路复用**：`epoll` (Linux) / `kqueue` (macOS) 单线程管理大量连接（Nginx 模式）

### 2. HTTP/1.1 vs HTTP/1.0

| 特性 | HTTP/1.0 | HTTP/1.1 |
|------|----------|----------|
| 长连接 | ❌ 每个请求关闭 | ✅ Keep-Alive |
| Host 头部 | 可选 | 必需 |
| 分块传输 | ❌ | ✅ chunked |

### 3. 安全考虑

本章是教学用的裸 socket 服务器。生产环境需要：
- HTTPS（TLS 加密）
- 请求大小限制（防止 DoS）
- 超时机制（防止慢速攻击）

## 小结

- **Socket 六步**：`socket → bind → listen → accept → send → close`
- **HTTP 请求** = 请求行（METHOD PATH VERSION）+ 请求头 + 空行 + body
- **HTTP 响应** = 状态行（VERSION CODE REASON）+ 响应头 + 空行 + body
- **Content-Type** 告诉客户端内容格式，必须设对
- **错误处理**：bind 失败、socket 泄漏、响应格式错误

> **我的教训是**：第一次写 HTTP 服务器时，我忘记在响应头部和 body 之间加 `\r\n\r\n`，导致浏览器一直转圈。记住：HTTP 协议的格式要求极其严格——`\r\n` 换行、`\r\n\r\n` 分隔头部和 body，一个都不能少。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| Socket（套接字） | 网络通信的端点，用 fd 表示 |
| 绑定（Bind） | 将 socket 绑定到特定 IP 和端口 |
| 监听（Listen） | socket 开始等待入站连接 |
| 接受连接（Accept） | 接收一个客户端连接，返回新的 fd |
| 请求行（Request Line） | HTTP 请求的第一行 |
| 响应行（Status Line） | HTTP 响应的第一行 |
| Content-Type | 告知客户端响应体的内容格式 |
| Keep-Alive | HTTP/1.1 的连接复用机制 |
| 文件描述符泄漏（FD Leak） | 打开 socket 但忘记 close |
| `SO_REUSEADDR` | 允许立即重用端口 |

## 延伸阅读

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — socket 编程圣经
- [HTTP 规范 (RFC 7230)](https://tools.ietf.org/html/rfc7230) — 正式规范
- [curl 命令大全](https://everything.curl.dev/) — 调试 HTTP 服务器利器
- [《TCP/IP 详解 卷1》](https://zh.wikipedia.org/wiki/TCP/IP详解) — 网络底层原理

## 继续学习

你已经从零实现了一个 HTTP 服务器——虽然极简，但它涵盖了所有核心概念。Socket、bind、listen、accept、recv、send、close——这些是网络编程的基石。

回到起点。你现在已经完成了 C 语言的两个最重要进阶领域：**数据库**（SQLite3）和**系统编程**（POSIX）。这两者分别代表了"数据的组织方式"和"与操作系统的交互方式"。

> 💡 **提示**：试一下运行代码和文档是完整的。

[← 上一章：系统调用](./system.md)

- 下一章：[阶段复习](./review.md)
