/**
 * @file web_sample.c
 * @brief HTTP 服务器 (Web Server) — Advance tutorial chapter
 *
 * Demonstrates bare-bones HTTP 1.0 server using POSIX sockets:
 *   1. Socket creation (socket())
 *   2. Address configuration (struct sockaddr_in)
 *   3. Socket options (SO_REUSEADDR)
 *   4. Bind & listen
 *   5. Accept one connection
 *   6. Parse HTTP request (method, path)
 *   7. Send HTTP response (headers + body)
 *   8. Proper resource cleanup
 *
 * Platform: POSIX sockets (macOS + Linux)
 * Keeps it simple: single connection, minimal parsing
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Platform-specific includes */
#if defined(__APPLE__) || defined(__linux__)
#define POSIX_SOCKETS 1
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif

#include "advance/web_sample.h"

/* ====================================================================
 *  HTTP 常量
 * ==================================================================== */

#define DEFAULT_PORT 8080
#define BUFFER_SIZE  4096
#define MAX_HEADERS  32

/* HTTP 响应模板 */
static const char HTTP_OK_TEMPLATE[] =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %zu\r\n"
    "Connection: close\r\n"
    "\r\n"
    "%s";

static const char HTTP_404_TEMPLATE[] =
    "HTTP/1.0 404 Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %zu\r\n"
    "Connection: close\r\n"
    "\r\n"
    "%s";

/* ====================================================================
 *  Section 1: Socket 创建
 * ==================================================================== */

/**
 * 创建 socket — 类比: 装一部电话
 */
#ifdef POSIX_SOCKETS
static int demo_socket_create(int port)
{
    (void)port;
    printf("  [1] 创建 Socket (Socket Creation):\n");
    printf("    类比: socket() = 装一部电话\n\n");

    /* 创建 IPv4 TCP socket */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("    [Error] socket(): %s\n", strerror(errno));
        return -1;
    }

    printf("    socket(AF_INET, SOCK_STREAM, 0) → fd=%d\n", sock_fd);
    printf("    AF_INET = IPv4, SOCK_STREAM = TCP\n");

    /* SO_REUSEADDR: 允许立即重用地址（防止 "Address already in use"） */
    int optval = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        printf("    [Error] setsockopt: %s\n", strerror(errno));
        close(sock_fd);
        return -1;
    }
    printf("    SO_REUSEADDR = 1 (允许地址重用)\n");

    printf("    ✅ Socket 创建成功\n\n");
    return sock_fd;
}
#endif /* POSIX_SOCKETS */

/* ====================================================================
 *  Section 2: Bind & Listen
 * ==================================================================== */

/**
 * 绑定端口并开始监听 — 类比: 把电话号码公布出去，等别人打进来
 */
#ifdef POSIX_SOCKETS
static int demo_bind_listen(int sock_fd, int port)
{
    printf("  [2] 绑定与监听 (Bind & Listen):\n");
    printf("    类比: bind = 分配电话号码, listen = 开始接听\n\n");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  /* 监听所有网卡 */
    addr.sin_port        = htons((uint16_t)port);

    /* Bind: 绑定地址和端口 */
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("    [Error] bind: %s\n", strerror(errno));
        printf("    可能原因: 端口 %d 已被占用\n", port);
        close(sock_fd);
        return -1;
    }
    printf("    bind(0.0..0.0:%d) — 成功\n", port);

    /* Listen: 开始监听 */
    if (listen(sock_fd, 5) < 0) {
        printf("    [Error] listen: %s\n", strerror(errno));
        close(sock_fd);
        return -1;
    }
    printf("    listen(backlog=5) — 成功\n");
    printf("    backlog=5: 最多允许 5 个待处理连接\n");
    printf("    ✅ 服务器已启动，等待连接...\n\n");

    return 0;  /* 继续 */
}
#endif /* POSIX_SOCKETS */

/* ====================================================================
 *  Section 3: Accept 连接
 * ==================================================================== */

/**
 * 接受连接 — 类比: 接起电话
 */
