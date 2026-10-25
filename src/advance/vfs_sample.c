#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#if defined(__APPLE__) || defined(BSD)
  #include <sys/mount.h>
  #include <sys/param.h>
#elif defined(__linux__)
  #include <mntent.h>
#elif defined(__sun__)
/* Solaris/illumos: /etc/mnttab 是纯文本，无需专用头文件 */
#endif

#include "advance/vfs_sample.h"

/* ====================================================================
 * VFS 与挂载点探秘 (VFS & Mount Points)
 *
 * VFS (Virtual File System) 是内核里的"文件系统总线"：上层提供
 * 统一的 open/read/stat 接口，下层由具体文件系统 (apfs/ext4/ufs)
 * 各自实现。挂载 (mount) 就是把一个文件系统接到目录树的某个节点上。
 *
 * 本节覆盖:
 *  - 挂载点枚举 — 平台四分叉 (macOS/BSD, Linux, Solaris, 兜底)
 *  - statvfs() — POSIX 侧统一拿空间使用量
 *  - 文件系统树遍历回顾 — nftw()/fts 的存在
 *  - 挂载语义 — fstab vs vfstab
 *
 * 类比: VFS 是图书馆总目录，挂载点是每个书架的门牌号 —
 *        读者 (程序) 只认目录，管理员 (内核) 负责路由到书架
 * ==================================================================== */

/* ── 1. Mount Enumeration (挂载点枚举) ── */

static void vfs_mount_sample(void) {
  printf("=== 1. 挂载点枚举 (vfs_mount_sample) ===\n");
  printf("  类比: VFS 是大楼总索引，挂载点是每个房间的门牌\n");
  printf("  核心: 同一个需求，四个平台四种读法 (平台四分叉)\n\n");

#if defined(__APPLE__) || defined(BSD)
  /* macOS/BSD: getmntinfo() 一次性快照内核挂载表 (快照式) */
  struct statfs* mntbuf = NULL;
  int count = getmntinfo(&mntbuf, MNT_NOWAIT);
  if (count <= 0 || mntbuf == NULL) {
    printf("  [Error] getmntinfo: %s\n\n", strerror(errno));
    return;
  }

  printf("  平台: macOS/BSD — getmntinfo() 快照式 (共 %d 个, 显示前 5 个)\n",
         count);
  printf("  %-4s %-30.30s %-28.28s %s\n", "#", "Device", "Mountpoint",
         "Fstype");
  printf("  %-4s %-30s %-28s %s\n", "----", "------------------------------",
         "----------------------------", "--------");
  for (int i = 0; i < count && i < 5; i++) {
    printf("  %-4d %-30.30s %-28.28s %.8s\n", i + 1, mntbuf[i].f_mntfromname,
           mntbuf[i].f_mntonname, mntbuf[i].f_fstypename);
  }
#elif defined(__linux__)
  /* Linux: /proc/mounts 是伪文件，用 setmntent/getmntent 逐行解析 */
  FILE* fp = setmntent("/proc/mounts", "r");
  if (fp == NULL) {
    printf("  [Error] setmntent(\"/proc/mounts\"): %s\n\n", strerror(errno));
    return;
  }

  printf("  平台: Linux — setmntent/getmntent/endmntent 读 /proc/mounts\n");
  printf("  %-4s %-30.30s %-28.28s %s\n", "#", "Device", "Mountpoint",
         "Fstype");
  printf("  %-4s %-30s %-28s %s\n", "----", "------------------------------",
         "----------------------------", "--------");
  struct mntent* ent;
  int shown = 0;
  while (shown < 5 && (ent = getmntent(fp)) != NULL) {
    printf("  %-4d %-30.30s %-28.28s %.8s\n", shown + 1, ent->mnt_fsname,
           ent->mnt_dir, ent->mnt_type);
    shown++;
  }
  endmntent(fp);
#elif defined(__sun__)
  /* Solaris/illumos 彩蛋: /etc/mnttab 是纯文本，strtok 切三列即可 */
  FILE* fp = fopen("/etc/mnttab", "r");
  if (fp == NULL) {
    printf("  [Error] fopen(\"/etc/mnttab\"): %s\n\n", strerror(errno));
    return;
  }

  printf("  平台: Solaris/illumos — 直接读 /etc/mnttab 纯文本\n");
  printf("  %-4s %-30.30s %-28.28s %s\n", "#", "Device", "Mountpoint",
         "Fstype");
  printf("  %-4s %-30s %-28s %s\n", "----", "------------------------------",
         "----------------------------", "--------");
  char line[1024];
  int shown = 0;
  while (shown < 5 && fgets(line, sizeof(line), fp) != NULL) {
    /* mnttab 格式: device mountpoint fstype options dump pass */
    char* dev = strtok(line, " \t\n");
    char* mnt = strtok(NULL, " \t\n");
    char* type = strtok(NULL, " \t\n");
    if (dev == NULL || mnt == NULL || type == NULL) {
      continue;
    }
    printf("  %-4d %-30.30s %-28.28s %.8s\n", shown + 1, dev, mnt, type);
    shown++;
  }
  fclose(fp);
#else
  printf("  [平台暂不支持] 本示例只覆盖 macOS/BSD、Linux、Solaris 三分叉\n");
  printf("  提示: 空间查询走纯 POSIX 的 statvfs()，见第 2 节\n");
#endif

  printf("\n");
}

