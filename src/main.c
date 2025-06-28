#include <stdio.h>
#include "module1/func.h"
#include "module2/util.h"
#include "sysinfo.h"
#include "hello.h"
// #include "global.h"
int main() {
    print_hello();
    print_util();

    main_hello();

    main_sysinfo();

    return 0;
}
