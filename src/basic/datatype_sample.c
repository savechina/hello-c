#include <stdio.h>

void string_sample() {
  printf("----basic ... string_sample.....start ----\n");

  printf("----basic ... string_sample.....done ----\n");
}

void array_sample() {
  printf("----basic ... array_sample.....start ----\n");

  int scores[100];

  scores[0] = 13;
  scores[99] = 42;

  printf("array scores %zu \n", sizeof(scores));
  printf("array scores: index 0 %d \n", scores[0]);

  int a[5] = {11, 22, 33, 44, 55};

  for (int i = 0; i < 5; i++) {
    printf("%d\n", *(a + i));
  }

  printf("----basic ... array_sample.....done ----\n");
}

void date_sample() {
  printf("----basic ... date_sample.....start ----\n");

  printf("----basic ... date_sample.....done ----\n");
}
