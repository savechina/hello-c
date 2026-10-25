#ifndef TODO_SAMPLE_H
#define TODO_SAMPLE_H

/**
 * @brief 终端 TODO 小项目 (Terminal TODO Mini-Project) — 真实项目篇
 *
 * 一个非交互的完整小项目演示 (非语法 demo):
 *   1. 数据结构   — Task / TodoStore, calloc 堆对象数组贯穿完整生命周期
 *   2. CRUD       — add (realloc 扩容) / list (表格) / toggle / remove
 * (swap+缩容) / find_by_id
 *   3. 持久化     — mkstemp 临时文件 + 简易二进制格式
 *                   (magic "CTDL" + version u32 + count u32 + Task 数组)
 *                   load 校验 magic/version/count, 坏数据 → "corrupt, skip"
 *   4. 工程纪律   — 参数校验 (int 错误码), errno, snprintf 路径, fclose 检查,
 *                   free 后置 NULL, task_new()/task_destroy() 工厂模式
 *
 * 不读 stdin, 全部由 demo 自身驱动; 结束时删除临时文件, 不留垃圾。
 * 调用方: main.c dispatch → main_awesome_sample() (orchestrator wiring)
 */
int main_todo_sample(void);

#endif /* TODO_SAMPLE_H */
