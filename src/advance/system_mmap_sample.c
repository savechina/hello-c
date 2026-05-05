#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "advance/system_mmap_sample.h"

/* ====================================================================
 * 内存映射类比: mmap 给文件开了扇窗 — 不用走门 (syscall)，
 *              直接看到里面的东西
 * ==================================================================== */

#if defined(__APPLE__) || defined(__linux__)

static double bench_readwrite(const char *path, size_t size)
{
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1.0;

    /* 分配测试数据 */
    char *data = malloc(size);
    if (!data) { close(fd); return -1.0; }
    memset(data, 'A', size);

    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    /* 分块写入 */
    size_t offset = 0;
    while (offset < size) {
        size_t chunk = size - offset > 4096 ? 4096 : size - offset;
        ssize_t nw = write(fd, data + offset, chunk);
        if (nw <= 0) break;
        offset += (size_t)nw;
    }

    /* 重置偏移 */
    lseek(fd, 0, SEEK_SET);

    /* 分块读取 */
    offset = 0;
    while (offset < size) {
        size_t chunk = size - offset > 4096 ? 4096 : size - offset;
        ssize_t nr = read(fd, data + offset, chunk);
        if (nr <= 0) break;
        offset += (size_t)nr;
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    double elapsed = (ts_end.tv_sec - ts_start.tv_sec) +
                     (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

    free(data);
    close(fd);
    unlink(path);
    return elapsed;
}

static double bench_mmap(const char *path, size_t size)
{
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1.0;

    if (ftruncate(fd, (off_t)size) < 0) {
        close(fd);
        unlink(path);
        return -1.0;
    }

    void *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        unlink(path);
        return -1.0;
    }

    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    /* 写入 mmap 区域 */
    memset(mapped, 'A', size);

    /* 从 mmap 区域读取 */
    volatile char sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += ((const char *)mapped)[i];
    }
    (void)sum;

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    double elapsed = (ts_end.tv_sec - ts_start.tv_sec) +
                     (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

    munmap(mapped, size);
    close(fd);
    unlink(path);
    return elapsed;
}

#endif /* POSIX */

/* ── 1. Memory-Mapped Read ── */

static void system_mmap_read_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 1. 内存映射读取 (system_mmap_read_sample) ===\n");
    printf("  类比: mmap 像给房间开了扇窗 — 不用走门，直接看到内容\n\n");

    const char *path = "/tmp/hello_c_mmap_read.txt";
    const char *content = "Memory-mapped read: Hello, mmap world!\n";
    size_t len = strlen(content);

    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("  [Error] open: %s\n", strerror(errno));
        return;
    }

    if (write(fd, content, len) != (ssize_t)len) {
        printf("  [Error] write: %s\n", strerror(errno));
        close(fd);
        return;
    }

    /* mmap with PROT_READ: 只读映射 */
    void *mapped = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        printf("  [Error] mmap: %s\n", strerror(errno));
        close(fd);
        return;
    }

    printf("  mmap 映射了 %zu 字节到只读内存\n", len);
    printf("  通过指针读取: [%.*s]", (int)len, (const char *)mapped);
    printf("  映射地址: %p\n", mapped);

    if (munmap(mapped, len) < 0) {
        printf("  [Error] munmap: %s\n", strerror(errno));
    }
    close(fd);
    unlink(path);
    printf("\n");
#else
    printf("=== 1. 内存映射读取 (system_mmap_read_sample) ===\n");
    printf("  [跳过] 当前平台不支持 mmap\n");
    printf("\n");
#endif
}

/* ── 2. Memory-Mapped Write ── */

