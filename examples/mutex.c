#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

unsigned long counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define NUM 10000
#define TRIES 10000

static void *
func(void *arg)
{
	int i = 0;

	while(i++ < TRIES) {
		pthread_mutex_lock(&mutex);
		counter++;
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}


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

	printf("counter %ld\n", counter);
	assert(counter == NUM * TRIES);
	return 0;
}
