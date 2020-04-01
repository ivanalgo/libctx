#ifndef _LOCK_H
#define _LOCK_H

#include <assert.h>
#include "list.h"

typedef int spin_t;

static inline void spin_init(spin_t *lock)
{
	*lock = 1;
}

static inline void spin_lock(spin_t *lock)
{
	while (1) {
		while(*lock < 0) {}

		if (__sync_bool_compare_and_swap(lock, 1, -1))
			break;
	}
}

static inline void spin_unlock(spin_t *lock)
{
	__sync_lock_test_and_set(lock, 1);
}

typedef struct mutex {
	spin_t spin_lock;
	int value;
	struct list waiter;
} mutex_t;

extern void co_mutex_init(mutex_t *mutex);
extern void co_mutex_lock(mutex_t *mutex);
extern void co_mutex_unlock(mutex_t *mutex);
	
typedef struct notify {
	mutex_t lock;
	struct list waiter;
	int value;
} notify_t;

extern void co_notify_init(notify_t *notify);
extern void co_notify_wait(notify_t *notify);
extern void co_notify_complete(notify_t *notify);

#endif /* _LOCK_H */
