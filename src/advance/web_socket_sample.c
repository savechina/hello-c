/**
 * @file web_socket_sample.c
 * @brief Socket 与 HTTP 协议 — 原始 Sockets + HTTP 解析
 *
 * 演示 POSIX 原始套接字编程三步:
 *   1. Socket 创建: socket(AF_INET, SOCK_STREAM, 0)
 *   2. HTTP 解析: 从原始字符串提取 METHOD /path HTTP/1.0
 *   3. HTTP 响应: 用 snprintf 构造完整响应并发送
 *
 * 类比: Socket 像装电话 — socket() 是买手机, bind() 是办号码,
 *       listen() 是开机, accept() 是接听。
 *
 * Platform: POSIX sockets (macOS + Linux)
 * NO actual server binding — parse+respond from raw strings.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(__APPLE__) || defined(__linux__)
#define POSIX_SOCKETS 1
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#include "advance/web_socket_sample.h"

/* ====================================================================
 *  Constants
 * ==================================================================== */

#define BUFFER_SIZE  4096

/* ====================================================================
 *  Demo 1: Socket Basics — 创建 socket, 理解 AF_INET / SOCK_STREAM
 * ==================================================================== */

#ifdef POSIX_SOCKETS
static void web_socket_basic_sample(void)
{
    printf("  [1] Socket 创建 (Socket Basic):\n");
    printf("    类比: socket() = 买一部手机\n\n");

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("    [Error] socket(): %s\n", strerror(errno));
        return;
    }

    printf("    socket(AF_INET, SOCK_STREAM, 0) → fd=%d\n", sock_fd);
    printf("    AF_INET    = IPv4 地址族\n");
    printf("    SOCK_STREAM = TCP 字节流连接\n");
    printf("    protocol=0  → 自动选 TCP\n\n");

    /* 选项: SO_REUSEADDR */
    int optval = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval)) < 0) {
        printf("    [Error] setsockopt: %s\n", strerror(errno));
        close(sock_fd);
        return;
    }
    printf("    SO_REUSEADDR = 1 (退出后可立即重用端口)\n");
    printf("    ❌ 不设 SO_REUSEADDR → 重启报 \"Address already in use\"\n");
    printf("    ✅ 设了就能秒重启\n");

    close(sock_fd);
    printf("    close(fd=%d) — 手机不用了关机\n\n", sock_fd);
}
#else
static void web_socket_basic_sample(void)
{
    printf("  [1] 跳过 — 平台不支持 POSIX sockets\n\n");
}
#endif

/* ====================================================================
 *  Demo 2: HTTP Parse — 从原始请求字符串解析 METHOD, Path, Version
 * ==================================================================== */

typedef struct {
    char method[16];
    char path[256];
    char version[16];
} HttpRequest;

static int parse_http_request(const char *buf, size_t buf_len,
                              HttpRequest *req)
{
    if (buf == NULL || buf_len == 0 || req == NULL) {
        return -1;
    }

    /* 找到第一行结尾 */
    const char *line_end = memchr(buf, '\r', buf_len);
    if (line_end == NULL) {
        line_end = memchr(buf, '\n', buf_len);
    }
    if (line_end == NULL) {
        return -1;
    }

    size_t line_len = (size_t)(line_end - buf);
    if (line_len >= 512) {
        return -1;
    }

    char first_line[512];
    memcpy(first_line, buf, line_len);
    first_line[line_len] = '\0';

    /* "GET /index.html HTTP/1.0" */
    if (sscanf(first_line, "%15s %255s %15s",
               req->method, req->path, req->version) < 2) {
        req->version[0] = '\0';
    }

    return 0;
}

