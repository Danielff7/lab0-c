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
    /*element_t *e, *prev = NULL;
    for (e = list_entry(l->next, element_t, list); &e->list != l;
         e = list_entry(e->list.next, element_t, list)) {
        // list_for_each_entry (e, l, list) {
        if (prev) {
            free(prev->value);
            free(prev);
        }
        prev = e;
    }
    if (prev) {
        free(prev->value);
        free(prev);
    }*/
    list_for_each_entry_safe (e, next, l, list) {
        q_release_element(e);
    }
    free(l);
    // INIT_LIST_HEAD(l);
}

static element_t *element_new(char *s)
{
    element_t *new_e = (element_t *) malloc(sizeof(element_t));
    if (new_e) {
        new_e->value =
            strdup(s);  //(char *) malloc((bufsize + 1) * sizeof(char));
        if (!new_e->value) {
            free(new_e);
            new_e = NULL;
        }
    }
    return new_e;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *new_e = element_new(s);
    bool ret = false;
    if (new_e) {
        list_add(&new_e->list, head);
        ret = true;
    }
    return ret;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *new_e = element_new(s);
    bool ret = false;
    if (new_e) {
        list_add_tail(&new_e->list, head);
        ret = true;
    }
    return ret;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *removed = head->next;
    list_del_init(removed);
    element_t *r = list_entry(removed, element_t, list);
    if (sp && bufsize > 0) {
        size_t srclen = strlen(r->value);
        size_t copylen = (bufsize - 1 <= srclen) ? bufsize - 1 : srclen;
        (void *) strncpy(sp, r->value, copylen);
        sp[copylen] = '\0';
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
    if (sp && bufsize > 0) {
        size_t srclen = strlen(r->value);
        size_t copylen = (bufsize - 1 <= srclen) ? bufsize - 1 : srclen;
        (void *) strncpy(sp, r->value, copylen);
        sp[copylen] = '\0';
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
    struct list_head *prev = (*indirect)->prev;
    *indirect = (*indirect)->next;
    (*indirect)->prev = prev;
    element_t *e = list_entry(tobefree, element_t, list);
    free(e->value);
    free(e);

    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    struct list_head **begin = &head->next, **end = &head->next;
    element_t *stack[100000] = {NULL};
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
            while (index >= 0) {
                free(stack[index]->value);
                free(stack[index--]);
            }
        } else {
            begin = end;
        }
        index = -1;
    }
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head)
        return;
    struct list_head **indirect = &head->next;
    while (*indirect != head && (*indirect)->next != head) {
        struct list_head *prev = (*indirect)->prev;
        struct list_head *old = *indirect, *old_next;

        (*indirect) = (*indirect)->next;
        (*indirect)->prev = prev;
        prev->next = *indirect;
        old_next = (*indirect)->next;
        (*indirect)->next = old;
        old->prev = *indirect;
        old->next = old_next;
        old_next->prev = old;
        indirect = &old->next;
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
        first->next = next->next;
        first->next->prev = first;
        next->next = head->next;
        next->next->prev = next;
        head->next = next;
        next->prev = head;
    }
}

static struct list_head *merge_two_list(struct list_head *l1,
                                        struct list_head *l2,
                                        struct list_head **tail)
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

    *indirect = (l1) ? l1 : l2;
    (*indirect)->prev = prev;
    while (*indirect && (*indirect)->next) {
        indirect = &(*indirect)->next;
    }
    *tail = *indirect;
    return head;
}

static struct list_head *merge_sort(struct list_head *head,
                                    struct list_head **tail)
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
    struct list_head *sort_l = merge_sort(head, tail);
    struct list_head *sort_r = merge_sort(new_head, tail);
    return merge_two_list(sort_l, sort_r, tail);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *tail;
    struct list_head *first = head->next;
    first->prev = NULL;
    head->prev->next = NULL;
    struct list_head *res = merge_sort(first, &tail);
    head->next = res;
    res->prev = head;
    head->prev = tail;
    tail->next = head;
}
