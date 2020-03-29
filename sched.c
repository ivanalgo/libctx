#include <stdlib.h>
#include <stdio.h>

#include "sched.h"
#include "task.h"
#include "debug.h"

struct list task_list;
task_t init;
task_t *current = NULL;

static inline void handle_main()
{
	if (!current) {
		current = &init;
		current->flags = TASK_RUNNING;
	}
}

void create_new_task(task_t *task)
{
	debug_log("new task: %p\n", task);
	list_add(&task_list, &task->list);
}

void sched_init() __attribute__((constructor));
void sched_init()
{
	list_init(&task_list);
	handle_main();
}

struct list exited_tasks = { .next = &exited_tasks, .prev = &exited_tasks };
void task_exit(task_t *task)
{
	co_notify_complete(&task->exit_notify);
}

void scheduler(void)
{
	static int counter = 0;
	task_t *next;
       	task_t *prev = current;

	if (current->flags == TASK_RUNNING) {
		list_add(&task_list, &current->list);
	}

	next = list_entry(list_pop(&task_list), task_t, list);

	if (!next) {
		return;
	}

	if (next == prev)
		return;

	debug_log("sched: %p (%d) -> %p (%d)\n", prev, prev->flags, next, next->flags);
	current = next;
	counter++;
	swapcontext(&prev->context, &next->context);
}


void co_yield()
{
	current->flags = TASK_RUNNING;
	scheduler();
}

void co_sleep()
{
	scheduler();
}

void co_wakeup(task_t *t)
{
	debug_log("wakeup: %p\n", t);
	t->flags = TASK_RUNNING;
	list_add(&task_list, &t->list);
}
