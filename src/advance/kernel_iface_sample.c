#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "advance/kernel_iface_sample.h"

#if defined(__APPLE__) || defined(__FreeBSD__)
  #include <sys/sysctl.h> // sysctlbyname() — macOS/FreeBSD 内核 oid 接口
#elif defined(__sun__)
  #include <kstat.h> // kstat_open/lookup/read/close — illumos 内核统计接口
#endif

/* ====================================================================
 * 内核接口探测 (userland 侧观察内核) — driver 替代教程
 *
 * 类比: 内核像一栋不对外开放的大楼, 但它留了三扇「服务窗口」:
 *   - macOS/FreeBSD: sysctl  — 报门牌号(oid), 内核递数据出来
 *   - illumos/Solaris: kstat  — 仪表盘, 打开→找表→读数→关上
 *   - Linux: /proc (procfs)  — 文件即 API, fopen 就是「敲窗」
 *
 * 本章不写内核驱动, 只从 userland 敲这三扇窗 (见文末延伸说明)。
 * ==================================================================== */

#if defined(__APPLE__) || defined(__FreeBSD__)
/* ── sysctl 值的类型嗅探: 内核说了算, 不要硬假设 ──
 * sysctl 值可能是字符串(含 NUL)、u32 或 u64。先查长度再读,
 * 按长度分派 — 这是 sysctl 的惯用两段式。
 * 整段平台守卫: sysctlbyname 只在 <sys/sysctl.h> 里声明。 */
static int kernel_iface_looks_like_string(const unsigned char* buf,
                                          size_t len) {
  size_t i;

  if (len == 0 || buf[len - 1] != '\0') {
    return 0;
  }
  for (i = 0; i + 1 < len; i++) {
    if (buf[i] < 0x20U || buf[i] > 0x7EU) {
      return 0;
    }
  }
  return 1;
}

/* 打印单个 sysctl oid 的值; 任何失败都优雅跳过, 绝不崩溃 */
static void kernel_iface_sysctl_show(const char* oid) {
  unsigned char buf[128];
  size_t len = 0;

  /* 第一段: 传 NULL 查询值长度 (两段式的第一段) */
  if (sysctlbyname(oid, NULL, &len, NULL, 0) != 0) {
    printf("  [跳过] %-20s 不可用: %s\n", oid, strerror(errno));
    return;
  }
  if (len == 0 || len > sizeof(buf)) {
    printf("  [跳过] %-20s 值长度 %zu 超出演示缓冲\n", oid, len);
    return;
  }

  /* 第二段: 按查询到的长度读取真实值 */
  memset(buf, 0, sizeof(buf));
  if (sysctlbyname(oid, buf, &len, NULL, 0) != 0) {
    printf("  [跳过] %-20s 读取失败: %s\n", oid, strerror(errno));
    return;
  }

  if (kernel_iface_looks_like_string(buf, len)) {
    printf("  %-22s = \"%s\" (string, %zu 字节)\n", oid, (char*)buf, len);
  } else if (len == sizeof(uint32_t)) {
    uint32_t v32 = 0;
    memcpy(&v32, buf, sizeof(v32)); // memcpy: 避免未对齐访问与别名问题
    printf("  %-22s = 0x%08x (u32, %zu 字节)\n", oid, v32, len);
  } else if (len == sizeof(uint64_t)) {
    uint64_t v64 = 0;
    memcpy(&v64, buf, sizeof(v64));
    printf("  %-22s = 0x%llx (u64, %zu 字节)\n", oid, (unsigned long long)v64,
           len);
  } else {
    printf("  %-22s = <%zu bytes raw>\n", oid, len);
  }
}
#endif /* defined(__APPLE__) || defined(__FreeBSD__) */

/* ── 1. sysctlbyname() — macOS/FreeBSD ── */

