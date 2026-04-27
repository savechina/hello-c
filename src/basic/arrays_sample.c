#include <stdio.h>
#include "basic/arrays_sample.h"
#include <stdint.h>

/**
 * 数组样例: 一维数组的声明、初始化、遍历与常用算法。
 */

void arrays_decl_init_sample(void) {
    printf("----basic ... arrays_decl_init_sample.....start ----\n");

    int32_t scores[5] = {85, 92, 78, 96, 88};
    int32_t count = (int32_t)(sizeof(scores) / sizeof(scores[0]));

    printf("数组元素: ");
    for (int32_t i = 0; i < count; i++) {
        printf("%d ", scores[i]);
    }
    printf("\n");

    int32_t temps[] = {23, 25, 19, 28};
    int32_t temp_count = (int32_t)(sizeof(temps) / sizeof(temps[0]));
    printf("自动推导长度: %d 个元素\n", temp_count);

    printf("----basic ... arrays_decl_init_sample.....done ----\n");
}

void arrays_algorithm_sample(void) {
    printf("----basic ... arrays_algorithm_sample.....start ----\n");

    int32_t data[] = {64, 34, 25, 12, 22, 11, 90};
    int32_t n = (int32_t)(sizeof(data) / sizeof(data[0]));

    int64_t sum = 0;
    int32_t min_val = data[0];
    int32_t max_val = data[0];
    for (int32_t i = 0; i < n; i++) {
        sum += data[i];
        if (data[i] < min_val) min_val = data[i];
        if (data[i] > max_val) max_val = data[i];
    }
    double avg = (double)sum / n;

    printf("数组: ");
    for (int32_t i = 0; i < n; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
    printf("总和: %lld, 平均值: %.1f\n", (long long)sum, avg);
    printf("最小值: %d, 最大值: %d\n", min_val, max_val);

    printf("----basic ... arrays_algorithm_sample.....done ----\n");
}

void arrays_sample(void) {
    arrays_decl_init_sample();
    arrays_algorithm_sample();
}

void main_arrays_sample(void) {
    arrays_decl_init_sample();
    arrays_algorithm_sample();
}
