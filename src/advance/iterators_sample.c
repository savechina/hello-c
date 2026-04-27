#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "advance/iterators_sample.h"

/* ============================================================
 * Data Structure Traversal & Iterators Demo
 *
 * Demonstrates:
 *   1. Singly linked list (create, traverse, insert_after, delete_after, free)
 *   2. Doubly linked list
 *   3. Dynamic array (init, push, get, free)
 *   4. Binary tree traversal (pre-order, in-order, post-order)
 *   5. Iterator pattern (has_next + next)
 * ============================================================ */

/* ── 1. Singly Linked List ── */

typedef struct SNode {
    int32_t data;
    struct SNode *next;
} SNode;

static SNode *snode_new(int32_t value)
{
    SNode *n = malloc(sizeof(SNode));
    if (n == NULL) {
        return NULL;
    }
    n->data = value;
    n->next = NULL;
    return n;
}

static void slist_insert_after(SNode *prev, SNode *new_node)
{
    if (prev == NULL || new_node == NULL) {
        return;
    }
    new_node->next = prev->next;
    prev->next = new_node;
}

static void slist_delete_after(SNode *prev)
{
    if (prev == NULL || prev->next == NULL) {
        return;
    }
    SNode *to_remove = prev->next;
    prev->next = to_remove->next;
    free(to_remove);
}

