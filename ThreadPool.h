/*
 * ThreadPool.h
 *
 *  Created on: Nov 3, 2014
 *      Author: root
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<assert.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<iostream>
#include<queue>

using namespace std;

#define TASK_SIZE 512
#define MIN_THREAD_NUM 8
#define MAX_THREAD_NUM 64
#define MIN_WAIT_TASK_NUM 8
#define DEFAULT_THREAD_VARIABLE 8

typedef struct
{
	void *(*function)(void *);
	void *arg;
} threadpool_task_t;
struct threadpool_type
{
	pthread_mutex_t lock;
	pthread_mutex_t thread_counter;
	pthread_cond_t queue_not_full;
	pthread_cond_t queue_not_empty;

	pthread_t *threads;
	pthread_t adjust_tid;
	queue<threadpool_task_t> *task_queue;


	int min_thread_num;
	int max_thread_num;
	int live_thread_num;
	int busy_thread_num;
	int wait_exit_thread_num;
//	int queue_front;
//	int queue_rear;
	int queue_size;
	int queue_max_size;
	bool shutdonwn;
};
class ThreadPool
{
public:
	ThreadPool();
	virtual ~ThreadPool();

	threadpool_type* init();
	static void* threadpool_thread(void *threadpool);
	static void* adjust_thread(void *threadpool);
	bool threadpool_add(threadpool_type *pool,void*(*function)(void *arg),void *arg);
	bool threadpool_destroy(threadpool_type *pool);
	void threadpoolfree(threadpool_type *pool);

	threadpool_type *pool;
private:


};

#endif /* THREADPOOL_H_ */
