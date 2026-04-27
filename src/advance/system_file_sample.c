#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#if defined(__linux__)
#include <sys/sysmacros.h>
#endif

#include "advance/system_file_sample.h"

/* ====================================================================
 * 文件与目录操作 (File and Directory Operations)
 *
 * 本节目演 POSIX 底层文件 I/O：
 *  - open(), read(), write(), close() — 文件描述符操作
 *  - stat() — 文件元数据
 *  - opendir(), readdir() — 目录扫描
 *
 * 类比: fd 是房间的钥匙 — open() 拿到钥匙，read/write 进出房间，
 *        close() 还钥匙
 * ==================================================================== */

/* ── 1. File Read (文件读取) ── */

static void demo_file_read(void)
{
#ifdef __APPLE__
    const char *path = "/etc/hosts";
#elif defined(__linux__)
    const char *path = "/etc/hostname";
#else
    const char *path = "/etc/hosts";
#endif

    printf("=== 1. 文件读取 (demo_file_read) ===\n");
    printf("  类比: open() 拿到钥匙，read() 进入房间读取\n\n");

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("  [Error] open(\"%s\"): %s\n", path, strerror(errno));
        return;
    }

    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[(size_t)n] = '\0';
        printf("  读取 %zd 字节:\n  ┌──────────────────\n", n);
        /* Split into 60-char lines for display */
        for (ssize_t i = 0; i < n; i += 60) {
            ssize_t chunk = n - i > 60 ? 60 : n - i;
            printf("  │ %.*s\n", (int)chunk, buf + i);
        }
        printf("  └──────────────────\n");
    } else if (n == 0) {
        printf("  文件为空\n");
    } else {
        printf("  [Error] read: %s\n", strerror(errno));
    }

    if (close(fd) < 0) {
        printf("  [Error] close: %s\n", strerror(errno));
    }

    printf("\n");
}

/* ── 2. File Write (文件写入) ── */

static void demo_file_write(void)
{
    printf("=== 2. 文件写入 (demo_file_write) ===\n");
    printf("  类比: create 创建空房间，write 放进内容\n\n");

    const char *test_file = "/tmp/hello_c_sys_file.txt";
    const char *lines[] = {
        "Hello from low-level write!\n",
        "This is line 2 of the demo.\n",
        "Using open/write/close, not fopen/fprintf.\n"
    };
    int num_lines = 3;

    /* O_CREAT | O_WRONLY | O_TRUNC: 创建或覆盖 */
    int fd = open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("  [Error] open for write: %s\n", strerror(errno));
        return;
    }

    ssize_t total = 0;
    for (int i = 0; i < num_lines; i++) {
        size_t len = strlen(lines[i]);
        ssize_t nw = write(fd, lines[i], len);
        if (nw < 0) {
            printf("  [Error] write line %d: %s\n", i + 1, strerror(errno));
        } else {
            total += nw;
            printf("  write line %d: %zd bytes\n", i + 1, nw);
        }
    }
    printf("  总写入: %zd bytes\n", total);

    if (close(fd) < 0) {
        printf("  [Error] close: %s\n", strerror(errno));
    }

    /* Verify by reading back */
    printf("\n  验证读取:\n");
    fd = open(test_file, O_RDONLY);
    if (fd >= 0) {
        char buf[256];
        ssize_t nr = read(fd, buf, sizeof(buf) - 1);
        if (nr > 0) {
            buf[(size_t)nr] = '\0';
            printf("  ┌──────────────────\n");
            printf("  │ %.*s", (int)nr, buf);
            printf("  └──────────────────\n");
        }
        close(fd);
    }

    /* Cleanup */
    unlink(test_file);
    printf("\n");
}

/* ── 3. File Stat (文件元数据) ── */

static const char *mode_to_string(mode_t mode)
{
    if (S_ISDIR(mode))  return "directory";
    if (S_ISREG(mode))  return "regular file";
    if (S_ISLNK(mode))  return "symlink";
    if (S_ISFIFO(mode)) return "FIFO";
    if (S_ISSOCK(mode)) return "socket";
    return "other";
}

