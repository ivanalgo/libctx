#include <stdlib.h>
#include <stdio.h>

#include "list.h"
#include "task.h"
#include "sched.h"
#include "context.h"
#include "lock.h"
#include "debug.h"

void co_task_func(task_t *task)
{
	current->func(task->arg);
	task->flags = TASK_EXIT;
	current->exit_state = (void *)1;
	task_exit(current);	
	debug_log("exited: %p %p\n", task, current);
	scheduler();
	assert(1);
}

task_t *co_create(void * (*fn)(void *arg), void *arg)
{
	void *stack = malloc(TASK_STACK_SIZE);
	if (!stack)
		return (task_t *) -1;

	task_t *task = malloc(sizeof(task_t));
	if (!task) {
		free(stack);
		return (task_t *)-1;
	}

	getcontext(&task->context);
	task->context.uc_stack.ss_sp = stack;
	task->context.uc_stack.ss_size = TASK_STACK_SIZE;
	task->func = fn;
	task->arg = arg;
	task->flags = TASK_RUNNING;
	co_notify_init(&task->exit_notify);
	makecontext(&task->context, (void (*)(void))co_task_func, 1, task);

	create_new_task(task);

	scheduler();

	return task;
}

void co_destroy(task_t *task)
{
	free(task);
}

int co_wait(task_t *task, void **ret)
{
	co_notify_wait(&task->exit_notify);
	if (ret)
		*ret = task->exit_state;
	co_destroy(task);

	return 0;
}
