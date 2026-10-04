#include <stdio.h>
#include "advance/advance.h"
#include "basic/basic.h"
/**
 * factorial
 */
unsigned long long factorial(int n) {
  if (n == 0 || n == 1)
    return 1;

  return n * factorial(n - 1);
}
/**
 * hello main
 */
int main_hello() {
    int x = 100020;
    printf("Hello, x = %d\n", x);

    int num = 20;
    printf("Factorial of %d is %llu\n", num, factorial(num));

    main_basic_sample();

    main_advance_sample();
    return 0;
}