static void kernel_iface_sysctl_demo(void) {
#if defined(__APPLE__) || defined(__FreeBSD__)
  size_t bogus_len = 0;

  printf("=== 1. sysctlbyname() — macOS/FreeBSD 内核接口 ===\n");
  printf(
      "  类比: sysctl 像「内核的点菜窗口」— 报出 oid 名字, 内核端上数据\n\n");

  kernel_iface_sysctl_show("kern.ostype");    // 例如 "Darwin" / "FreeBSD"
  kernel_iface_sysctl_show("kern.osrelease"); // 例如 "24.6.0"

  /* 有趣的 oid: kern.usrstack — 进程栈基址潜伏在内核里。
   * macOS 上它是 32 位值(高地址被截断), kern.usrstack64 才是完整值。 */
  printf("  --- 进程栈基址 (userland 栈区域, 内核视角) ---\n");
  kernel_iface_sysctl_show("kern.usrstack");
  kernel_iface_sysctl_show("kern.usrstack64");

  /* 错误处理演示: 未知 oid 必须优雅跳过 — errno=ENOENT, 程序继续 */
  printf("  --- 错误处理: 未知 oid 不崩溃 ---\n");
  if (sysctlbyname("kern.this_oid_does_not_exist", NULL, &bogus_len, NULL, 0) !=
      0) {
    printf("  [预期失败] kern.this_oid_does_not_exist → %s (已优雅跳过)\n",
           strerror(errno));
  } else {
    printf("  [意外] 未知 oid 竟然存在?\n");
  }
  printf("\n");
#else
  printf("=== 1. sysctlbyname() — macOS/FreeBSD 内核接口 ===\n");
  printf("  [跳过] sysctl(3) 需要 macOS/FreeBSD — 当前平台不适用\n");
  printf("         (Linux 的对应物是 /proc/sys, 见第 3 节)\n\n");
#endif
}

/* ── 2. kstat() — illumos/Solaris ── */

static void kernel_iface_kstat_demo(void) {
#if defined(__sun__)
  kstat_ctl_t* kc;
  kstat_t* ksp;
  kstat_named_t* kn;

  printf("=== 2. kstat() — illumos/Solaris 内核接口 ===\n");
  printf("  类比: kstat 像「内核的仪表盘」— 打开、找表、读数、关上\n\n");

  /* 1) 打开 kstat 框架 — 所有 kstat 操作的总句柄 */
  kc = kstat_open();
  if (kc == NULL) {
    printf("  [错误] kstat_open: %s\n", strerror(errno));
    return;
  }

  /* 2) 定位: module="unix", instance=0, name="system_misc" */
  ksp = kstat_lookup(kc, "unix", 0, "system_misc");
  if (ksp == NULL) {
    printf("  [跳过] kstat_lookup(unix/system_misc): %s\n", strerror(errno));
    kstat_close(kc);
    return;
  }

  /* 3) 类型检查: 只有 KSTAT_type_named (KSTAT_TYPE_NAMED) 的 kstat
   *    才能按 kstat_named_t 解读 — 这是 illumos principal 接口的约定 */
  if (ksp->ks_type != KSTAT_TYPE_NAMED) {
    printf("  [跳过] kstat 类型不是 KSTAT_type_named (实际类型=%u)\n",
           (unsigned)ksp->ks_type);
    kstat_close(kc);
    return;
  }

  /* 4) 读取当前快照 — kstat 是内核周期性刷新的统计, read 才拿到最新值 */
  if (kstat_read(kc, ksp, NULL) == -1) {
    printf("  [错误] kstat_read: %s\n", strerror(errno));
    kstat_close(kc);
    return;
  }

  /* 5) 在 named 数据里查 "nproc" — 当前系统进程总数 */
  kn = (kstat_named_t*)kstat_data_lookup(ksp, "nproc");
  if (kn == NULL) {
    printf("  [跳过] kstat_data_lookup(nproc): %s\n", strerror(errno));
    kstat_close(kc);
    return;
  }
  printf("  nproc = %u (当前系统进程总数)\n", kn->value.ui32);

  /* 6) 收尾: 每条错误路径都已 kstat_close, 正常路径在此关闭 */
  kstat_close(kc);
  printf("\n");
#else
  printf("=== 2. kstat() — illumos/Solaris 内核接口 ===\n");
  printf("  [跳过] kstat(3K) 需要 illumos/Solaris — 当前平台不适用\n");
  printf("         (macOS/FreeBSD 用 sysctl, Linux 用 /proc)\n\n");
#endif
}

/* ── 3. /proc (procfs) — Linux ── */

