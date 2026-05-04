#include <stdio.h>
#include <stdlib.h>
#include "algo/algo.h"

/* ============================================================
 * Algo 模块 — 常用算法演示
 * ============================================================
 * 本模块展示 C 语言中常见算法的实现，每个算法独立为函数，
 * 通过 main_algo_sample() 统一调度执行。
 * ============================================================ */

/* ---------- 冒泡排序 (Bubble Sort) ---------- */
static void bubble_sort(int arr[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

static void print_array(const int arr[], int n)
{
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

/* ---------- 二分查找 (Binary Search) ---------- */
static int binary_search(const int arr[], int n, int target)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] == target)
            return mid;
        else if (arr[mid] < target)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

/* ---------- 斐波那契动态规划 (unsigned long long, 上限 F(92)) ---------- */
static unsigned long long fibonacci_ull(int n)
{
    if (n <= 0) return 0;
    if (n == 1) return 1;

    unsigned long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        unsigned long long c = a + b;
        a = b;
        b = c;
    }
    return b;
}

/* ---------- 斐波那契大数版 (数组模拟，支持 F(200)+) ---------- */
/* 用 int 数组存储大数，每个元素存一位十进制数字(低位在前)
 * 最多支持 MAX_DIGITS 位，F(200) 约 42 位，128 位足够 */
#define MAX_DIGITS 128

static void big_add(int a[], int b[], int result[])
{
    int carry = 0;
    for (int i = 0; i < MAX_DIGITS; i++) {
        int sum = a[i] + b[i] + carry;
        result[i] = sum % 10;
        carry = sum / 10;
    }
}

static void big_print(const int num[])
{
    /* 跳过前导零 */
    int i = MAX_DIGITS - 1;
    while (i > 0 && num[i] == 0) {
        i--;
    }
    for (; i >= 0; i--) {
        printf("%d", num[i]);
    }
}

static void big_copy(int dest[], const int src[])
{
    for (int i = 0; i < MAX_DIGITS; i++) {
        dest[i] = src[i];
    }
}

/* ---------- 斐波那契大数版 (Fibonacci up to F(200)+) ---------- */
static void fibonacci_big(int n, int result[])
{
    for (int i = 0; i < MAX_DIGITS; i++) result[i] = 0;

    if (n <= 0) {
        result[0] = 0;
        return;
    }
    if (n == 1) {
        result[0] = 1;
        return;
    }

    int a[MAX_DIGITS] = {0}, b[MAX_DIGITS] = {0};
    a[0] = 0;  /* F(0) = 0 */
    b[0] = 1;  /* F(1) = 1 */

    for (int i = 2; i <= n; i++) {
        big_add(a, b, result);
        big_copy(a, b);
        big_copy(b, result);
    }
}

/* ---------- 主入口 ---------- */
int main_algo_sample(void)
{
    printf("=== Algo Module: Common Algorithms ===\n\n");

    /* 1. 冒泡排序演示 */
    printf("[1] Bubble Sort Example\n");
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int n = sizeof(arr) / sizeof(arr[0]);
    printf("  Before: "); print_array(arr, n);
    bubble_sort(arr, n);
    printf("  After:  "); print_array(arr, n);
    printf("\n");

    /* 2. 二分查找演示 (需要有序数组) */
    printf("[2] Binary Search Example\n");
    int sorted[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    int target = 23;
    int result = binary_search(sorted, 10, target);
    if (result != -1)
        printf("  Found %d at index %d\n", target, result);
    else
        printf("  %d not found in array\n", target);
    printf("\n");

    /* 3. 斐波那契 — 两种实现对比 */
    printf("[3] Fibonacci Comparison (ull vs big number)\n");

    /* 3a. unsigned long long 版本 (仅到 F(90)，超限溢出) */
    printf("  --- unsigned long long 版本 (上限 F(92)) ---\n");
    printf("  F(0) = %llu\n", fibonacci_ull(0));
    printf("  F(1) = %llu\n", fibonacci_ull(1));
    printf("  F(10) = %llu\n", fibonacci_ull(10));
    printf("  F(50) = %llu\n", fibonacci_ull(50));
    printf("  F(90) = %llu\n", fibonacci_ull(90));
    printf("  (注意: F(93) 开始 unsigned long long 溢出\n\n");

    /* 3b. 大数版本 (支持 F(200)+) */
    printf("  --- Big Number 版本 (数组模拟，支持 F(200)+) ---\n");
    int fib_values[] = {0, 1, 10, 50,90, 100, 200};
    int count = sizeof(fib_values) / sizeof(fib_values[0]);
    int result_big[MAX_DIGITS];
    for (int i = 0; i < count; i++) {
        printf("  F(%d) = ", fib_values[i]);
        fibonacci_big(fib_values[i], result_big);
        big_print(result_big);
        printf("\n");
    }
    printf("\n");

    printf("=== Algo Module Complete ===\n");
    return 0;
}
