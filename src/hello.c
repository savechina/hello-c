#include <stdio.h>

/**
 * factorial
 */
unsigned long factorial(int n) {
  if (n == 0 || n == 1)
    return 1;
  return n * factorial(n - 1);
}
/**
 * hello main
 */
int main_hello() {
  int x = 42;
  printf("Hello, x = %d\n", x);

  int num = 5;
  printf("Factorial of %d is %lu\n", num, factorial(num));

  return 0;
}
