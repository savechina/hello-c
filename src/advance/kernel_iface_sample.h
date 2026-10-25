#ifndef KERNEL_IFACE_SAMPLE_H
#define KERNEL_IFACE_SAMPLE_H

/**
 * @brief Kernel Interface Probing — 内核接口探测 (userland 侧观察内核)
 *
 * A driver-substitute chapter: instead of loading kernel modules, this
 * probes the three userland-visible kernel interfaces:
 *   1. sysctlbyname()  — macOS / FreeBSD  (named oids, e.g. kern.ostype)
 *   2. kstat()         — illumos / Solaris (kernel statistics framework)
 *   3. /proc + /sys    — Linux (procfs: the file IS the API)
 * plus a closing note on why kernel drivers live outside this repo.
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_kernel_iface_sample(void);

#endif /* KERNEL_IFACE_SAMPLE_H */
