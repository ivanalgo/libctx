#ifndef _TASK_H_
#define _TASK_H_

#include <ucontext.h>

#include "list.h"

typedef struct {
	void * (*func)(void *arg);
	void *arg;
	ucontext_t context;

	/* list node for all tasks */
	struct list list;
} task_t;

extern void co_yield();
extern int co_create(void* (*fn)(void*), void *arg);
#endif /* _TASK_H_ */