#ifdef POSIX_SOCKETS
static int demo_accept_connection(int sock_fd)
{
    printf("  [3] 接受连接 (Accept):\n");
    printf("    类比: accept = 接起电话\n\n");

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("    等待客户端连接 (accept 阻塞中)...\n");

    int client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        printf("    [Error] accept: %s\n", strerror(errno));
        return -1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    printf("    客户端已连接!\n");
    printf("      客户端 IP: %s\n", client_ip);
    printf("      客户端端口: %d\n", ntohs(client_addr.sin_port));
    printf("      连接 fd: %d\n", client_fd);
    printf("    ✅ 连接已建立\n\n");

    return client_fd;
}
#endif /* POSIX_SOCKETS */

/* ====================================================================
 *  Section 4: 解析 HTTP 请求
 * ==================================================================== */

/**
 * 解析 HTTP 请求行
 * 格式: METHOD PATH HTTP/VERSION\r\n
 */
typedef struct {
    char method[16];   /* GET, POST, etc */
    char path[256];    /* /path/to/resource */
    char version[16];  /* HTTP/1.0, HTTP/1.1 */
} HttpRequest;

/**
 * 解析简易 HTTP 请求
 * 类比: 读取电话那头说的话，提取关键信息
 */
#ifdef POSIX_SOCKETS
static int parse_http_request(const char *buf, size_t buf_len, HttpRequest *req)
{
    if (buf_len == 0 || buf == NULL || req == NULL) {
        return -1;
    }

    /* 解析第一行: METHOD PATH VERSION */
    const char *first_line_end = memchr(buf, '\r', buf_len);
    if (first_line_end == NULL) {
        first_line_end = memchr(buf, '\n', buf_len);
    }
    if (first_line_end == NULL) {
        return -1;  /* 没有换行，格式不对 */
    }

    size_t first_line_len = (size_t)(first_line_end - buf);
    if (first_line_len >= 512) {
        return -1;  /* 行太长，放弃 */
    }

    char first_line[512];
    strncpy(first_line, buf, first_line_len);
    first_line[first_line_len] = '\0';

    /* sscanf 解析: "GET /index.html HTTP/1.1" */
    if (sscanf(first_line, "%15s %255s %15s",
               req->method, req->path, req->version) < 2) {
        return -1;
    }

    return 0;
}

static void demo_parse_request(int client_fd)
{
    printf("  [4] 解析 HTTP 请求 (Parse Request):\n");
    printf("    类比: 读取电话那头说的话\n\n");

    char buf[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (bytes_read < 0) {
        printf("    [Error] recv: %s\n", strerror(errno));
        return;
    }

    buf[(size_t)bytes_read] = '\0';
    printf("    收到 %zd 字节原始数据:\n", bytes_read);
    printf("    ---\n");
    printf("    %.*s", (int)bytes_read, buf);
    printf("    ---\n\n");

    /* 解析请求 */
    HttpRequest req;
    memset(&req, 0, sizeof(req));

    if (parse_http_request(buf, (size_t)bytes_read, &req) == 0) {
        printf("    解析结果:\n");
        printf("      Method: %s\n", req.method);
        printf("      Path:   %s\n", req.path);
        printf("      Version:%s\n", req.version);
    } else {
        printf("    [Error] 无法解析 HTTP 请求\n");
    }
    printf("\n");
}
#endif /* POSIX_SOCKETS */

/* ====================================================================
 *  Section 5: 构造 HTTP 响应
 * ==================================================================== */

/**
 * 构造并发送 HTTP 响应
 * 类比: 电话那头说完后，给他一个答复
 */
#ifdef POSIX_SOCKETS
static void demo_send_response(int client_fd, const char *path)
{
    printf("  [5] 发送 HTTP 响应 (Send Response):\n");
    printf("    类比: 给电话那头一个答复\n\n");

    char response[BUFFER_SIZE * 2];

    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        /* 200 OK — HTML 响应 */
        const char *body =
            "<!DOCTYPE html>\n"
            "<html><head><title>Hello C</title></head>\n"
            "<body><h1>👋 Hello from C HTTP Server!</h1>\n"
            "<p>This page was generated by a bare-bones POSIX socket server.</p>\n"
            "<p>No frameworks, no libraries — just C.</p>\n"
            "</body></html>";

        int written = snprintf(response, sizeof(response), HTTP_OK_TEMPLATE,
                               "text/html; charset=utf-8",
                               strlen(body), body);

        if (written > 0 && (size_t)written < sizeof(response)) {
            ssize_t sent = send(client_fd, response, (size_t)written, 0);
            printf("    状态码: 200 OK\n");
            printf("    Content-Type: text/html\n");
            printf("    发送了 %zd 字节\n", sent);
            printf("    ✅ 响应已发送\n\n");
        } else {
            printf("    [Error] 响应缓冲区太小\n");
        }

    } else if (strcmp(path, "/hello") == 0) {
        /* 200 OK — 纯文本响应 */
        const char *body = "Hello, World!\n";

        int written = snprintf(response, sizeof(response), HTTP_OK_TEMPLATE,
                               "text/plain; charset=utf-8",
                               strlen(body), body);

        if (written > 0 && (size_t)written < sizeof(response)) {
            ssize_t sent = send(client_fd, response, (size_t)written, 0);
            printf("    状态码: 200 OK\n");
            printf("    Content-Type: text/plain\n");
            printf("    发送了 %zd 字节\n", sent);
            printf("    ✅ 响应已发送\n\n");
        }

    } else {
        /* 404 Not Found */
        const char *body = "404 Not Found\n";

        int written = snprintf(response, sizeof(response), HTTP_404_TEMPLATE,
                               strlen(body), body);

        if (written > 0 && (size_t)written < sizeof(response)) {
            ssize_t sent = send(client_fd, response, (size_t)written, 0);
            printf("    状态码: 404 Not Found\n");
            printf("    Content-Type: text/plain\n");
            printf("    发送了 %zd 字节\n", sent);
            printf("    ✅ 404 响应已发送\n\n");
        }
    }
}
#endif /* POSIX_SOCKETS */

