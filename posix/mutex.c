#include <pthread.h>
#include <limits.h>
#include "list.h"
#include "task.h"
#include "sched.h"
#include "debug.h"

/*
 * Just implement a normal and timeout mutex
 * no RECURSIVE, ERRORCHECK, robust, realtime mutex
 *
 */

/* futex object hash mgnt */

struct futex_obj {
	void *addr;
	spin_t lock;	
	struct list chain_list;
	struct list wait_list;
};

#define FUTEX_HASH_BUCKETS	1023

struct futex_hash {
	struct list buckets[FUTEX_HASH_BUCKETS];
} futex_hash;

void futex_hash_init() __attribute__((constructor));
void futex_hash_init()
{
	int i;
	for (i = 0; i < FUTEX_HASH_BUCKETS; i++) {
		list_init(&futex_hash.buckets[i]);
	}
}

unsigned int futex_key(void *addr)
{
	return ((unsigned long)addr) % FUTEX_HASH_BUCKETS;
}

struct futex_obj * futex_hash_get_obj(void *addr)
{
	int key = (unsigned long)addr % FUTEX_HASH_BUCKETS;
	struct list *list = &futex_hash.buckets[key];
	struct list *node;
	struct futex_obj *obj = NULL;

	list_for_each(node, list) {
		obj = list_entry(node, struct futex_obj, chain_list);
		if (obj->addr == addr)
			break;
	}

	if (!obj) {
		obj = malloc(sizeof(struct futex_obj));
		assert(obj);
		obj->addr = addr;	
		spin_init(&obj->lock);
		list_add(list, &obj->chain_list);
		list_init(&obj->wait_list);
	}

	return obj;
}

void lll_futex_wait(int *futex, int val)
{
	struct futex_obj *obj = futex_hash_get_obj(futex);
	int fval;

	spin_lock(&obj->lock);
	fval = *futex;

	/* futex val has been changed, just return */
	if (fval != val) {
		spin_unlock(&obj->lock);
		return;
	}

	current->flags = TASK_SLEEP;
	list_add(&obj->wait_list, &current->list);
	spin_unlock(&obj->lock);

	debug_log("mutex-sleep: %p on mutex %p, total waiter: %d\n",
		current, futex, list_counter(&obj->wait_list));
	co_sleep();
}

void lll_futex_wake(int *futex, int nr_wake)
{
	struct futex_obj *obj = futex_hash_get_obj(futex);
	struct list *node;
	task_t *task;
	int i;

	spin_lock(&obj->lock);

	for (i = 0; i < nr_wake; i++) {
		node = list_pop(&obj->wait_list);
		if (!node)
			goto out;
	
		task = list_entry(node, task_t, list);
		debug_log("mutex-wake: task %p on mutex %p, remainder waiter: %d\n",
			task, futex, list_counter(&obj->wait_list));
		task->flags = TASK_RUNNING;
		co_wakeup(task);
	}

out:
	spin_unlock(&obj->lock);
}

int pthread_mutex_init(pthread_mutex_t *restrict mutex,
              const pthread_mutexattr_t *restrict attr)
{
	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	return 0;
}

static inline void lll_lock(int *futex)
{
	if (!__sync_bool_compare_and_swap(futex, 0, 1))
	{
		if (*futex == 2) {
			lll_futex_wait(futex, 2);
		}
								
		while (__atomic_exchange_n(futex, 2, __ATOMIC_ACQUIRE) != 0) {
			lll_futex_wait(futex, 2);
		}
	}
}

static inline void lll_unlock(int *futex)
{
	int oval;

	oval = __atomic_exchange_n(futex, 0, __ATOMIC_RELEASE);
	if (oval > 1) {
		lll_futex_wake(futex, 1);
	}
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	int type = mutex->__data.__kind;

	assert(type == 0);

	lll_lock(&mutex->__data.__lock);

	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	mutex->__data.__owner = 0;
	lll_unlock(&mutex->__data.__lock);

	return 0;
}

#define COND_NWAITERS_SHIFT	1

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	int val, seq, bc_seq;

	lll_lock(&cond->__data.__lock);
	pthread_mutex_unlock(mutex);

  	++cond->__data.__total_seq;
  	++cond->__data.__futex;
	cond->__data.__nwaiters += 1 << COND_NWAITERS_SHIFT;

  	if (cond->__data.__mutex != (void *) ~0l)
    		cond->__data.__mutex = mutex;

	val = seq = cond->__data.__wakeup_seq;
	bc_seq = cond->__data.__broadcast_seq;

	do {
		int futex_val = cond->__data.__futex;
		lll_unlock (&cond->__data.__lock);
		lll_futex_wait ((int *)&cond->__data.__futex, futex_val);

		lll_lock (&cond->__data.__lock);

		if (bc_seq != cond->__data.__broadcast_seq)
			goto bc_out;

		val = cond->__data.__wakeup_seq;
	} while (val == seq || cond->__data.__woken_seq == val);

  	/* Another thread woken up.  */
  	++cond->__data.__woken_seq;

bc_out:
	cond->__data.__nwaiters -= 1 << COND_NWAITERS_SHIFT;

  	/* If pthread_cond_destroy was called on this varaible already,
     	notify the pthread_cond_destroy caller all waiters have left
     	and it can be successfully destroyed.  */
  	if (cond->__data.__total_seq == -1ULL
      		&& cond->__data.__nwaiters < (1 << COND_NWAITERS_SHIFT))
    		lll_futex_wake ((int *)&cond->__data.__nwaiters, 1);

  	/* We are done with the condvar.  */
  	lll_unlock (&cond->__data.__lock);
	
	return pthread_mutex_lock (mutex);
}

int pthread_cond_signal(pthread_cond_t *cond)
{
	lll_lock(&cond->__data.__lock);

	if (cond->__data.__total_seq > cond->__data.__wakeup_seq) {
		++cond->__data.__wakeup_seq;
		++cond->__data.__futex;
		lll_futex_wake ((int *)&cond->__data.__futex, 1);
	}
	lll_unlock(&cond->__data.__lock);

	return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
	lll_lock (&cond->__data.__lock);
	if (cond->__data.__total_seq > cond->__data.__wakeup_seq) {
		cond->__data.__wakeup_seq = cond->__data.__total_seq;
		cond->__data.__woken_seq = cond->__data.__total_seq;
		cond->__data.__futex = (unsigned int) cond->__data.__total_seq * 2;
		++cond->__data.__broadcast_seq;
		lll_unlock (&cond->__data.__lock);

		/* wakoen all threads */
		lll_futex_wake ((int *)&cond->__data.__futex, INT_MAX);
		return 0;
	}

	lll_unlock (&cond->__data.__lock);

	return 0;
}
