#include <stdio.h>
#include "basic/control_flow_sample.h"
#include <stdint.h>

/**
 * 控制流样例: if/else 判断、switch 分支与三元运算符。
 */

void if_else_sample(void) {
    printf("----basic ... if_else_sample.....start ----\n");

    int32_t score = 75;
    if (score >= 90) {
        printf("  优秀 (A)\n");
    } else if (score >= 60) {
        printf("  及格 (C) - 分数: %d\n", score);
    } else {
        printf("  不及格 (F)\n");
    }

    int32_t year = 2024;
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        printf("  %d 是闰年 (leap year)\n", year);
    } else {
        printf("  %d 是平年 (common year)\n", year);
    }

    printf("----basic ... if_else_sample.....done ----\n");
}

void switch_sample(void) {
    printf("----basic ... switch_sample.....start ----\n");

    int32_t choice = 2;
    switch (choice) {
        case 1:
            printf("  选择: 开始游戏 (New Game)\n");
            break;
        case 2:
            printf("  选择: 继续游戏 (Continue)\n");
            break;
        case 3:
            printf("  选择: 设置 (Settings)\n");
            break;
        default:
            printf("  无效选项！\n");
            break;
    }

    printf("----basic ... switch_sample.....done ----\n");
}

void ternary_sample(void) {
    printf("----basic ... ternary_sample.....start ----\n");

    int32_t x = 10, y = 20;
    int32_t max = (x > y) ? x : y;
    printf("  max(%d, %d) = %d\n", x, y, max);

    int32_t age = 20;
    const char *label = (age >= 18) ? "adult" : "minor";
    printf("  age %d -> %s\n", age, label);

    printf("----basic ... ternary_sample.....done ----\n");
}

void control_flow_sample(void) {
    if_else_sample();
    switch_sample();
    ternary_sample();
}

void main_control_flow_sample(void) {
    if_else_sample();
    switch_sample();
    ternary_sample();
}
