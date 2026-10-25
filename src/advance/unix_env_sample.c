#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if !defined(_WIN32)
  #include <pwd.h>
#endif

#include "advance/unix_env_sample.h"

/* ====================================================================
 * Unix 环境编程类比: 进程像一位新员工 — environ 是入职信息条,
 *                    uid/euid 是工牌与门禁卡, getopt 是前台登记台
 * ==================================================================== */

/* POSIX 历史接口: <unistd.h> 可能已声明 environ, 这里显式再声明一次
 * 是 APUE 的可移植写法 (compatible redeclaration, 重复声明合法) */
extern char** environ;

/* ── 平台标识 (platform tag) ── */

static const char* unix_env_platform(void) {
#if defined(_WIN32)
  return "Windows (不支持)";
#elif defined(__APPLE__)
  return "macOS (Darwin)";
#elif defined(__sun__)
  return "Solaris/illumos";
#elif defined(__FreeBSD__)
  return "FreeBSD";
#elif defined(__linux__)
  return "Linux";
#else
  return "Unknown POSIX";
#endif
}

/* ── 1. environ / getenv ── */

static void unix_env_environ_sample(void) {
#if !defined(_WIN32)
  printf("=== 1. 环境变量 environ/getenv (unix_env_environ_sample) ===\n");
  printf("  类比: environ 像入职信息条 — PATH=工具柜位置, HOME=工位\n\n");

  /* environ 是 NULL 结尾的 "KEY=VALUE" 字符串数组 (char *[]) */
  int count = 0;
  while (environ[count] != NULL) {
    count++;
  }

  printf(
      "  当前进程共 %d 个环境变量, 前 10 个名字 (只打印名字, 避免泄漏密钥):\n",
      count);
  for (int i = 0; i < count && i < 10; i++) {
    const char* eq = strchr(environ[i], '=');
    if (eq != NULL) {
      printf("    [%2d] %.*s\n", i, (int)(eq - environ[i]), environ[i]);
    }
  }
  printf("\n");

  /* getenv() 读取白名单变量 — 只打印已知安全的 key */
  static const char* safe_keys[] = {"HOME", "SHELL", "LANG", "TMPDIR"};
  printf("  getenv() 读取白名单变量:\n");
  for (size_t i = 0; i < sizeof(safe_keys) / sizeof(safe_keys[0]); i++) {
    const char* val = getenv(safe_keys[i]);
    if (val != NULL) {
      printf("    %-8s = %.60s\n", safe_keys[i], val);
    } else {
      printf("    %-8s = (未设置)\n", safe_keys[i]);
    }
  }
  printf("\n");

  printf("  常见环境变量含义:\n");
  printf("    PATH          — 可执行文件搜索路径 (冒号 ':' 分隔)\n");
  printf("    HOME          — 用户主目录 (~ 展开目标)\n");
  printf("    SHELL         — 登录 shell 路径\n");
  printf("    LANG / LC_ALL — locale 设置, 影响排序/编码/大小写\n");
  printf("    TMPDIR        — 临时目录 (macOS 默认 /var/folders/...)\n");
  printf("    PWD / OLDPWD  — 当前/上一个工作目录 (shell 维护)\n\n");

  printf("  安全警示 (security caveats):\n");
  printf("    ❌ 环境变量注入: 攻击者可控 env → 操纵 setuid 程序行为\n");
  printf("    ❌ LD_PRELOAD / LD_LIBRARY_PATH — 动态链接器加载指定库;\n");
  printf("       setuid 二进制必须忽略它们, 否则等于让攻击者注入代码\n");
  printf("    ❌ Trojan source (木马源): 不可信输入伪装成可信配置混入 env\n");
  printf("    ❌ IFS 劫持 — shell 按 IFS 分词, 篡改后命令被拆错\n");
  printf("    ✅ 安全决策不要直接信 getenv() (如 HTTP_PROXY) — 先白名单校验\n");
  printf("    ✅ setuid 程序启动时清空危险变量; 解析 PATH 用绝对路径\n\n");
#else
  printf("=== 1. 环境变量 environ/getenv (unix_env_environ_sample) ===\n");
  printf("  [跳过] 当前平台不支持 POSIX environ\n\n");
#endif
}

/* ── 2. getopt option parsing ── */

