#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static void *
func(void *arg)
{
	long idx = (long)arg;
	int counter = 0;

	while(counter++ < 10) {
		pthread_yield();
	}

	printf (" thread id %ld exit...\n", idx);
	return NULL;
}

#define NUM 10000

int
main(int argc, char *argv[])
{
	int i;
	pthread_t th[NUM];

	for (i = 0; i < NUM; i++) {
		pthread_create(th + i, NULL, func, (void *)(long)i);
	}

	for (i = 0; i < NUM; i++) {
		pthread_join(th[i], NULL);
	}

	printf("main return\n");
	return 0;
}