/* ── 2. statvfs (POSIX 统一空间查询) ── */

static void vfs_statvfs_sample(void) {
  printf("=== 2. statvfs — POSIX 统一空间查询 (vfs_statvfs_sample) ===\n");
  printf("  类比: statvfs 是查水电表 — 不管哪个房间，读数方式一样\n\n");

  /* 纯 POSIX: 同一份代码在所有平台编译运行 */
  struct statvfs sv;
  if (statvfs("/", &sv) < 0) {
    printf("  [Error] statvfs(\"/\"): %s\n\n", strerror(errno));
    return;
  }

  unsigned long long frsize = (unsigned long long)sv.f_frsize;
  unsigned long long total = frsize * (unsigned long long)sv.f_blocks;
  unsigned long long avail = frsize * (unsigned long long)sv.f_bavail;
  unsigned long long free_bytes = frsize * (unsigned long long)sv.f_bfree;

  printf("  statvfs(\"/\") — 全平台同一份代码:\n");
  printf("    f_frsize (块大小):   %llu bytes\n", frsize);
  printf("    总容量 (f_blocks):   %10.2f GiB\n",
         (double)total / (1024.0 * 1024.0 * 1024.0));
  printf("    可用 (f_bavail):     %10.2f GiB  ← 普通用户能用的\n",
         (double)avail / (1024.0 * 1024.0 * 1024.0));
  printf("    空闲 (f_bfree):      %10.2f GiB  ← 含 root 保留块\n",
         (double)free_bytes / (1024.0 * 1024.0 * 1024.0));
  printf("\n  关键公式: 可用字节 = f_frsize * f_bavail\n");
  printf("  注意: f_bfree >= f_bavail，差值是给 root 预留的 (f_bresvd)\n");
  printf("\n");
}

/* ── 3. Tree Traversal Recap (文件系统树遍历回顾) ── */

static void vfs_tree_sample(void) {
  printf("=== 3. 文件系统树遍历回顾 (vfs_tree_sample) ===\n");
  printf("  类比: readdir 是巡逻一层楼，nftw/fts 是坐电梯走遍全楼\n\n");

  printf("  单层扫描 opendir/readdir 已在 system_file_sample 覆盖，\n");
  printf("  递归整棵树 POSIX 提供两个现成工具 (本章点到为止):\n");
  printf("    - nftw(path, cb, depth, FTW_PHYS) — POSIX 标准，回调式遍历\n");
  printf(
      "    - fts_open()/fts_read()           — BSD/Solaris 家族，可剪枝排序\n");
  printf("    - scandir() + 选择器              — 单层过滤 (Linux 常用)\n\n");
  printf("  选择建议: 简单递归用 nftw，需要跳过/剪枝用 fts，单层用 readdir\n");
  printf("\n");
}

/* ── 4. Mount Semantics (挂载语义与配置文件) ── */

static void vfs_semantics_sample(void) {
  printf("=== 4. 挂载语义与配置文件 (vfs_semantics_sample) ===\n\n");

  printf("  ① statfs 结构在 macOS 家族同构 — f_blocks/f_bavail 字段名一致，\n");
  printf("     代码在 Darwin 与 BSD 之间基本零改动\n");
  printf("  ② 开机自动挂载表 (fstab) 家族:\n");
  printf("     /etc/fstab   — Linux / BSD (设备 挂载点 类型 选项)\n");
  printf("     /etc/vfstab  — Solaris/illumos (多出 fsck 设备与检查顺序列)\n");
  printf("     /etc/mnttab  — Solaris 当前挂载表 (对应 Linux /proc/mounts)\n");
  printf("  ③ 挂载点 (mountpoint) 只是普通目录 — 挂上后目录内容被新文件系统\n");
  printf("     接管，umount 后又露出下层内容 (像叠罗汉)\n");
  printf("\n");
}

/* ── Coordinator entry ── */

int main_vfs_sample(void) {
  printf("========================================\n");
  printf("  VFS 与挂载点探秘 (VFS & Mount Points)\n");
  printf("========================================\n\n");

  printf("  类比: VFS 是图书馆的总目录，\n");
  printf("        每个挂载点是书架的门牌号\n\n");

  vfs_mount_sample();
  vfs_statvfs_sample();
  vfs_tree_sample();
  vfs_semantics_sample();

  printf("VFS 与挂载点演示完毕。\n");
  return 0;
}
