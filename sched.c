#include <stdlib.h>

#include "sched.h"

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
	task_t _main;

	if (prev == NULL) {
		prev = &_main;
	}

	next = list_entry(list_pop(&task_list), task_t, list);
	current = next;
	counter++;
	swapcontext(&prev->context, &next->context);
}


void co_yield()
{
	list_add(&task_list, &current->list);
	scheduler();
}