static void unix_env_getopt_sample(void) {
#if !defined(_WIN32)
  printf("=== 2. getopt 选项解析 (unix_env_getopt_sample) ===\n");
  printf("  类比: getopt 像前台登记台 — 逐个核对选项, 直到遇到位置参数\n\n");

  /* 用本地 synthetic argv 演示, 不干扰程序自身 argv */
  char* fake_argv[] = {"mytool", "-v",      "-n",        "3",
                       "-o",     "out.txt", "input.txt", NULL};
  int fake_argc = (int)(sizeof(fake_argv) / sizeof(fake_argv[0])) - 1;
  int verbose = 0;
  long count = 1;
  const char* output = NULL;
  int bad_input = 0;

  /* 重置解析器状态 (本程序只解析一次, 仅为卫生习惯):
   * glibc/BSD/macOS 用 optind = 1; illumos AT&T getopt 完整重置用 0;
   * BSD 另有 optreset 变量可用。三者此场景等价。 */
  optind = 1;

  int ch;
  while ((ch = getopt(fake_argc, fake_argv, "vn:o:h")) != -1) {
    switch (ch) {
    case 'v':
      verbose = 1;
      break;
    case 'n': {
      /* optarg 可能为 NULL (缺参数时 getopt 通常已返回 '?', 仍要防御) */
      if (optarg == NULL) {
        fprintf(stderr, "  [错误] -n 缺少参数\n");
        bad_input = 1;
        break;
      }
      char* end = NULL;
      errno = 0;
      long v = strtol(optarg, &end, 10);
      if (errno != 0 || end == optarg || *end != '\0' || v <= 0) {
        fprintf(stderr, "  [错误] -n 需要正整数, 得到: %s\n", optarg);
        bad_input = 1;
      } else {
        count = v;
      }
      break;
    }
    case 'o':
      if (optarg == NULL) {
        fprintf(stderr, "  [错误] -o 缺少文件名\n");
        bad_input = 1;
      } else {
        output = optarg;
      }
      break;
    case 'h':
      printf("  用法: mytool [-v] [-n count] [-o file] [input...]\n");
      break;
    case '?':
    default:
      /* 未知选项或缺少参数 — 不处理会静默吞掉用户输入 */
      bad_input = 1;
      break;
    }
  }

  printf("  解析结果: verbose=%d, count=%ld, output=%s\n", verbose, count,
         output != NULL ? output : "(未指定)");
  printf("  剩余位置参数 (operands):");
  for (int i = optind; i < fake_argc; i++) {
    printf(" %s", fake_argv[i]);
  }
  printf("%s\n\n", bad_input ? "  (存在非法输入)" : "");

  printf("  optstring \"vn:o:h\" 规则:\n");
  printf("    v    — 开关选项, 不带参数\n");
  printf("    n:   — 必需参数 (-n 3)\n");
  printf("    o:   — 必需参数 (-o file)\n");
  printf("    h    — 帮助\n");
  printf("    '?'  — 未知选项 / 缺少必需参数\n");
  printf("    剩余参数: argv[optind .. argc-1]\n\n");

  printf("  要点:\n");
  printf("    - getopt 可能重排 argv, 把选项挪到位置参数前面 (glibc)\n");
  printf("    - long options (--verbose) 需要 getopt_long + <getopt.h>\n");
  printf("    - 每次使用 optarg 前判 NULL; 必须处理 '?' 分支\n\n");
#else
  printf("=== 2. getopt 选项解析 (unix_env_getopt_sample) ===\n");
  printf("  [跳过] 当前平台不支持 POSIX getopt\n\n");
#endif
}

/* ── 3. process identity: uid/euid/passwd ── */

static void unix_env_identity_sample(void) {
#if !defined(_WIN32)
  printf("=== 3. 进程身份 uid/euid/passwd (unix_env_identity_sample) ===\n");
  printf("  类比: uid 是工牌号, euid 是门禁卡 — 提权程序两张卡不同\n\n");

  uid_t uid = getuid();
  uid_t euid = geteuid();
  gid_t gid = getgid();
  gid_t egid = getegid();

  printf("  四个 ID:\n");
  printf("    real uid     (getuid)  = %u  — 谁启动了我\n", (unsigned)uid);
  printf("    effective uid(geteuid) = %u  — 当前权限 (access check 用它)\n",
         (unsigned)euid);
  printf("    real gid     (getgid)  = %u\n", (unsigned)gid);
  printf("    effective gid(getegid) = %u\n", (unsigned)egid);
  if (uid != euid) {
    printf("    ⚠ uid != euid — 处于提权/降权状态 (setuid 程序)\n");
  } else {
    printf("    uid == euid — 普通进程, 两个身份一致\n");
  }
  printf("\n");

  /* getpwuid: uid → struct passwd (返回静态缓冲区, 非线程安全) */
  struct passwd* pw = getpwuid(uid);
  if (pw != NULL) {
    printf("  getpwuid(%u) → struct passwd:\n", (unsigned)uid);
    printf("    pw_name  = %s\n", pw->pw_name);
    printf("    pw_uid   = %u\n", (unsigned)pw->pw_uid);
    printf("    pw_gid   = %u\n", (unsigned)pw->pw_gid);
    printf("    pw_dir   = %s\n", pw->pw_dir != NULL ? pw->pw_dir : "(null)");
    printf("    pw_shell = %s\n",
           pw->pw_shell != NULL ? pw->pw_shell : "(null)");
  } else {
    printf("  getpwuid(%u) 失败: %s\n", (unsigned)uid, strerror(errno));
  }

  /* getpwnam: 名字 → struct passwd (cheap demo) */
  struct passwd* root = getpwnam("root");
  if (root != NULL) {
    printf("  getpwnam(\"root\") → name=%s, uid=%u, dir=%s\n", root->pw_name,
           (unsigned)root->pw_uid,
           root->pw_dir != NULL ? root->pw_dir : "(null)");
  } else {
    printf("  getpwnam(\"root\") 未找到: %s\n", strerror(errno));
  }
  printf("\n");

  printf("  setuid 家族 (提权/降权):\n");
  printf("    setuid(uid)   — 同时设置 real/effective/saved uid\n");
  printf("    seteuid(euid) — 只改 effective uid, 可来回切换\n");
  printf("    setreuid(r,e) — 分别设置 real 与 effective\n");
  printf(
      "    典型模式: root 启动 → seteuid(普通用户) 干活 → seteuid(0) 提权\n");
  printf("    ⚠ getpwuid/getpwnam 用静态缓冲区 — 多线程改用 *_r 版本\n\n");
#else
  printf("=== 3. 进程身份 uid/euid/passwd (unix_env_identity_sample) ===\n");
  printf("  [跳过] 当前平台不支持 POSIX uid/passwd\n\n");
#endif
}

