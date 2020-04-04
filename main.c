#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *
func(void *arg)
{
	long idx = (long)arg;
	int counter = 0;

	while(counter++ < 10) {
		pthread_mutex_lock(&lock);
		pthread_yield();
		pthread_mutex_unlock(&lock);
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

	pthread_mutex_lock(&lock);

	for (i = 0; i < NUM; i++) {
		pthread_create(th + i, NULL, func, (void *)(long)i);
	}

	printf("main try to unlock\n");
	pthread_mutex_unlock(&lock);

	for (i = 0; i < NUM; i++) {
		pthread_join(th[i], NULL);
	}

	return 0;
}
