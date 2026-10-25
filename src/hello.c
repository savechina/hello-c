#include "advance/advance.h"
#include "basic/basic.h"
#include <stdio.h>
/**
 * factorial
 */
unsigned long long factorial(int n) {
  if (n == 0 || n == 1)
    return 1;

  return n * factorial(n - 1);
}

/**
 * hello main entry
 */
int main_hello() {
  int x = 100020;
  printf("Hello, x = %d\n", x);

  int num = 20;
  printf("Factorial of %d is %llu\n", num, factorial(num));

  // basic sample entry
  main_basic_sample();

  // advance sample entry
  main_advance_sample();

  return 0;
}