/* ── 4. process group / session concepts ── */

static void unix_env_session_sample(void) {
#if !defined(_WIN32)
  printf("=== 4. 进程组与会话 (unix_env_session_sample) ===\n");
  printf("  类比: 会话=整栋写字楼, 进程组=楼层, 进程=员工\n\n");

  pid_t pid = getpid();
  pid_t pgrp = getpgrp();
  pid_t sid = getsid(pid);

  printf("  当前进程: pid=%d, 进程组 pgrp=%d\n", (int)pid, (int)pgrp);
  if (sid == (pid_t)-1) {
    printf("  getsid 失败: %s\n", strerror(errno));
  } else {
    printf("  会话 sid=%d\n", (int)sid);
  }
  printf("\n");

  printf("  层级结构:\n");
  printf("    会话 (session) ── 最多一个 controlling terminal\n");
  printf("      └── 进程组 (process group) ── 信号分发单位\n");
  printf("            └── 进程 (process)\n\n");

  printf("  要点:\n");
  printf(
      "    - setsid() 创建新会话+新进程组, 调用者成为组长 (daemon 第一步)\n");
  printf("    - 终端关闭 → SIGHUP 发给前台进程组\n");
  printf("    - Ctrl+C (SIGINT) 发给整个前台进程组 — 组内成员全被中断\n");
  printf("    - getpgrp() 返回当前进程组 ID; getsid(pid) 返回会话 ID\n");
  printf("    - 本演示只读不改 — 不调用 setsid(), 避免脱离控制终端\n\n");
#else
  printf("=== 4. 进程组与会话 (unix_env_session_sample) ===\n");
  printf("  [跳过] 当前平台不支持 POSIX session API\n\n");
#endif
}

/* ── 5. sysconf / pathconf ── */

static void unix_env_print_limit(const char* label, long value,
                                 const char* desc) {
  if (value < 0) {
    printf("    %-18s = (无限制/不确定)  — %s\n", label, desc);
  } else {
    printf("    %-18s = %ld  — %s\n", label, value, desc);
  }
}

static void unix_env_limits_sample(void) {
  printf("=== 5. 系统限制 sysconf/pathconf (unix_env_limits_sample) ===\n");
  printf("  平台: %s\n", unix_env_platform());
  printf("  类比: sysconf 问大楼物业参数, pathconf 问某个房间的规格\n\n");

  printf("  sysconf(name) — 运行时查询可配置系统限制:\n");
  unix_env_print_limit("_SC_OPEN_MAX", sysconf(_SC_OPEN_MAX),
                       "每个进程可打开的最大 fd 数");
  unix_env_print_limit("_SC_PAGESIZE", sysconf(_SC_PAGESIZE),
                       "内存页大小 (字节)");
  unix_env_print_limit("_SC_CLK_TCK", sysconf(_SC_CLK_TCK),
                       "每秒时钟滴答数 (times() 用)");
  unix_env_print_limit("_SC_ARG_MAX", sysconf(_SC_ARG_MAX),
                       "exec 参数+环境最大字节数");
  unix_env_print_limit("_SC_NGROUPS_MAX", sysconf(_SC_NGROUPS_MAX),
                       "每个进程最多附加组数");
  printf("\n");

  printf("  pathconf(path, name) — 挂载点/文件相关限制:\n");
  unix_env_print_limit("_PC_NAME_MAX(/)", pathconf("/", _PC_NAME_MAX),
                       "文件名最大长度 (NAME_MAX)");
  unix_env_print_limit("_PC_PATH_MAX(/)", pathconf("/", _PC_PATH_MAX),
                       "路径最大长度 (PATH_MAX)");
  unix_env_print_limit("_PC_LINK_MAX(/)", pathconf("/", _PC_LINK_MAX),
                       "单文件硬链接数上限");
  printf("\n");

  printf("  要点:\n");
  printf("    - 返回 -1 表示「无限制」或「不确定」— 不一定是错误\n");
  printf("    - 传统宏 NAME_MAX/PATH_MAX 是编译期猜测, 可能未定义或过时\n");
  printf("    - 不同文件系统限制不同 — 用 pathconf 按路径查询最准\n\n");
}