static void kernel_iface_procfs_demo(void) {
#if defined(__linux__)
  char line[128];
  double load1 = 0.0, load5 = 0.0, load15 = 0.0;
  FILE* fp;

  printf("=== 3. /proc (procfs) — Linux 内核接口 ===\n");
  printf("  类比: 文件即 API — fopen 就是「敲内核的窗」\n\n");

  /* 3.1 内核版本: /proc/sys/kernel/osrelease 单行文本 */
  fp = fopen("/proc/sys/kernel/osrelease", "r");
  if (fp != NULL) {
    if (fgets(line, sizeof(line), fp) != NULL) {
      line[strcspn(line, "\r\n")] = '\0';
      printf("  /proc/sys/kernel/osrelease = %s\n", line);
    } else {
      printf("  [跳过] osrelease 读取失败: %s\n", strerror(errno));
    }
    fclose(fp);
  } else {
    printf("  [跳过] fopen(/proc/sys/kernel/osrelease): %s\n", strerror(errno));
  }

  /* 3.2 负载均值: /proc/loadavg 前三个字段是 1/5/15 分钟三个 double */
  fp = fopen("/proc/loadavg", "r");
  if (fp != NULL) {
    if (fscanf(fp, "%lf %lf %lf", &load1, &load5, &load15) == 3) {
      printf("  /proc/loadavg = %.2f %.2f %.2f (1/5/15 分钟)\n", load1, load5,
             load15);
    } else {
      printf("  [跳过] loadavg 解析失败: 字段不足 3 个\n");
    }
    fclose(fp);
  } else {
    printf("  [跳过] fopen(/proc/loadavg): %s\n", strerror(errno));
  }

  printf("  说明: /proc 自身是 procfs — Linux 的「驱动式」内核实现,\n");
  printf("        内核把状态导出成文件树, 文件即 API, 读文件 = 读内核。\n\n");
#else
  printf("=== 3. /proc (procfs) — Linux 内核接口 ===\n");
  printf("  [跳过] procfs 需要 Linux — 当前平台不适用\n");
  printf("         (macOS/FreeBSD 用 sysctl, illumos 用 kstat)\n\n");
#endif
}

/* ── 4. 延伸说明: 为什么本仓不教内核驱动 (所有平台都打印) ── */

static void kernel_iface_why_no_drivers(void) {
  printf("=== 4. 延伸说明: 为什么本仓不教内核驱动 ===\n");
  printf("  想写 LED 驱动? 先在这里学会走路:\n\n");
  printf(
      "  - 内核驱动跑在内核态, 一次野指针 = 整机 panic, 不是 segfault 而已\n");
  printf("  - macOS: KEXT/KPI 需要内核头 + 独立签名/构建流程 (DriverKit "
         "是新路)\n");
  printf("  - FreeBSD: kernel modules 需要 src 树 + sys/conf 构建系统\n");
  printf("  - illumos: drivers 需要 DDK + 独立的驱动构建模板\n");
  printf("  - 本项目 Makefile 只能编译 userland 二进制, 无法产出内核态模块\n");
  printf("  故此章用「userland 内核接口」代替 driver 教学 — 同样的内核,\n");
  printf("  零 panic 风险。\n\n");
  printf("  指路:\n");
  printf("    FreeBSD Handbook → Kernel Modules & Tuning\n");
  printf("    Apple Developer → DriverKit (取代 KEXT 的用户态驱动框架)\n");
  printf("    illumos Docs → Device Driver 教程 (Writing Device Drivers)\n\n");
}

/* ── Coordinator entry ── */

int main_kernel_iface_sample(void) {
  printf("========================================\n");
  printf("  内核接口探测 (Kernel Interface Probing)\n");
  printf("========================================\n\n");

  /* 平台四分叉: 与 src/sysinfo.c 同款 #ifdef 迷宫 */
#if defined(__APPLE__) || defined(__FreeBSD__)
  printf("  当前平台接口: sysctl(3) — 内核通过命名 oid 暴露状态\n\n");
#elif defined(__sun__)
  printf("  当前平台接口: kstat(3K) — 内核统计框架\n\n");
#elif defined(__linux__)
  printf("  当前平台接口: /proc + /sys (procfs/sysfs) — 文件即 API\n\n");
#else
  printf("  [兜底] 未知平台: 三大接口均不可用, 仅打印说明\n\n");
#endif

  kernel_iface_sysctl_demo(); // 只在 macOS/FreeBSD 生效, 其余平台打印跳过
  kernel_iface_kstat_demo();  // 只在 illumos/Solaris 生效
  kernel_iface_procfs_demo(); // 只在 Linux 生效
  kernel_iface_why_no_drivers();

  printf("内核接口探测演示完毕。\n");
  return 0;
}
