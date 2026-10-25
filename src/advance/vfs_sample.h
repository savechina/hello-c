#ifndef VFS_SAMPLE_H
#define VFS_SAMPLE_H

/**
 * @brief VFS 与挂载点探秘 — VFS & Mount Points
 *
 * Demonstrates virtual filesystem (VFS) introspection across platforms:
 *   1. Mount point enumeration — getmntinfo() (macOS/BSD),
 *      setmntent/getmntent (/proc/mounts, Linux), /etc/mnttab (Solaris)
 *   2. statvfs() — POSIX-unified free space query (f_frsize * f_bavail)
 *   3. Filesystem tree traversal recap — nftw() / fts() existence note
 *   4. Mount semantics — fstab vs vfstab, mountpoint as plain directory
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_vfs_sample(void);

#endif /* VFS_SAMPLE_H */