/* ====================================================================
 *  Section 6: 关闭连接 & 资源清理
 * ==================================================================== */

/**
 * 关闭 socket — 类比: 挂断电话
 */
#ifdef POSIX_SOCKETS
static void demo_close_socket(int sock_fd, int client_fd)
{
    printf("  [6] 关闭与清理 (Close & Cleanup):\n");
    printf("    类比: 挂断电话 + 关电话机\n\n");

    /* 先关闭客户端连接 */
    if (client_fd >= 0) {
        close(client_fd);
        printf("    close(client_fd=%d) — 客户端连接已关闭\n", client_fd);
    }

    /* 再关闭监听 socket */
    if (sock_fd >= 0) {
        close(sock_fd);
        printf("    close(sock_fd=%d) — 服务器 socket 已关闭\n", sock_fd);
    }

    printf("    ✅ 资源全部释放\n\n");

    printf("  ❌→✅ 教训:\n");
    printf("    ❌ 忘记 close(client_fd) → 文件描述符泄漏\n");
    printf("    ✅ 每个 accept() 的 fd 必须配对 close()\n");
    printf("    ❌ 忘记 close(listen_fd) → 端口一直被占用\n");
    printf("    ✅ 服务器退出前 close 监听 socket\n");
    printf("\n");
}
#endif /* POSIX_SOCKETS */

/* ====================================================================
 *  Section 7: 完整请求-响应周期演示
 * ==================================================================== */

/**
 * 运行一个完整周期的 HTTP 服务器
 * 注意: 这是一个演示，只处理一个连接然后退出
 */
#ifdef POSIX_SOCKETS
__attribute__((unused))
static void demo_full_cycle(int port)
{
    printf("  [7] 完整请求-响应周期 (Full Cycle):\n");
    printf("    类比: 一个完整的来电接起→听→回答→挂断流程\n\n");

    int sock_fd  = demo_socket_create(port);
    if (sock_fd < 0) {
        printf("    [Error] Socket 创建失败!\n\n");
        return;
    }

    if (demo_bind_listen(sock_fd, port) < 0) {
        return;  /* bind_listen 内部已 close */
    }

    int client_fd = demo_accept_connection(sock_fd);
    if (client_fd < 0) {
        close(sock_fd);
        return;  /* accept 内部不会 close, 但我们在外面 close */
    }

    demo_parse_request(client_fd);

    HttpRequest tmp = {0};
    demo_send_response(client_fd, "/");
    (void)tmp;  /* parse result consumed in demo_parse_request */

    demo_close_socket(sock_fd, client_fd);
}
#endif /* POSIX_SOCKETS */

/* ====================================================================
 *  Section 8: HTTP 协议概念讲解 (非代码演示)
 * ==================================================================== */

/**
 * HTTP 请求/响应格式讲解
 */
