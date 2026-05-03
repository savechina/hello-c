#include <stdio.h>
#include <string.h>
#include "module1/func.h"
#include "module2/util.h"
#include "sysinfo.h"
#include "hello.h"
#include "basic/basic.h"
#include "advance/advance.h"

void print_usage(void) {
    printf("Usage: hello [subcommand]\n");
    printf("  basic     Run all basic chapters\n");
    printf("  advance   Run all advance chapters\n");
    printf("  list      List available topics\n");
    printf("  all       Run all chapters (default)\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "basic") == 0) {
            main_basic_sample();
        } else if (strcmp(argv[1], "advance") == 0) {
            main_advance_sample();
        } else if (strcmp(argv[1], "list") == 0) {
            printf("Available topics:\n");
            printf("  basic     - basic C chapters (variables, functions, pointers, structs, etc.)\n");
            printf("  advance   - advance C topics (threads, error handling, testing, etc.)\n");
            printf("  module1   - print_hello example\n");
            printf("  module2   - print_util example\n");
        } else if (strcmp(argv[1], "all") == 0 || strcmp(argv[1], "help") == 0) {
            print_usage();
        } else {
            print_usage();
            return 1;
        }
    } else {
        // Default: run all (backward compatible)
        main_hello();
    }
    return 0;
}
