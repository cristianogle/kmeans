#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef struct tpool_work {
	/*pointer to the function executed by the worker that selects the request*/
	void (*func)();
	/* pointer to the argument (if exists) */
	void *arg;
	/* pointer to the next request on the queue. */
	struct tpool_work *next;
} tpool_work_t;

typedef struct tpool {
	int num_threads;
	int max_queue_size;

	pthread_t *threads;
	int cur_queue_size;
	tpool_work_t *queue_head;
	tpool_work_t *queue_tail;

	pthread_mutex_t queue_lock;
	pthread_cond_t  queue_not_empty;
	pthread_cond_t  queue_not_full;
	pthread_cond_t  queue_empty;

	int queue_closed;
	int shutdown;
} *tpool_t;

void tpool_init(tpool_t *tpoolp,
           int num_worker_threads,
           int max_queue_size);

void * tpool_thread(void * tpool);

int tpool_add_work(tpool_t tpool,
           void *func,
           void *arg);

int tpool_destroy(tpool_t tpoolp, int finish);
