# Web 开发：原始 Socket 与 HTTP 解析

> "打电话前先买个手机，拨号前先得有号码。" —— C 语言的网络编程从 socket() 开始。

## 开篇故事

想象你开了一家电话客服中心。socket() 是买电话机, bind() 是办电话号码, listen() 是把电话开机设为待接状态, accept() 是拿起电话听筒说"喂"。

C 语言的原始 socket 编程就是这样一步步建立连接的。不像 Python 的 `requests.get(url)` 一行搞定，C 语言要求你理解电话线的每一根铜丝。

## 本章适合谁

- 想理解 HTTP 底层工作原理的开发者
- 完成了系统调用章节，想学习网络编程

## 你会学到什么

1. socket() 创建 TCP 连接
2. HTTP 请求的文本格式解析
3. 构造并发送 HTTP 响应
4. 错误处理（bind 失败、连接拒绝等）

## 前置要求

- 系统调用章节（文件描述符概念）
- 基础：结构体、指针

## 第一个例子

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    // 买个"电话机"
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket 创建失败");
        return 1;
    }
    printf("✅ 电话机已购买 (fd=%d)\n", sock_fd);

    // 配置号码
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    // 绑定号码（会失败如果端口被占用）
    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind 失败（端口可能被占用）");
        close(sock_fd);
        return 1;
    }
    printf("✅ 号码已绑定: 0.0.0.0:8080\n");

    close(sock_fd);
    return 0;
}
```

## 原理解析

### Socket 是什么？

Socket 是操作系统提供的"网络文件描述符"。就像 open() 返回文件 fd，socket() 返回网络 fd。你可以 read/write 它，就像读写文件一样。

### TCP 三次握手

1. 客户端发 SYN → 服务端
2. 服务端回 SYN-ACK → 客户端
3. 客户端发 ACK → 服务端（连接建立）

C 语言中，listen() + accept() 在自动完成这些步骤。

### HTTP 请求格式

```
GET /index.html HTTP/1.1\r\n
Host: localhost:8080\r\n
User-Agent: curl/7.68.0\r\n
\r\n
```

### HTTP 响应格式

```
HTTP/1.1 200 OK\r\n
Content-Type: text/html; charset=utf-8\r\n
Content-Length: 13\r\n
\r\n
Hello, World!
```

## 常见错误

### ❌ 错误：忘记 htons()

```c
addr.sin_port = 8080;  // 错误！应该用 htons()
```

**编译器不报错，但端口会乱码：**
8080 的字节序在小端机器上变成 0x1F90 → 实际监听端口 36879

**✅ 修复：**
```c
addr.sin_port = htons(8080);  // 主机字节序 → 网络字节序
```

### ❌ 错误：不检查 bind 返回值

```c
bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
// 如果端口被占用，继续运行 → 后续 accept 永远阻塞
```

**✅ 修复：**
```c
if (bind(...) < 0) {
    perror("bind 失败");
    close(sock_fd);
    return 1;
}
```

## 动手练习

### 🟢 入门

编写代码创建 socket，绑定到 0.0.0.0:9090，然后关闭。打印每一步的状态。

<details><summary>点击查看答案</summary>

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
struct sockaddr_in addr = {0};
addr.sin_family = AF_INET;
addr.sin_port = htons(9090);
addr.sin_addr.s_addr = INADDR_ANY;
bind(fd, (struct sockaddr*)&addr, sizeof(addr));
printf("Socket created and bound to 9090\n");
close(fd);
```

</details>

### 🟡 中级

修改代码，使用 SO_REUSEADDR 选项，使程序退出后立即可重启（不用等 TIME_WAIT 超时）。

<details><summary>点击查看答案</summary>

```c
int opt = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

</details>

### 🔴 挑战

实现一个最小 HTTP 1.0 服务器：
1. listen + accept 等待连接
2. recv 读取请求
3. 解析 GET 方法
4. 发送 "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nHello"
5. close 连接

<details><summary>点击查看答案</summary>

参考 `web_socket_sample.c` 的 `demo_http_simple()` 函数

</details>

## 故障排查 (FAQ)

**Q: "Address already in use" 怎么办？**

**A:** 端口被之前的程序占用。加 SO_REUSEADDR：
```c
int opt = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

**Q: recv() 返回 0 是什么意思？**

**A:** 客户端关闭了连接。这**不是错误**，是正常的连接结束信号。

**Q: 为什么 listen 需要一个 backlog 参数？**

**A:** backlog 是等待 accept 的队列长度。如果同时有 10 个客户端连接，backlog=5 意味着只有 5 个在队列中等待，其余 5 个会被拒绝。

## 知识扩展 (选学)

### send() vs write()

两者都可以发送数据，但 send() 支持额外参数（如 MSG_OOB 带外数据）。在普通 TCP 场景下行为相同。

### 非阻塞 I/O

```c
fcntl(fd, F_SETFL, O_NONBLOCK);
```

设置非阻塞后，accept/recv 在没有连接/数据时立即返回 -1（errno=EAGAIN），而不是永远阻塞。这是构建高并发服务器的基础。

## 小结

**核心要点:**
1. socket → bind → listen → accept 是 TCP 服务器四步曲
2. 永远检查返回值，bind/accept 失败很常见
3. HTTP 就是文本协议：请求和响应都是字符串
4. 记得 close() 每个 accept 返回的 fd，防止文件描述符泄漏

**关键术语:**
socket → 网络文件描述符 → AF_INET（IPv4）, SOCK_STREAM（TCP）

## 术语表

| English | 中文 |
|---------|------|
| Socket | 套接字 |
| Bind | 绑定 |
| Listen | 监听 |
| Accept | 接受连接 |
| Backlog | 等待队列长度 |
| htons | 主机→网络字节序转换 |
| Content-Length | 响应体字节数 |

## 延伸阅读

- `web_concurrent.md` — 并发服务器模型（fork/thread per connection）
- POSIX sockets man page: `man 2 socket`

## 继续学习

← [系统调用](../system.md) | [并发服务器模型](concurrent.md) →
