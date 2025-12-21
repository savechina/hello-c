#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <time.h>
#include <wchar.h>
#ifdef __APPLE__
    /* macOS 缺失 uchar.h，手动定义 C11/C17 标准类型 */
    typedef uint_least16_t char16_t;
    typedef uint_least32_t char32_t;
    // 将 mbrtoc32 映射到 macOS 存在的 mbrtowc
    #define mbrtoc32(pc32, s, n, ps) mbrtowc((wchar_t *)(pc32), (s), (n), (ps))
#else
    #include <uchar.h>
#endif

void unicode_sample() {
  // 1. 定义 UTF-8 字符串 (char *)
  char *utf8_str = u8"你好世界";

  // 2. strlen 只能获取字节数 (UTF-8 中文通常占 3 字节)
  printf("UTF-8 字节长度: %zu\n", strlen(utf8_str)); // 输出: 12

  // 3. 将 UTF-8 转换为 UTF-32 (char32_t) 以统计字符数
  // 注意：需要设置 locale 才能正确转换
  setlocale(LC_ALL, "C.UTF-8");

  mbstate_t state = {0};
  char32_t c32;
  size_t char_count = 0;
  char *ptr = utf8_str;
  char *end = utf8_str + strlen(utf8_str);
  size_t len;

  while ((len = mbrtoc32(&c32, ptr, end - ptr, &state)) > 0) {
    if (len == (size_t)-1 || len == (size_t)-2)
      break;
    ptr += len;
    char_count++;
  }

  printf("实际字符数量: %zu\n", char_count); // 输出: 4
}

void string_sample() {
  printf("----basic ... string_sample.....start ----\n");

  // 1. 定义与初始化
  char str1[20] = "Hello";
  char str2[] = "World";
  char dest[50];

  // 2. 安全拷贝 (strncpy)
  // 建议使用 strncpy 替代 strcpy 以防止溢出
  strncpy(dest, str1, sizeof(dest) - 1);
  dest[sizeof(dest) - 1] = '\0'; // 确保以 \0 结尾
  printf("拷贝结果: %s\n", dest);

  // 3. 字符串拼接 (strncat)
  strncat(dest, " ", sizeof(dest) - strlen(dest) - 1);
  strncat(dest, str2, sizeof(dest) - strlen(dest) - 1);
  printf("拼接结果: %s\n", dest);

  // 4. 获取长度 (strlen)
  // 注意：strlen 返回字符数，不包括结尾的 \0
  printf("字符串长度: %zu\n", strlen(dest));

  // 5. 字符串比较 (strcmp)
  if (strcmp(str1, "Hello") == 0) {
    printf("str1 与 'Hello' 相等\n");
  }

  // 6. 查找子串 (strstr)
  char *pos = strstr(dest, "World");
  if (pos) {
    printf("'World' 始于位置: %ld\n", pos - dest);
  }

  // 7. 字符串拆分 (strtok) - 注意：strtok 会修改原字符串
  printf("\n--- 拆分字符串示例 ---\n");
  char data[] = "apple,banana,cherry";
  char *token = strtok(data, ",");
  while (token != NULL) {
    printf("拆分项: %s\n", token);
    token = strtok(NULL, ",");
  }

  // 8. 格式化到字符串 (sprintf)
  char info[100];
  int age = 25;
  sprintf(info, "姓名: %s, 年龄: %d", "张三", age);
  printf("\n格式化结果: %s\n", info);

  // UTF-8 字符串
  unicode_sample();

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

  //
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long long milliseconds = (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
  printf("当前时间戳（毫秒）：%lld\n", milliseconds);

  // 获取当前时间
  time_t raw_time;
  struct tm *time_info;
  char buffer[80];

  time(&raw_time);
  // 1. 将 time_t 转换为本地时间结构体 struct tm
  time_info = localtime(&raw_time);

  printf("当前时（分钟）：%i\n", time_info->tm_min);

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

// 1. 定义结构体
// 使用 typedef 可以让我们在声明变量时省略 'struct' 关键字
typedef struct {
  char name[50]; // 姓名（字符数组）
  int age;       // 年龄
} Person;

/**
 * 创建一个新的 Person 实例并初始化
 * 这里演示了如何在堆（Heap）上分配内存
 */
Person *create_person(const char *name, int age) {
  // 分配person
  Person *p = (Person *)malloc(sizeof(Person));
  if (p == NULL) {
    return NULL; // 内存分配失败
  }

  // 初始化字段
  strncpy(p->name, name, sizeof(p->name) - 1);
  p->name[sizeof(p->name) - 1] = '\0'; // 确保字符串以 null 结尾
  p->age = age;

  return p;
}

/**
 * 打印 Person 信息
 */
void print_person(const Person *p) {
  if (p != NULL) {
    printf("姓名: %s, 年龄: %d\n", p->name, p->age);
  }
}

void struct_sample() {
  printf("----basic ... struct_sample.....start ----\n");

  // 方式 A: 在栈（Stack）上创建结构体（自动管理内存）
  printf("--- 栈分配示例 ---\n");
  Person p1;
  strcpy(p1.name, "张三");
  p1.age = 25;
  print_person(&p1);

  // 方式 B: 使用初始化列表（栈分配）
  Person p2 = {"李四", 30};
  print_person(&p2);

  // 方式 C: 动态内存分配（堆分配，适合复杂项目）
  printf("\n--- 堆分配示例 ---\n");
  Person *p3 = create_person("王五", 22);
  if (p3 != NULL) {
    print_person(p3);

    // 重要：动态分配的内存必须手动释放
    free(p3);
    p3 = NULL;
  }

  printf("\n--- struct 的复制示例 ---\n");
  // struct 的复制
  struct cat {
    char name[30];
    short age;
  } a, b;

  strcpy(a.name, "Hula");
  a.age = 3;

  b = a;
  b.name[0] = 'M';

  printf("%s\n", a.name); // Hula
  printf("%s\n", b.name); // Mula

  printf("\n--- 嵌套 struct 的示例 ---\n");

  struct name {
    char first[50];
    char last[50];
  };

  struct student {
    struct name name;
    short age;
    char sex;
  } student1;

  strcpy(student1.name.first, "Harry");
  strcpy(student1.name.last, "Potter");

  // or
  struct name myname = {"Harry", "Potter"};
  student1.name = myname;
  student1.age = 25;
  student1.sex = 'M';

  printf("student1.name: {first: %s, last:%s }, age: %d, sex: %c\n",
         student1.name.first, student1.name.last, student1.age,
         student1.sex);                                    // Hula
  printf("student1.name.first %s\n", student1.name.first); // Hula

  printf("----basic ... struct_sample.....done ----\n");
}
