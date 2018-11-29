#include <ucontext.h>
#include <stdlib.h>

#include "list.h"
#include "task.h"
#include "sched.h"

void co_task_func(task_t *task)
{
	current->func(task->arg);
}

int co_create(void * (*fn)(void *arg), void *arg)
{
	void *stack = malloc(64 * 1024);
	if (!stack)
		return -1;

	task_t *task = malloc(sizeof(task_t));
	if (!task) {
		free(stack);
		return -1;
	}

	getcontext(&task->context);
	task->context.uc_stack.ss_sp = stack;
	task->context.uc_stack.ss_size = 64 * 1026;
	task->func = fn;
	task->arg = arg;
	makecontext(&task->context, (void (*)(void))co_task_func, 1, task);

	create_new_task(task);

	return 0;
}
