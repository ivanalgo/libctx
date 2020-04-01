#include <pthread.h>
#include <stdio.h>

#include "task.h"

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg)
{
	task_t *t;

	t = co_create(start_routine, arg);
	if ((long) t < 0) {
		return -1;
	}

	*thread = (pthread_t)t;
	return 0;
}

int pthread_yield(void)
{
	co_yield();

	return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
	task_t *t = (task_t *)thread;
	co_wait(t, retval);
	return 0;
}
