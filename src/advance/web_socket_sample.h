#ifndef WEB_SOCKET_SAMPLE_H
#define WEB_SOCKET_SAMPLE_H

/**
 * @brief Web Socket chapter — POSIX sockets + HTTP parsing demo
 *
 * Demonstrates:
 *   1. Socket creation (socket, AF_INET, SOCK_STREAM)
 *   2. HTTP request parsing (method, path, version from raw string)
 *   3. HTTP response construction (status line, headers, body via snprintf)
 *
 * Platform: POSIX sockets (macOS + Linux)
 * Called from main_advance() → hello.c → main.c
 */
int main_web_socket_sample(void);

#endif /* WEB_SOCKET_SAMPLE_H */
