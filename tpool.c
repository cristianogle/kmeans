#include "tpool.h"

void tpool_init(tpool_t *tpoolp, int num_worker_threads, int max_queue_size) {
   int i;
   tpool_t tpool;
   /* allocate a pool data structure */
   tpool = (tpool_t )malloc(sizeof(struct tpool));
   /* initialize the fields */
   tpool->num_threads = num_worker_threads;
   tpool->max_queue_size = max_queue_size;
   tpool->threads = (pthread_t *)malloc(sizeof(pthread_t)*num_worker_threads);
   tpool->cur_queue_size = 0;
   tpool->queue_head = NULL;
   tpool->queue_tail = NULL;
   tpool->queue_closed = 0;
   tpool->shutdown = 0;

   pthread_mutex_init(&(tpool->queue_lock), NULL);
   pthread_cond_init(&(tpool->queue_not_empty), NULL);
   pthread_cond_init(&(tpool->queue_not_full), NULL);
   pthread_cond_init(&(tpool->queue_empty), NULL);

   /* creates all worker threads, starting each one in the tpool_thread() function */
   for (i = 0; i != num_worker_threads; i++) {
	   pthread_create( &(tpool->threads[i]), NULL,
					   tpool_thread, (void *)tpool);
   }
   *tpoolp = tpool;
}

/* contains the logic each worker uses to check the queue for work and take appropriate action.
 * parameter: pointer to the tpool_t struct for the pool to which the thread belongs */

void * tpool_thread(void * pool) {
	tpool_t tpool = (tpool_t)pool;
	tpool_work_t *my_workp;
	for (;;) {
		pthread_mutex_lock(&(tpool->queue_lock));
        while ( (tpool->cur_queue_size == 0) && (!tpool->shutdown)) {
        	pthread_cond_wait(&(tpool->queue_not_empty), &(tpool->queue_lock));
		}
		if (tpool->shutdown) {
			pthread_mutex_unlock(&(tpool->queue_lock));
			pthread_exit(NULL);
        }
		// remove the request from the head
		my_workp = tpool->queue_head;
		tpool->cur_queue_size --;
		if (tpool->cur_queue_size == 0)
			tpool->queue_head = tpool->queue_tail =NULL;
		else
			tpool->queue_head = my_workp->next;

		if (tpool->cur_queue_size == (tpool->max_queue_size -1))
			pthread_cond_broadcast(&(tpool->queue_not_full));
		if (tpool->cur_queue_size == 0)
			pthread_cond_signal(&(tpool->queue_empty));
		pthread_mutex_unlock(&(tpool->queue_lock));

		(*(my_workp->func))(my_workp ->arg);

		free(my_workp);
   }
}

int tpool_add_work(tpool_t tpool, void *func, void *arg) {
	tpool_work_t *workp;
	pthread_mutex_lock(&tpool->queue_lock);

	while ((tpool->cur_queue_size == tpool->max_queue_size) && (!(tpool->shutdown || tpool->queue_closed))) {
		pthread_cond_wait(&tpool->queue_not_full, &tpool->queue_lock);
    }
	if (tpool->shutdown || tpool->queue_closed) {
		pthread_mutex_unlock(&tpool->queue_lock);
		return -1;
	}
	/* allocate work structure */
	workp = (tpool_work_t *)malloc(sizeof(tpool_work_t));
	workp->func = func;
	workp->arg = arg;
	workp->next = NULL;
	if (tpool->cur_queue_size == 0) {
		tpool->queue_tail = tpool->queue_head = workp;
		pthread_cond_broadcast(&tpool->queue_not_empty);
	} else {
		(tpool->queue_tail) ->next = workp;
		tpool->queue_tail = workp;
	}
	tpool->cur_queue_size++;
	pthread_mutex_unlock(&tpool->queue_lock);
	return 1;
}

int tpool_destroy(tpool_t tpool, int finish) {
	int i;
	tpool_work_t *cur_nodep;
	pthread_mutex_lock(&(tpool->queue_lock));
	/* Is a shutdown already in progress? */
	if (tpool->queue_closed || tpool->shutdown) {
		pthread_mutex_unlock(&(tpool->queue_lock));
		return 0;
	}
	tpool->queue_closed = 1;
	/* If the finish flag is set, wait for workers to drain queue*/
	if (finish == 1) {
		while (tpool->cur_queue_size != 0) {
			pthread_cond_wait(&(tpool->queue_empty), &(tpool->queue_lock));
		}
	}
	// indicate to workers that the pool is being deactivated(add_work returns -1)
	tpool->shutdown = 1;
	pthread_mutex_unlock(&(tpool->queue_lock));
	/* Wake up any workers so they recheck shutdown flag */
	pthread_cond_broadcast(&(tpool->queue_not_empty));
	pthread_cond_broadcast(&(tpool->queue_not_full));
	/* Wait for workers to exit */
	for(i=0; i < tpool->num_threads; i++) {
        pthread_join(tpool->threads[i],NULL);
	}
	/* Now free pool structures */
	free(tpool->threads);
	while(tpool->queue_head != NULL) {
		cur_nodep = tpool->queue_head ->next;
		tpool->queue_head = tpool->queue_head ->next;
		free(cur_nodep);
	}
	free(tpool);
	printf("Thread pool destroyed");
	return 0;
}
