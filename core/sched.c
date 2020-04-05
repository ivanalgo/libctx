#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <ucontext.h>

#include "sched.h"
#include "task.h"
#include "debug.h"

struct list task_list;
spin_t task_list_lock;

__thread task_t *current = NULL;

task_t main_co;

/* raw functions */
int (*p_pthread_create)(pthread_t *id, const pthread_attr_t *attr,
			void *(*func)(void *arg),  void *arg);
pthread_t (*p_pthread_self)(void);
void (*p_makecontext)(ucontext_t *ucp, void (*func)(), int argc);
int (*p_getcontext)(ucontext_t *ucp);
int (*p_setcontext)(ucontext_t *ucp);

void raw_functions_init()
{
	p_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
	assert(p_pthread_create);
	p_pthread_self = dlsym(RTLD_NEXT, "pthread_self");
	assert(p_pthread_self);
	p_makecontext = dlsym(RTLD_NEXT, "makecontext");
	assert(p_makecontext);
	p_getcontext = dlsym(RTLD_NEXT, "getcontext");
	assert(p_getcontext);
	p_setcontext = dlsym(RTLD_NEXT, "setcontext");
	assert(p_setcontext);
}

void create_new_task(task_t *task)
{
	debug_log("new task: %p\n", task);
	spin_lock(&task_list_lock);
	list_add(&task_list, &task->list);
	spin_unlock(&task_list_lock);
}

#define VCPU_NUM	2
pthread_t vcpu[VCPU_NUM];
task_t idle[VCPU_NUM];

void * vcpu_idle(void *arg)
{
	int cpuid = (int)(long)arg;

	while (1) {
		int has_task;
		task_t *task;

		spin_lock(&task_list_lock);
		has_task = !list_empty(&task_list);
		if (has_task) {
			task = list_entry(list_pop(&task_list), task_t, list);
		}
		spin_unlock(&task_list_lock);

		if (has_task) {
			current = task;
			current->cpu = cpuid;
			printf("vcpu dile %d\n", cpuid);
			lwt_swapcontext(&idle[current->cpu].context, &current->context);
		}
	}
}

void vcpu_idle0()
{
	int i;

	for (i = 1; i < VCPU_NUM; i++) {
		p_pthread_create(vcpu + i, NULL, vcpu_idle, (void *)(long)i);
	}

	vcpu_idle(0);
}

void init_idle0(task_t *idle0)
{
	ucontext_t context;

        void *stack = malloc(TASK_STACK_SIZE);
        if (!stack)
                exit(1);

        p_getcontext(&context);
        context.uc_stack.ss_sp = stack;
        context.uc_stack.ss_size = TASK_STACK_SIZE;

	/* add main co to task_list */
	spin_lock(&task_list_lock);
	list_add(&task_list, &main_co.list);
	spin_unlock(&task_list_lock);

	{
		static int in_main_co = 0;

		/* save this linux stack context as context of main coroutine */
		lwt_getcontext(&main_co.context);

		if (in_main_co)
			return;

		/* change the in_main_co as 1, then the idle0 will not go through */
		in_main_co = 1;

		/* switch this linux thread to vcpu idle function */
		p_makecontext(&context, vcpu_idle0, 0);
		p_setcontext(&context);
	}
}

void sched_init() __attribute__((constructor));
void sched_init()
{
	list_init(&task_list);
	spin_init(&task_list_lock);

	raw_functions_init();

	vcpu[0] =p_pthread_self();
	init_idle0(&idle[0]);


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
	struct list *node;
	int cpu = current->cpu;

	spin_lock(&task_list_lock);

	if (current->flags == TASK_RUNNING) {
		list_add(&task_list, &current->list);
	}

	node = list_pop(&task_list);
	if (node) {
		next = list_entry(node, task_t, list);
	} else {
		next = &idle[current->cpu];
	}

	spin_unlock(&task_list_lock);

	if (!next) {
		return;
	}

	if (next == prev)
		return;

	next->cpu = cpu;
	debug_log("sched: %p (%d) -> %p (%d)\n", prev, prev->flags, next, next->flags);
	current = next;
	counter++;
	lwt_swapcontext(&prev->context, &next->context);
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