static void slist_free_all(SNode *head)
{
    SNode *cur = head;
    while (cur != NULL) {
        SNode *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
}

static void demo_singly_linked_list(void)
{
    printf("=== 1. 单向链表 (Singly Linked List) ===\n");

    SNode *head = snode_new(10);
    if (head == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }
    head->next = snode_new(20);
    if (head->next == NULL) {
        slist_free_all(head);
        return;
    }
    head->next->next = snode_new(30);
    if (head->next->next == NULL) {
        slist_free_all(head);
        return;
    }

    printf("  创建链表: 10 -> 20 -> 30\n");

    printf("  遍历: ");
    for (SNode *cur = head; cur != NULL; cur = cur->next) {
        printf("%" PRId32 " -> ", cur->data);
    }
    printf("NULL\n");

    SNode *ins = snode_new(15);
    if (ins != NULL) {
        slist_insert_after(head, ins);
        printf("  在 10 后面插入 15: ");
        for (SNode *cur = head; cur != NULL; cur = cur->next) {
            printf("%" PRId32 " -> ", cur->data);
        }
        printf("NULL\n");
    }

    slist_delete_after(head);
    printf("  删除 15 后:       ");
    for (SNode *cur = head; cur != NULL; cur = cur->next) {
        printf("%" PRId32 " -> ", cur->data);
    }
    printf("NULL\n");

    printf("\n  内存布局:\n");
    printf("    head\n");
    printf("      |\n");
    printf("      v\n");
    printf("    [10|next] --> [20|next] --> [30|NULL]\n");

    slist_free_all(head);
    printf("\n");
}

/* ── 2. Doubly Linked List ── */

typedef struct DNode {
    int32_t data;
    struct DNode *prev;
    struct DNode *next;
} DNode;

static DNode *dnode_new(int32_t value)
{
    DNode *n = malloc(sizeof(DNode));
    if (n == NULL) {
        return NULL;
    }
    n->data = value;
    n->prev = NULL;
    n->next = NULL;
    return n;
}

static void demo_doubly_linked_list(void)
{
    printf("=== 2. 双向链表 (Doubly Linked List) ===\n");

    DNode *head = dnode_new(10);
    if (head == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }

    DNode *second = dnode_new(20);
    DNode *tail = dnode_new(30);
    if (second == NULL || tail == NULL) {
        free(head); free(second); free(tail);
        return;
    }

    head->next = second;  second->prev = head;
    second->next = tail;  tail->prev = second;

    printf("  正向遍历: ");
    for (DNode *cur = head; cur != NULL; cur = cur->next) {
        printf("%" PRId32 " <-> ", cur->data);
    }
    printf("NULL\n");

    printf("  反向遍历: ");
    for (DNode *cur = tail; cur != NULL; cur = cur->prev) {
        printf("%" PRId32 " <-> ", cur->data);
    }
    printf("NULL\n");

    printf("\n  内存布局:\n");
    printf("      head                                    tail\n");
    printf("        |                                       ^\n");
    printf("        v                                       |\n");
    printf("    [NULL|<->|10|<->] <-> [20] <-> [30|>->|NULL]\n");

    tail->prev->next = NULL;
    free(tail);
    free(second);
    free(head);
    printf("\n");
}

/* ── 3. Dynamic Array ── */

typedef struct {
    int32_t *data;
    size_t size;
    size_t capacity;
} DynArray;

static void dynarray_init(DynArray *arr)
{
    arr->capacity = 4;
    arr->size = 0;
    arr->data = malloc(arr->capacity * sizeof(int32_t));
}

static void dynarray_push(DynArray *arr, int32_t value)
{
    if (arr->size >= arr->capacity) {
        size_t new_cap = arr->capacity * 2;
        int32_t *tmp = realloc(arr->data, new_cap * sizeof(int32_t));
        if (tmp == NULL) {
            fprintf(stderr, "  Error: realloc failed\n");
            return;
        }
        arr->data = tmp;
        arr->capacity = new_cap;
    }
    arr->data[arr->size] = value;
    arr->size++;
}

static int32_t dynarray_get(const DynArray *arr, size_t index)
{
    if (index >= arr->size) {
        fprintf(stderr, "  Error: index %" PRIu64 " out of bounds (size=%" PRIu64 ")\n",
                (uint64_t)index, (uint64_t)arr->size);
        return -1;
    }
    return arr->data[index];
}

static void demo_dynamic_array(void)
{
    printf("=== 3. 动态数组 (Dynamic Array) ===\n");

    DynArray arr;
    dynarray_init(&arr);
    if (arr.data == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }

    printf("  初始: capacity=%zu, size=%zu\n", arr.capacity, arr.size);

    for (int32_t i = 0; i < 6; i++) {
        dynarray_push(&arr, (i + 1) * 10);
    }

    printf("  Push 6 个元素后: capacity=%zu, size=%zu\n", arr.capacity, arr.size);

    printf("  内容: ");
    for (size_t i = 0; i < arr.size; i++) {
        printf("%" PRId32 " ", arr.data[i]);
    }
    printf("\n");

    printf("\n  扩容过程:\n");
    printf("    capacity=4, size=4        capacity=8, size=6\n");
    printf("    [10,20,30,40]  --realloc翻倍-->  [10,20,30,40,50,60,  ,  ]\n");

    free(arr.data);
    printf("\n");
}

/* ── 4. Binary Tree Traversal ── */

typedef struct TreeNode {
    int32_t data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

static TreeNode *tree_new(int32_t value)
{
    TreeNode *n = malloc(sizeof(TreeNode));
    if (n == NULL) {
        return NULL;
    }
    n->data = value;
    n->left = NULL;
    n->right = NULL;
    return n;
}

static void tree_free_all(TreeNode *node)
{
    if (node == NULL) {
        return;
    }
    tree_free_all(node->left);
    tree_free_all(node->right);
    free(node);
}

static void preorder_print(TreeNode *node)
{
    if (node == NULL) {
        return;
    }
    printf("%" PRId32 " ", node->data);
    preorder_print(node->left);
    preorder_print(node->right);
}

static void inorder_print(TreeNode *node)
{
    if (node == NULL) {
        return;
    }
    inorder_print(node->left);
    printf("%" PRId32 " ", node->data);
    inorder_print(node->right);
}

static void postorder_print(TreeNode *node)
{
    if (node == NULL) {
        return;
    }
    postorder_print(node->left);
    postorder_print(node->right);
    printf("%" PRId32 " ", node->data);
}

static void demo_binary_tree(void)
{
    printf("=== 4. 二叉树遍历 (Binary Tree Traversal) ===\n");

    TreeNode *root = tree_new(1);
    root->left = tree_new(2);
    root->right = tree_new(3);
    if (root->left == NULL || root->right == NULL) {
        tree_free_all(root);
        return;
    }
    root->left->left = tree_new(4);
    root->left->right = tree_new(5);
    root->right->right = tree_new(6);
    if (root->left->left == NULL || root->left->right == NULL ||
        root->right->right == NULL) {
        tree_free_all(root);
        return;
    }

    printf("\n  树结构:\n");
    printf("        1\n");
    printf("       / \\\n");
    printf("      2   3\n");
    printf("     / \\   \\\n");
    printf("    4   5   6\n\n");

    printf("  前序 (根->左->右): ");
    preorder_print(root);
    printf("\n");
    printf("  中序 (左->根->右): ");
    inorder_print(root);
    printf("\n");
    printf("  后序 (左->右->根): ");
    postorder_print(root);
    printf("\n");

    tree_free_all(root);
    printf("\n");
}

/* ── 5. Iterator Pattern ── */

typedef struct {
    const DynArray *arr;
    size_t index;
} ArrayIterator;

static void iter_init(ArrayIterator *iter, const DynArray *arr)
{
    iter->arr = arr;
    iter->index = 0;
}

static int iter_has_next(const ArrayIterator *iter)
{
    return iter->index < iter->arr->size;
}

static int32_t iter_next(ArrayIterator *iter)
{
    return iter->arr->data[iter->index++];
}

static void demo_iterator_pattern(void)
{
    printf("=== 5. 迭代器模式 (Iterator Pattern) ===\n");

    DynArray arr;
    dynarray_init(&arr);
    if (arr.data == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }

    for (int32_t i = 0; i < 5; i++) {
        dynarray_push(&arr, (i + 1) * 100);
    }

    printf("  创建 [100, 200, 300, 400, 500]\n");

    ArrayIterator it;
    iter_init(&it, &arr);

    printf("  迭代器输出: ");
    while (iter_has_next(&it)) {
        printf("%" PRId32 " ", iter_next(&it));
    }
    printf("\n");

    printf("\n  就像读手指:\n");
    printf("    index=0    has_next()=1    next()=100\n");
    printf("    index=1    has_next()=1    next()=200\n");
    printf("    index=2    has_next()=1    next()=300\n");
    printf("    index=3    has_next()=1    next()=400\n");
    printf("    index=4    has_next()=1    next()=500\n");
    printf("    index=5    has_next()=0           <- 遍历结束\n");

    free(arr.data);
    printf("\n");
}

/* ── Coordinator entry ── */

int main_iterators_sample(void)
{
    printf("========================================\n");
    printf("  数据结构遍历 (Data Structure Traversal)\n");
    printf("========================================\n\n");

    demo_singly_linked_list();
    demo_doubly_linked_list();
    demo_dynamic_array();
    demo_binary_tree();
    demo_iterator_pattern();

    printf("数据结构遍历演示完毕。\n");
    return 0;
}
