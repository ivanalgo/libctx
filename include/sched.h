#ifndef _SCHED_H_
#define _SCHED_H_

#include "task.h"

extern void scheduler(void);
extern void create_new_task(task_t *task);
extern void sched_init(void);
extern void task_exit(task_t *task);

extern task_t *current;

#endif /* _SCHED_H_ */