/* ── 6. errno / strerror recap ── */

static void unix_env_errno_sample(void) {
  printf("=== 6. errno/strerror 错误报告 (unix_env_errno_sample) ===\n");
  printf("  平台: %s\n", unix_env_platform());
  printf("  类比: errno 是故障代码牌 — 调用失败后才挂出来\n\n");

  printf("  常见 errno 速查表 (数值随平台略有差异):\n");
  printf("    %-8s %-4s %s\n", "宏名", "值", "含义");
  printf("    %-8s %-4d %s\n", "EPERM", EPERM, "操作不允许");
  printf("    %-8s %-4d %s\n", "ENOENT", ENOENT, "文件/路径不存在");
  printf("    %-8s %-4d %s\n", "EINTR", EINTR, "被信号中断, 可重试");
  printf("    %-8s %-4d %s\n", "EIO", EIO, "I/O 错误");
  printf("    %-8s %-4d %s\n", "EBADF", EBADF, "坏的文件描述符");
  printf("    %-8s %-4d %s\n", "EAGAIN", EAGAIN,
         "暂不可用, 重试 (EWOULDBLOCK)");
  printf("    %-8s %-4d %s\n", "ENOMEM", ENOMEM, "内存不足");
  printf("    %-8s %-4d %s\n", "EACCES", EACCES,
         "权限拒绝 (Permission denied)");
  printf("    %-8s %-4d %s\n", "EEXIST", EEXIST, "文件已存在");
  printf("    %-8s %-4d %s\n", "EINVAL", EINVAL, "参数无效");
  printf("    %-8s %-4d %s\n", "ENOSPC", ENOSPC, "磁盘空间不足");
  printf("    %-8s %-4d %s\n", "EPIPE", EPIPE, "管道破裂 (SIGPIPE)");
  printf("    %-8s %-4d %s\n", "ERANGE", ERANGE, "数值超出范围");
  printf("    %-8s %-4d %s\n", "ENOSYS", ENOSYS, "功能未实现\n");

  /* 实测 1: 打开不存在的文件 → ENOENT */
  errno = 0;
  FILE* fp = fopen("/nonexistent/hello-c-demo.txt", "r");
  if (fp == NULL) {
    printf("  实测 1: fopen(不存在的文件) → errno=%d (%s)\n", errno,
           strerror(errno));
  } else {
    fclose(fp);
  }

  /* 实测 2: 整数溢出 → ERANGE (纯 ISO C, 全平台可复现) */
  errno = 0;
  long huge = strtol("99999999999999999999999999", NULL, 10);
  printf("  实测 2: strtol(超大数) → %ld, errno=%d (%s)\n", huge, errno,
         strerror(errno));
  printf("\n");

  printf("  使用规则:\n");
  printf("    1. 只在函数失败后读 errno — 成功时它可能残留旧值\n");
  printf("    2. 调用前先 errno = 0, 才能区分「失败且值为0」与「没失败」\n");
  printf("    3. strerror 不是线程安全的 — 多线程用 strerror_r\n");
  printf("    4. perror(prefix) 快捷打印 \"prefix: <strerror(errno)>\\n\"\n\n");
}

/* ── Coordinator entry ── */

int main_unix_env_sample(void) {
  printf("========================================\n");
  printf("  Unix 环境编程 (Unix Environment Programming)\n");
  printf("========================================\n\n");

  printf("  平台: %s\n", unix_env_platform());
  printf(
      "  类比: 进程像新员工 — environ 是入职条, uid/euid 是工牌与门禁卡\n\n");

  unix_env_environ_sample();
  unix_env_getopt_sample();
  unix_env_identity_sample();
  unix_env_session_sample();
  unix_env_limits_sample();
  unix_env_errno_sample();

  printf("Unix 环境编程演示完毕。\n");
  return 0;
}
