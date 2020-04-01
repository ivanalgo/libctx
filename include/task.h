#ifndef _TASK_H_
#define _TASK_H_

#include "context.h"
#include "list.h"
#include "lock.h"

typedef struct {
	void * (*func)(void *arg);
	void *arg;
	ucontext_t context;
	int flags;

	/* for task exited */
	notify_t exit_notify;
	void *exit_state;

	/* list node for all tasks */
	struct list list;
} task_t;

extern task_t *current;
#define TASK_RUNNING 	0x00
#define TASK_EXIT	0x01
#define TASK_SLEEP	0x02

#define TASK_STACK_SIZE		(64 * 1024)

extern void co_yield();
extern task_t * co_create(void* (*fn)(void*), void *arg);
extern int co_wait(task_t *task, void **ret);
extern void co_sleep(void);
extern void co_wakeup(task_t *task);

#endif /* _TASK_H_ */
