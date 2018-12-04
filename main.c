#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "task.h"
#include "sched.h"

static void *
func(void *arg)
{
	//long idx = (long)arg;
	int counter = 0;

	while(counter++ < 10000000) {
		//printf("func idx = %d\n", idx);
		co_yield();
	}

	return NULL;
}

void sig(int sig)
{
	exit(1);
}

int
main(int argc, char *argv[])
{
	signal(SIGINT, sig);
      	sched_init(); 

	co_create(func, (void *)1);
	co_create(func, (void *)2);
	co_create(func, (void *)3);
	co_create(func, (void *)4);
	co_create(func, (void *)5);

	scheduler();

	return 0;
}
