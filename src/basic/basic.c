#include "basic.h"
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#include "datatype_sample.h"

void variable_bind() {
  printf("----basic ... variable_bind.....start ----\n");

  int a = 1;
  printf("int variable:  a :%d ", a);

  size_t a_len = sizeof(a);
  printf(" length: %zu \n", a_len);

  // long Linux 64位整数,Windows 32位整数
  long b = 2;
  printf("long variable:  b :%ld ", b);

  size_t b_len = sizeof(b);
  printf(" length: %zu \n", b_len);

  // long long 64位整数
  long long c = 3;
  printf("long long variable:  c :%lld ", c);

  size_t c_len = sizeof(c);
  printf("length: %zu \n", c_len);

  // int64_t 64位整数
  int64_t d = 4;
  printf("int64_t variable: d :%lld ", d);

  size_t d_len = sizeof(d);
  printf("int64_t length: %zu \n", d_len);

  const int32_t x = 5;

  printf("const int32_t variable: x :%d \n", x);

  printf("---basic ... variable_bind.....end ---\n\n");
}

// declare function
//
void before_declare_func();

int main_basic(void) {
  printf("---Basic...start---\n");

  printf("Basic...\n");

  variable_bind();

  before_declare_func();

  string_sample();

  array_sample();

  date_sample();

  printf("---Basic...end---\n\n");
  return 0;
}

/**
 * declare function before when call it
 */
void before_declare_func() {
  printf("function must declare before  call it \n");
}