static void demo_file_stat(void)
{
    printf("=== 3. 文件元数据 (demo_file_stat) ===\n");
    printf("  类比: stat 是查房产登记，看文件的大小、权限、时间\n\n");

#ifdef __APPLE__
    const char *paths[] = { "/etc/hosts", "/tmp" };
    int npaths = 2;
#elif defined(__linux__)
    const char *paths[] = { "/etc/hostname", "/tmp" };
    int npaths = 2;
#else
    const char *paths[] = { "/etc/hosts" };
    int npaths = 1;
#endif

    for (int i = 0; i < npaths; i++) {
        struct stat st;
        if (stat(paths[i], &st) < 0) {
            printf("  [Error] stat(\"%s\"): %s\n", paths[i], strerror(errno));
            continue;
        }

        printf("  stat(\"%s\"):\n", paths[i]);
        printf("    类型:      %s\n", mode_to_string(st.st_mode));
        printf("    大小:      %lld bytes\n", (long long)st.st_size);
        printf("    块数:      %lld\n", (long long)st.st_blocks);
        printf("    权限:      %04o\n", st.st_mode & 07777);
        printf("    硬链接:    %lld\n", (long long)st.st_nlink);
        printf("    UID/GID:   %d/%d\n", (int)st.st_uid, (int)st.st_gid);
#if defined(__APPLE__)
        printf("    修改时间:  %lld\n", (long long)st.st_mtimespec.tv_sec);
#elif defined(__linux__)
        printf("    修改时间:  %lld\n", (long long)st.st_mtim.tv_sec);
#else
        printf("    修改时间:  %lld\n", (long long)st.st_mtime);
#endif
        printf("\n");
    }

    /* Error-first: try stat on nonexistent file */
    printf("  错误示范 — stat 不存在的文件:\n");
    struct stat st;
    if (stat("/nonexistent/path/that/does_not_exist", &st) < 0) {
        printf("    stat failed: errno=%d (%s)\n", errno, strerror(errno));
        printf("    ✅ 正确做法: 检查返回值 < 0，用 strerror(errno) 获取原因\n");
    }
    printf("\n");
}

/* ── 4. Directory Scan (目录扫描) ── */

static void demo_dir_scan(void)
{
    printf("=== 4. 目录扫描 (demo_dir_scan) ===\n");
    printf("  类比: opendir 打开一扇门，readdir 逐一查看每个房间\n\n");

#ifdef __APPLE__
    const char *dir_path = "/tmp";
#elif defined(__linux__)
    const char *dir_path = "/tmp";
#else
    const char *dir_path = ".";
#endif

    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        printf("  [Error] opendir(\"%s\"): %s\n", dir_path, strerror(errno));
        printf("\n");
        return;
    }

    printf("  scandir(\"%s\"): (最多显示 10 项)\n", dir_path);
    printf("  ┌──────┬──────────────────────────┬──────────┐\n");
    printf("  │ #    │ Name                     │ Type     │\n");
    printf("  ├──────┼──────────────────────────┼──────────┤\n");

    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL && count < 10) {
        const char *type;
        switch (ent->d_type) {
            case DT_DIR:  type = "dir";     break;
            case DT_REG:  type = "file";    break;
            case DT_LNK:  type = "link";    break;
            case DT_FIFO: type = "fifo";    break;
            case DT_SOCK: type = "socket";  break;
            default:      type = "unknown"; break;
        }
        printf("  │ %-6d │ %-26.26s │ %-8s │\n", count + 1, ent->d_name, type);
        count++;
    }
    printf("  └──────┴──────────────────────────┴──────────┘\n");
    if (count >= 10) {
        printf("  (更多条目已省略)\n");
    }

    closedir(dir);
    printf("\n");
}

/* ── Coordinator entry ── */

int main_system_file_sample(void)
{
    printf("========================================\n");
    printf("  文件与目录操作 (File & Directory Operations)\n");
    printf("========================================\n\n");

    printf("  类比: fd 是房间的钥匙 — open() 拿到钥匙，\n");
    printf("        read/write 进出房间，close() 还钥匙\n\n");

    demo_file_read();
    demo_file_write();
    demo_file_stat();
    demo_dir_scan();

    printf("文件与目录操作演示完毕。\n");
    return 0;
}
