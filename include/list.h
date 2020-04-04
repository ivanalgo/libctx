#ifndef _LIST_H_
#define _LIST_H_

#include "stdlib.h"

struct list {
	struct list *next;
	struct list *prev;
};

static inline void list_init(struct list *head)
{
	head->next = head->prev = head;
}

static inline void __list_add(struct list *new,
			      struct list *prev,
			      struct list *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list *head, struct list*node)
{
	__list_add(node, head->prev, head);
}

static inline int list_empty(struct list *head)
{
	return head->next == head;
}

static inline void list_del(struct list *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
}

static inline struct list * list_pop(struct list *head)
{
	struct list *node = head->next;

	if (list_empty(head))
		return NULL;

	list_del(node);

	return node;
}

#define container_of(ptr, type, member)				\
	((type *)((unsigned long)ptr - ((unsigned long)&(((type *)0)->member))))

#define list_entry(ptr, type, member)				\
	container_of(ptr, type, member)

#define list_for_each(node, head)	\
	for (node = head->next; node != head; node = node->next)

static inline int list_counter(struct list *head)
{
	int counter = 0;
	struct list *node;

	list_for_each(node, head) {
		counter++;
	}

	return counter;
}

#endif /* _LIST_H_ */
