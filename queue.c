#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (new) {
        INIT_LIST_HEAD(new);
    }
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *e, *next;
    list_for_each_entry_safe (e, next, l, list) {
        q_release_element(e);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *new_e = (element_t *) malloc(sizeof(element_t));
    if (!new_e) {
        return false;
    }
    new_e->value = strdup(s);
    if (!new_e->value) {
        free(new_e);
        return false;
    }
    list_add(&new_e->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *new_e = (element_t *) malloc(sizeof(element_t));
    if (!new_e) {
        return false;
    }
    new_e->value = strdup(s);
    if (!new_e->value) {
        free(new_e);
        return false;
    }
    list_add_tail(&new_e->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *removed = head->next;
    list_del_init(removed);
    element_t *r = list_entry(removed, element_t, list);
    if (sp) {
        (void *) strncpy(sp, r->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return r;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *removed = head->prev;
    list_del_init(removed);
    element_t *r = list_entry(removed, element_t, list);
    if (sp) {
        (void *) strncpy(sp, r->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return r;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *n;
    int size = 0;
    list_for_each (n, head) {
        size++;
    }
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head **indirect = &head->next;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        indirect = &(*indirect)->next;
    }
    struct list_head *tobefree = *indirect;
    list_del_init(tobefree);
    element_t *e = list_entry(tobefree, element_t, list);
    q_release_element(e);
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    struct list_head **begin = &head->next, **end = &head->next;
    element_t *stack[10000] = {NULL};
    int index = -1;
    while (*begin != head && (*begin)->next != head) {
        element_t *b = list_entry(*begin, element_t, list);
        while (*end != head) {
            element_t *e = list_entry(*end, element_t, list);
            if (!strcmp(b->value, e->value)) {
                end = &(*end)->next;
                stack[++index] = e;
            } else {
                break;
            }
        }
        if (*end != (*begin)->next) {
            struct list_head *prev = (*begin)->prev;
            *begin = *end;
            (*begin)->prev = prev;
            end = begin;
        } else {
            begin = end;
            index--;
        }
    }
    while (index >= 0) {
        q_release_element(stack[index--]);
    }
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *cur = head->next;
    while (cur != head && cur->next != head) {
        struct list_head *next = cur->next;
        list_del(cur);
        list_add(cur, next);
        cur = cur->next;
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *first = head->next;
    while (first->next != head) {
        struct list_head *next = first->next;
        list_del(next);
        list_add(next, head);
    }
}

static struct list_head *merge_two_list(struct list_head *l1,
                                        struct list_head *l2)
{
    struct list_head *head = NULL, *prev = NULL, **indirect = &head;
    for (struct list_head **node; l1 && l2; *node = (*node)->next) {
        element_t *e1 = list_entry(l1, element_t, list),
                  *e2 = list_entry(l2, element_t, list);
        if (strcmp(e1->value, e2->value) <= 0) {
            node = &l1;
        } else {
            node = &l2;
        }
        *indirect = *node;
        (*indirect)->prev = prev;
        prev = *indirect;
        indirect = &(*indirect)->next;
    }

    *indirect = (struct list_head *) ((uintptr_t) l1 | (uintptr_t) l2);
    (*indirect)->prev = prev;
    return head;
}

static struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head **slow = &head;
    for (struct list_head *fast = head; fast && fast->next;
         fast = fast->next->next) {
        slow = &(*slow)->next;
    }
    struct list_head *new_head = *slow;
    *slow = NULL;
    new_head->prev = NULL;  // might not needed
    struct list_head *sort_l = merge_sort(head);
    struct list_head *sort_r = merge_sort(new_head);
    return merge_two_list(sort_l, sort_r);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *first = head->next;
    first->prev = NULL;
    head->prev->next = NULL;
    struct list_head *res = merge_sort(first);
    head->next = res;
    res->prev = head;
    while (res && res->next) {
        res = res->next;
    }
    head->prev = res;
    if (res)
        res->next = head;
}
