#ifndef UNIX_ENV_SAMPLE_H
#define UNIX_ENV_SAMPLE_H

/**
 * @brief Unix Environment Programming — Unix 环境编程
 *
 * Demonstrates:
 *   1. environ / getenv() — environment variables + security caveats
 *   2. getopt() — POSIX CLI option parsing
 *   3. getuid/geteuid/getgid + struct passwd (getpwuid/getpwnam)
 *   4. process group / session concepts (getpgrp/getsid)
 *   5. sysconf()/pathconf() — portable runtime system limits
 *   6. errno/strerror — error reporting recap
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_unix_env_sample(void);

#endif /* UNIX_ENV_SAMPLE_H */
