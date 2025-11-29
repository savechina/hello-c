#include <stdio.h>
#include <sys/signal.h>
#include <time.h>

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

  time_t current_time;

  // 1. 获取当前时间的秒数（自 Epoch 以来）
  current_time = time(NULL);

  printf("--- 1. 获取当前时间和日期 ---\n");
  // 2. 将 time_t 转换为可读的本地时间字符串
  printf("当前系统时间 (ctime): %s\n", ctime(&current_time));

  time_t raw_time;
  struct tm *time_info;
  char buffer[80];

  time(&raw_time);

  // 1. 将 time_t 转换为本地时间结构体 struct tm
  time_info = localtime(&raw_time);

  printf("--- 2. 使用 localtime 和 strftime 格式化时间 ---\n");

  // 2. 使用 strftime 格式化时间
  // 格式字符串的含义：
  // %Y - 4位数年份 (e.g., 2025)
  // %m - 2位数月份 (01-12)
  // %d - 2位数日期 (01-31)
  // %H - 24小时制小时 (00-23)
  // %M - 分钟 (00-59)
  // %S - 秒 (00-59)
  strftime(buffer, 80, "今天是 %Y-%m-%d，现在是 %H:%M:%S", time_info);

  printf("格式化后的时间: %s\n", buffer);

  // 另一个常用格式: 星期几, 月份, 日期, 时间, 年份
  strftime(buffer, 80, "%a, %b %d, %Y - %I:%M:%S %p", time_info);
  printf("另一种格式:    %s\n", buffer);

  printf("----basic ... date_sample.....done ----\n");
}
