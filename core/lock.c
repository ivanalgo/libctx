#include <assert.h>
#include <stdlib.h>
#include "list.h"
#include "lock.h"
#include "sched.h"

void co_mutex_init(mutex_t*mutex)
{
	spin_init(&mutex->spin_lock);
	mutex->value = 1;
	list_init(&mutex->waiter);
}

void co_mutex_lock(mutex_t *mutex)
{
	spin_lock(&mutex->spin_lock);

	if (mutex->value == 1) {
		mutex->value = -1;
		spin_unlock(&mutex->spin_lock);
		return;
	}

	/* lock failed, put itself to the waiter list */
	list_add(&mutex->waiter, &current->list);

	current->flags = TASK_SLEEP;
	spin_unlock(&mutex->spin_lock);
	co_sleep();
}

void co_mutex_unlock(mutex_t *mutex)
{
	task_t *task = NULL;

	spin_lock(&mutex->spin_lock);

	assert(mutex->value == -1);
	if (!list_empty(&mutex->waiter)) {
		task = list_entry(list_pop(&mutex->waiter), task_t, list);
	}

	mutex->value = 1;
	spin_unlock(&mutex->spin_lock);

	if (task)
		co_wakeup(task);

	return;
}

void co_notify_init(notify_t *notify)
{
	co_mutex_init(&notify->lock);
	list_init(&notify->waiter);
	notify->value = -1;
}

void co_notify_wait(notify_t *notify)
{
	co_mutex_lock(&notify->lock);

	if (notify->value > 0) {
		co_mutex_unlock(&notify->lock);
		return;
	}

	list_add(&notify->waiter, &current->list);
	current->flags = TASK_SLEEP;
	co_mutex_unlock(&notify->lock);
	co_sleep();
}

void co_notify_complete(notify_t *notify)
{
	co_mutex_lock(&notify->lock);
	notify->value = 1;

	while (!list_empty(&notify->waiter)) {
		task_t *task = list_entry(list_pop(&notify->waiter), task_t, list);
		co_wakeup(task);
	}

	co_mutex_unlock(&notify->lock);
}