static void demo_http_concepts(void)
{
    printf("  [8] HTTP 协议概念 (HTTP Protocol Concepts):\n\n");

    printf("    HTTP 请求格式:\n");
    printf("    ┌──────────────────────────────────────────────┐\n");
    printf("    │ GET /index.html HTTP/1.1    ← 请求行        │\n");
    printf("    │ Host: localhost:8080        ← 请求头        │\n");
    printf("    │ User-Agent: curl/7.79.1     ← 请求头        │\n");
    printf("    │                              ← 空行 (CRLF)  │\n");
    printf("    │ [可选 body]                                  │\n");
    printf("    └──────────────────────────────────────────────┘\n\n");

    printf("    HTTP 响应格式:\n");
    printf("    ┌──────────────────────────────────────────────┐\n");
    printf("    │ HTTP/1.0 200 OK             ← 状态行        │\n");
    printf("    │ Content-Type: text/html     ← 响应头        │\n");
    printf("    │ Content-Length: 1234        ← 响应头        │\n");
    printf("    │                              ← 空行 (CRLF)  │\n");
    printf("    │ <!DOCTYPE html>...           ← 响应体        │\n");
    printf("    └──────────────────────────────────────────────┘\n\n");

    printf("    Content-Type 常见值:\n");
    printf("      text/html          → HTML 页面\n");
    printf("      text/plain         → 纯文本\n");
    printf("      application/json   → JSON 数据\n");
    printf("      image/png          → PNG 图片\n\n");
}

/* ====================================================================
 *  Coordinator entry
 * ==================================================================== */

int main_web_sample(void)
{
    printf("========================================\n");
    printf("  HTTP 服务器 (Web Server — POSIX Sockets)\n");
    printf("========================================\n\n");

#ifdef POSIX_SOCKETS

    /* 概念讲解 */
    demo_http_concepts();

    /* 完整周期演示 — 使用端口 0 (会失败, 展示错误处理) */
    printf("\n  ⚡ 演示: 完整 HTTP 服务器生命周期\n");
    printf("     (使用端口 0 演示错误处理，展示服务器启动流程)\n\n");

    /* 注意: 这是一个演示，实际使用时需要用合法端口并处理信号 */
    (void)0;  /* 概念展示 — 实际使用时需要合法端口 */

    /* 先展示概念，然后展示错误处理 */
    printf("  [演示] 尝试用端口 0 启动服务器 (预期失败):\n\n");

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("    [Error] socket(): %s\n", strerror(errno));
    } else {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port        = htons(0);  /* 端口 0 */

        if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("    [Error] bind: %s\n", strerror(errno));
            printf("    实际部署时: 选一个未被占用的端口 (如 8080)\n");
        }

        close(sock_fd);
    }

    printf("\n  [注意] 要实际运行服务器:\n");
    printf("    1. 取消注释下面的 demo_full_cycle()\n");
    printf("    2. 改用合法端口 (如 8080)\n");
    printf("    3. 在另一个终端运行: curl http://localhost:8080/\n\n");

    /* 取消注释以下行来实际运行服务器 (会阻塞等待连接):
     *
     * demo_full_cycle(8080);
     */

    /* 代码结构展示 */
    printf("  [代码结构] 服务器生命周期:\n");
    printf("    socket() → setsockopt() → bind() → listen() → accept()\n");
    printf("    → recv() → parse() → send() → close(client) → close(server)\n");
    printf("     装电话      选号码        分配端口    开始接听   接电话\n");
    printf("     听对方说    理解意图      给出答复     挂电话      关机\n\n");

    printf("  ❌→✅ 关键教训:\n");
    printf("    ❌ 忽略 bind() 的返回值 → 端口冲突不报错\n");
    printf("    ✅ bind() 失败时打印 strerror(errno) 并退出\n");
    printf("    ❌ accept 后忘记 close → fd 泄漏\n");
    printf("    ✅ 每个 accept 的 fd 用完后必须 close\n");
    printf("    ❌ 用 printf 发 HTTP 响应 → 可能格式错误\n");
    printf("    ✅ 用 snprintf 构造完整响应，再用 send() 发送\n");

#else

    printf("  [跳过] 当前平台不支持 POSIX sockets\n");
    printf("  需要在 macOS / Linux 上编译才能运行此演示\n");
    printf("  核心概念仍然适用: socket → bind → listen → accept → read/write\n");

#endif

    printf("\nHTTP 服务器演示完毕。\n");
    return 0;
}