static void web_socket_http_parse_sample(void)
{
    printf("  [2] HTTP 请求解析 (HTTP Parse):\n");
    printf("    类比: 读电话那头说的话，抓关键词\n\n");

    /* 用原始字符串模拟 recv() 拿到的数据 */
    const char *raw_request =
        "GET /index.html HTTP/1.0\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: curl/8.0\r\n"
        "Accept: */*\r\n"
        "\r\n";

    size_t raw_len = strlen(raw_request);

    printf("    收到 %zu 字节 (模拟 recv):\n", raw_len);
    printf("    --- Raw Request ---\n");
    printf("    %.*s", (int)raw_len, raw_request);
    printf("    --- End ---\n\n");

    HttpRequest req;
    memset(&req, 0, sizeof(req));

    if (parse_http_request(raw_request, raw_len, &req) == 0) {
        printf("    ✅ 解析结果:\n");
        printf("      Method  = \"%s\"\n", req.method);
        printf("      Path    = \"%s\"\n", req.path);
        printf("      Version = \"%s\"\n", req.version);

        /* 错误-first 教训 */
        printf("\n    ❌→✅ 教训:\n");
        printf("      ❌ 用 sscanf 不加宽度限制 → 缓冲区溢出\n");
        printf("      ✅ %%15s %%255s — 限制最大长度\n");
        printf("      ❌ 不检查结果数量 → 请求不完整也不报错\n");
        printf("      ✅ sscanf 返回值 < 2 表示格式不对\n");
    } else {
        printf("    [Error] 无法解析 HTTP 请求\n");
    }
    printf("\n");
}

/* ====================================================================
 *  Demo 3: HTTP Response — 构造响应 (状态行 + 头部 + 空行 + body)
 * ==================================================================== */

static void web_socket_http_response_sample(void)
{
    printf("  [3] HTTP 响应构造 (HTTP Response):\n");
    printf("    类比: 理解了对方的话，给个答复\n\n");

    /* --- 200 OK — HTML --- */
    const char *html_body =
        "<!DOCTYPE html>\n"
        "<html><head><title>Hello C</title></head>\n"
        "<body><h1>👋 Hello from Socket Server!</h1>"
        "<p>Zero frameworks. Just C.</p></body></html>";

    char response[BUFFER_SIZE * 2];
    int written = snprintf(response, sizeof(response),
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(html_body), html_body);

    if (written > 0 && (size_t)(unsigned int)written < sizeof(response)) {
        printf("    ✅ 200 OK 响应 (Content-Type: text/html):\n");
        printf("    --- Response ---\n");
        /* 只打印头部，body 太长 */
        const char *body_start = strstr(response, "\r\n\r\n");
        if (body_start) {
            printf("    %.*s", (int)(body_start - response), response);
            printf("    ...\n");
            printf("    [Body: %zu bytes HTML]\n", strlen(html_body));
        }
        printf("    --- End ---\n\n");

        printf("    关键格式:\n");
        printf("    HTTP/1.0 200 OK\\r\\n   ← 状态行\n");
        printf("    Content-Type: ...\\r\\n  ← 头部\n");
        printf("    Content-Length: ...\\r\\n← 头部\n");
        printf("    \\r\\n                    ← 空行 (必须!)\n");
        printf("    <body>...               ← body\n\n");
    } else {
        printf("    [Error] 响应缓冲区不够大\n\n");
    }

    /* --- 404 Not Found --- */
    const char *err_body = "404 Not Found\n";
    written = snprintf(response, sizeof(response),
        "HTTP/1.0 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(err_body), err_body);

    if (written > 0 && (size_t)(unsigned int)written < sizeof(response)) {
        printf("    ✅ 404 Not Found 响应:\n");
        printf("    --- Response ---\n");
        printf("    %.*s", written, response);
        printf("    --- End ---\n\n");
    }

    /* 错误-first 教训 */
    printf("    ❌→✅ 教训:\n");
    printf("      ❌ 用 sprintf 拼响应 → 缓冲区溢出\n");
    printf("      ✅ 用 snprintf(buf, sizeof(buf), ...) — 安全\n");
    printf("      ❌ 忘写 \\r\\n\\r\\n → 客户端无限等 \n");
    printf("      ✅ 头部和 body 之间必须 \\r\\n\\r\\n\n");
    printf("      ❌ Content-Length 算错 → 客户端截断或挂起\n");
    printf("      ✅ 用 strlen(body) 精确计算\n\n");
}

/* ====================================================================
 *  Coordinator
 * ==================================================================== */

int main_web_socket_sample(void)
{
    printf("========================================\n");
    printf("  Socket 与 HTTP 协议 (Raw Sockets + HTTP)\n");
    printf("========================================\n\n");

    web_socket_basic_sample();
    web_socket_http_parse_sample();
    web_socket_http_response_sample();

    printf("Socket 与 HTTP 解析演示完毕。\n");
    return 0;
}
