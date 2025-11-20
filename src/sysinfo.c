#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#if defined(__APPLE__)
#include <mach/mach.h> // macOS 特有的头文件，用于获取系统信息
#include <sys/sysctl.h> // macOS 和 Linux 都可能有，但具体用法可能不同
#define OS_NAME "macOS"
#elif defined(__linux__)
#include <sys/sysinfo.h> // Linux 特有的头文件，用于获取系统信息
#define OS_NAME "Linux"
// #elif defined(__sun) && defined(__SRV4)
#elif defined(__illumos__)
#include <sys/sysinfo.h> // Solaris特有的头文件，用于获取系统信息
#define OS_NAME "sunOS"
#if defined(__illumos__)
// Specific code for Illumos-based OS (best way to distinguish forks)
#define OS_ILLUMOS
#else
// Assumed to be traditional (Oracle) Solaris or older Illumos/SunOS
#define OS_SOLARIS
#endif
#else
#define OS_NAME "Unknown"
// 对于其他系统，可能需要添加更多宏判断和头文件
// 例如 Windows: #include <windows.h>
#endif

// 声明一个函数来获取系统信息，具体实现会在后面根据平台定义
void get_system_info();
void get_info();

int main_sysinfo() {
  printf("Detected operating system: %s\n", OS_NAME);

#if defined(__APPLE__)
  get_info();
#endif

  printf("Print operating system: %s\n", OS_NAME);
  get_system_info();
  return 0;
}

// macOS 平台获取系统信息
#if defined(__APPLE__)
void get_system_info() {
  printf("\n--- macOS System Information ---\n");

  // 获取系统名称 (例如 "Darwin")
  char os_type[256];
  size_t os_type_len = sizeof(os_type);
  if (sysctlbyname("kern.ostype", os_type, &os_type_len, NULL, 0) == 0) {
    printf("OS Type: %s\n", os_type);
  } else {
    perror("Failed to get kern.ostype");
  }

  // 获取系统版本 (例如 "23.5.0")
  char os_release[256];
  size_t os_release_len = sizeof(os_release);
  if (sysctlbyname("kern.osrelease", os_release, &os_release_len, NULL, 0) ==
      0) {
    printf("OS Release: %s\n", os_release);
  } else {
    perror("Failed to get kern.osrelease");
  }

  // 获取 CPU 信息 (例如 "arm64", "x86_64")
  char cpu_type[256];
  size_t cpu_type_len = sizeof(cpu_type);
  if (sysctlbyname("machdep.cpu.brand_string", cpu_type, &cpu_type_len, NULL,
                   0) == 0) {
    printf("CPU Type: %s\n", cpu_type);
  } else {
    // Fallback for older macOS or simpler CPU info
    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_MACHINE;
    char arch_name[256];
    size_t arch_name_len = sizeof(arch_name);
    if (sysctl(mib, 2, arch_name, &arch_name_len, NULL, 0) == 0) {
      printf("CPU Architecture: %s\n", arch_name);
    } else {
      perror("Failed to get CPU type");
    }
  }

  // 获取内存信息
  vm_statistics64_data_t vm_stats;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
                        (host_info_t)&vm_stats, &count) == KERN_SUCCESS) {
    // 页面大小通常是 4096 字节
    long long total_pages = vm_stats.wire_count + vm_stats.active_count +
                            vm_stats.inactive_count + vm_stats.free_count;
    long long total_memory_bytes = total_pages * vm_page_size;
    printf("Total Memory: %.2f GB\n",
           (double)total_memory_bytes / (1024 * 1024 * 1024));
  } else {
    perror("Failed to get memory info");
  }
}

void get_info() {
  // Get system architecture
  char *arch = NULL;
  size_t size = 0;
  if (sysctlbyname("hw.machine", NULL, &size, NULL, 0) == 0) {
    arch = (char *)malloc(size);
    if (arch != NULL) {
      if (sysctlbyname("hw.machine", arch, &size, NULL, 0) == 0) {
        printf("System Architecture: %s\n", arch);
      } else {
        perror("sysctlbyname (hw.machine) failed");
      }
      free(arch);
    } else {
      perror("malloc failed");
    }
  } else {
    perror("sysctlbyname (size of hw.machine) failed");
  }

  // Get operating system type and release
  struct utsname uname_data;
  if (uname(&uname_data) == 0) {
    printf("Operating System Type: %s\n", uname_data.sysname);
    printf("Operating System Release: %s\n", uname_data.release);

    // Further identify as macOS (Darwin is the underlying OS)
    if (strcmp(uname_data.sysname, "Darwin") == 0) {
      printf("Operating System: macOS\n");
    }
  } else {
    perror("uname failed");
  }
}

// Linux 平台获取系统信息
#elif defined(__linux__)
void get_system_info() {
  printf("\n--- Linux System Information ---\n");
  struct sysinfo info;

  if (sysinfo(&info) == 0) {
    // 系统正常运行时间
    printf("Uptime: %ld seconds\n", info.uptime);

    // 总内存 (转换为 GB)
    double total_ram_gb =
        (double)info.totalram * info.mem_unit / (1024 * 1024 * 1024);
    printf("Total RAM: %.2f GB\n", total_ram_gb);

    // 可用内存 (转换为 GB)
    double free_ram_gb =
        (double)info.freeram * info.mem_unit / (1024 * 1024 * 1024);
    printf("Free RAM: %.2f GB\n", free_ram_gb);

    // 进程数量
    printf("Processes: %u\n", info.procs);
  } else {
    perror("Failed to get sysinfo");
  }

  // 获取操作系统版本（例如 Ubuntu 22.04
  // LTS） 通常从 /etc/os-release 文件中获取
  FILE *fp = fopen("/etc/os-release", "r");
  if (fp) {
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
      if (strstr(line, "PRETTY_NAME=")) {
        printf("OS Version: %s",
               line + strlen("PRETTY_NAME=") + 1); // +1
                                                   // 跳过等号和第一个引号
        // 移除末尾的引号和换行符
        char *end = strrchr(line, '"');
        if (end)
          *end = '\0';
        printf("OS Version: %s\n", line + strlen("PRETTY_NAME=") + 1);
        break;
      }
    }
    fclose(fp);
  } else {
    perror("Failed to open /etc/os-release");
  }

  // 获取 CPU 信息 (从 /proc/cpuinfo)
  fp = fopen("/proc/cpuinfo", "r");
  if (fp) {
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
      if (strstr(line, "model name")) {
        printf("CPU Model: %s", line + strlen("model name\t: "));
        break;
      }
    }
    fclose(fp);
  } else {
    perror("Failed to open /proc/cpuinfo");
  }
}

// Linux 平台获取系统信息
#elif defined(__sun) && defined(__SRV4)
void get_system_info() {
    printf("\n--- Solaris System Information ---\n");
    struct sysinfo info;
  printf("System information retrieval not implemented for this Solaris.\n");
}
#else // Unknown OS
void get_system_info() {
  printf("System information retrieval not implemented for this OS.\n");
}
#endif
