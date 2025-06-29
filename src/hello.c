#include <stdio.h>

/**
 * factorial
 */
unsigned long long fibonacci(int n) {
  if (n == 0 || n == 1)
    return 1;

  return n * fibonacci(n - 1);
}
/**
 * hello main
 */
int main_hello() {
  int x = 100020;
  printf("Hello, x = %d\n", x);

  int num = 60;
  printf("Factorial of %d is %llu\n", num, fibonacci(num));

  return 0;
}
