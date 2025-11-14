#include "basic.h"
#include <stdio.h>

void variable_bind() {
  int a = 1;
  printf("var %d \n", a);
}

// declare function
//
void before_declare_func();

int main_basic(void) {
  printf("---Basic...start---\n");

  printf("Basic...\n");

  variable_bind();

  before_declare_func();

  printf("---Basic...end---\n");
  return 0;
}

void before_declare_func() { printf("function must declare before  call it \n"); }