static void system_mmap_write_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 2. 内存映射写入 (system_mmap_write_sample) ===\n");
    printf("  类比: 在墙上开了扇窗，可以直接往房间里放东西\n\n");

    const char *path = "/tmp/hello_c_mmap_write.txt";
    const char *content = "Memory-mapped write test!";
    size_t len = strlen(content);

    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("  [Error] open: %s\n", strerror(errno));
        return;
    }

    if (ftruncate(fd, (off_t)len) < 0) {
        printf("  [Error] ftruncate: %s\n", strerror(errno));
        close(fd);
        return;
    }

    void *mapped = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        printf("  [Error] mmap: %s\n", strerror(errno));
        close(fd);
        return;
    }

    /* 直接写入映射内存 */
    memcpy(mapped, content, len);
    printf("  通过 memcpy 写入 mmap 内存区域: [%.*s]\n", (int)len, (const char *)mapped);

    /* msync 将修改同步回磁盘 */
    if (msync(mapped, len, MS_SYNC) < 0) {
        printf("  [Error] msync: %s\n", strerror(errno));
    } else {
        printf("  msync(MS_SYNC) 已强制写回磁盘\n");
    }

    printf("\n  MAP_SHARED vs MAP_PRIVATE:\n");
    printf("    MAP_SHARED:   修改会写回文件，其他映射进程可见\n");
    printf("    MAP_PRIVATE:  写时复制 (copy-on-write)，不写回文件\n");
    printf("    ✅ 需要持久化用 MAP_SHARED，临时缓存用 MAP_PRIVATE\n");

    if (munmap(mapped, len) < 0) {
        printf("  [Error] munmap: %s\n", strerror(errno));
    }
    close(fd);
    unlink(path);
    printf("\n");
#else
    printf("=== 2. 内存映射写入 (system_mmap_write_sample) ===\n");
    printf("  [跳过] 当前平台不支持 mmap\n");
    printf("\n");
#endif
}

/* ── 3. Performance Comparison (Conceptual) ── */

static void system_mmap_compare_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 3. 性能对比 (system_mmap_compare_sample) ===\n");
    printf("  类比: read/write 是敲门等管理员开门；mmap 是打通墙壁\n\n");

    const char *path_rw = "/tmp/hello_c_mmap_bench_rw.txt";
    const char *path_mm = "/tmp/hello_c_mmap_bench_mm.txt";

    /* 使用 1MB 数据比较 */
    size_t size = 1024 * 1024; /* 1 MB */

    printf("  测试: %zu 字节 (1 MB) 读写各一次\n", size);

    double time_rw = bench_readwrite(path_rw, size);
    double time_mm = bench_mmap(path_mm, size);

    if (time_rw > 0 && time_mm > 0) {
        printf("  read/write:  %.6f 秒\n", time_rw);
        printf("  mmap:        %.6f 秒\n", time_mm);
        if (time_mm < time_rw) {
            double speedup = time_rw / time_mm;
            printf("  mmap 快 %.1fx\n", speedup);
        } else {
            double slowdown = time_mm / time_rw;
            printf("  read/write 快 %.1fx (小文件场景正常)\n", slowdown);
        }
    } else {
        printf("  基准测试失败 (time_rw=%.6f, time_mm=%.6f)\n", time_rw, time_mm);
    }

    printf("\n  性能要点:\n");
    printf("    - 小文件 (< 4KB): read/write 更快 (mmap 有映射开销)\n");
    printf("    - 大文件 (> 1MB): mmap 更快 (零拷贝，直接访问)\n");
    printf("    - 频繁随机访问: mmap 明显更快\n");
    printf("    - 顺序读写: 差距不大\n\n");
#else
    printf("=== 3. 性能对比 (system_mmap_compare_sample) ===\n");
    printf("  [跳过] 当前平台不支持 mmap\n");
    printf("\n");
#endif
}

/* ── Coordinator entry ── */

int main_system_mmap_sample(void)
{
    printf("========================================\n");
    printf("  内存映射 I/O (Memory-Mapped I/O)\n");
    printf("========================================\n\n");

    printf("  类比: mmap 像给文件开了扇窗 — 直接看内容，不用走门\n\n");

    system_mmap_read_sample();
    system_mmap_write_sample();
    system_mmap_compare_sample();

    printf("内存映射 I/O 演示完毕。\n");
    return 0;
}
