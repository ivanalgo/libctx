#include <stdlib.h>
#include <stdio.h>

#include "sched.h"
#include "task.h"

struct list task_list;
task_t *current = NULL;

void create_new_task(task_t *task)
{
	list_add(&task_list, &task->list);
}

void sched_init()
{
	list_init(&task_list);
}

void scheduler(void)
{
	static int counter = 0;
	task_t *next;
       	task_t *prev = current;

	if (list_empty(&task_list)) {
		printf("not task to sched, exiting...\n");
		exit(1);
	}
	next = list_entry(list_pop(&task_list), task_t, list);
	current = next;
	counter++;
	if (!prev || prev->flags != TASK_RUNNING)
		setcontext(&next->context);
	else
		swapcontext(&prev->context, &next->context);
}


void co_yield()
{
	list_add(&task_list, &current->list);
	scheduler();
}
