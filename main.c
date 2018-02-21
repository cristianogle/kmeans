#include "tpool.h"

void task1(){
	printf("Thread #%u working on task1\n", (int)pthread_self());
	sleep(1);
}

void task2(){
	printf("Thread #%u working on task2\n", (int)pthread_self());
	sleep(2);
}

int main() {
	tpool_t thread_pool;
	printf("Making threadpool with 4 threads\n");
	tpool_init(&thread_pool, 4, 40);
	printf("Adding 40 tasks to threadpool\n");
	int i;
	for (i=0; i<20; i++){
		tpool_add_work(thread_pool, (void*)task1, NULL);
		tpool_add_work(thread_pool, (void*)task2, NULL);
	};
	/* Destroy the threadpool after emptying the queue (1) */
	tpool_destroy(thread_pool, 1);
}
