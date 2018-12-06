#ifndef _TASK_H_
#define _TASK_H_

#include "context.h"
#include "list.h"

typedef struct {
	void * (*func)(void *arg);
	void *arg;
	ucontext_t context;
	int flags;

	/* list node for all tasks */
	struct list list;
} task_t;

#define TASK_RUNNING 	0x00
#define TASK_EXIT	0x01

#define TASK_STACK_SIZE		(64 * 1024)

extern void co_yield();
extern int co_create(void* (*fn)(void*), void *arg);
#endif /* _TASK_H_ */
