#include <stdio.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>

int main_sysinfo() {
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

    return 0;
}
