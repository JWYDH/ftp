/*
 * ThreadPool.cpp
 *
 *  Created on: Nov 3, 2014
 *      Author: root
 */

#include "ThreadPool.h"

ThreadPool::ThreadPool()
{
	// TODO Auto-generated constructor stub
	pool=init();
}

ThreadPool::~ThreadPool()
{
	// TODO Auto-generated destructor stub
}

threadpool_type* ThreadPool::init()
{
	do
	{

		if((pool=(threadpool_type *)malloc(sizeof(threadpool_type))) == NULL)
		{
			cout<<"malloc pool fail"<<endl;
			break;
		}
//		memset(pool,0,sizeof(threadpool_type));
		pool->min_thread_num=MIN_THREAD_NUM;
		pool->max_thread_num=MAX_THREAD_NUM;
		pool->live_thread_num=MIN_THREAD_NUM;
		pool->busy_thread_num=0;
		pool->queue_size=0;
		pool->queue_max_size=TASK_SIZE;
		pool->wait_exit_thread_num=0;
		pool->task_queue=new queue<threadpool_task_t>;
		pool->shutdonwn=false;
		pool->threads=(pthread_t *)malloc(sizeof(pthread_t)*pool->queue_max_size);
		if(pool->threads==NULL)
		{
			cout<<"malloc threads fail"<<endl;
			break;
		}
		memset(pool->threads,0,sizeof(pthread_t)*pool->queue_max_size);

		if(pthread_mutex_init(&(pool->lock),NULL)!=0
		|| pthread_mutex_init(&(pool->thread_counter),NULL)!=0
		|| pthread_cond_init(&(pool->queue_not_empty),NULL)!=0
		|| pthread_cond_init(&(pool->queue_not_full),NULL)!=0)
		{
			cout<<"init the lock or cond fail"<<endl;
			break;
		}
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr,PTHREAD_SCOPE_PROCESS);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		for(int i=0;i<pool->min_thread_num;i++)
		{
            pthread_create(&(pool->threads[i]),&attr,threadpool_thread,(void*)pool);
            cout<<"start thread "<<pool->threads[i]<<endl;
		}
		pthread_create(&(pool->adjust_tid),NULL,adjust_thread,(void*)pool);

		return pool;
	}while(false);

	// threadpoolfree(pool);
	return NULL;
}

void* ThreadPool::threadpool_thread(void *threadpool)
{
	threadpool_type *pool=(threadpool_type*)threadpool;
	threadpool_task_t task;
	while(true)
	{
		pthread_mutex_lock(&(pool->lock));

		while(pool->queue_size==0 && (!pool->shutdonwn))
		{
			cout<<pthread_self()<<" thread is waiting"<<endl;
			pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));
			//队列任务为空，大部分线程空闲，判断线程调整，是否需要销毁
			if(pool->wait_exit_thread_num>0)
			{
				pool->wait_exit_thread_num--;
				if(pool->live_thread_num>pool->min_thread_num)
				{
					cout<<pthread_self()<<" thread is exiting"<<endl;
					pool->live_thread_num--;
					pthread_mutex_unlock(&(pool->lock));
					pthread_exit(NULL);
				}
			}
		}

		//广播唤醒销毁线程
		if(pool->shutdonwn)
		{
			pthread_mutex_unlock(&(pool->lock));
			cout<<pthread_self()<<"broadcast thread is exiting"<<endl;
			pthread_exit(NULL);
		}

		task.function=pool->task_queue->front().function;
		task.arg=pool->task_queue->front().arg;

		pool->task_queue->pop();
		pool->queue_size--;

		pthread_cond_broadcast(&(pool->queue_not_full));
		pthread_mutex_unlock(&(pool->lock));

		cout<<pthread_self()<<"start working"<<endl;
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thread_num++;
		pthread_mutex_unlock(&(pool->thread_counter));

		(*(task.function))(task.arg);

		cout<<pthread_self()<<"end working"<<endl;
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thread_num--;
		pthread_mutex_unlock(&(pool->thread_counter));
	}
	return NULL;
}
void* ThreadPool::adjust_thread(void *threadpool)
{
	threadpool_type *pool=(threadpool_type *)threadpool;

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr,PTHREAD_SCOPE_PROCESS);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	while(!pool->shutdonwn)
	{
		pthread_mutex_lock(&(pool->thread_counter));
		int busy_thread_num=pool->busy_thread_num;
		pthread_mutex_unlock(&(pool->thread_counter));

		pthread_mutex_lock(&(pool->lock));
		int queue_size=pool->queue_size;
		int live_thread_num=pool->live_thread_num;
		pthread_mutex_unlock(&(pool->lock));

		if(queue_size >= MIN_WAIT_TASK_NUM
		  && live_thread_num < pool->max_thread_num)
		{
			//add thread
			pthread_mutex_lock(&(pool->lock));
			int i=0;
			int add=0;
			while(i<pool->max_thread_num
			&& add<DEFAULT_THREAD_VARIABLE
			&& pool->live_thread_num<pool->max_thread_num)
			{
				if(pool->threads[i]==0)
				{
					pthread_create(&(pool->threads[i]),&attr,threadpool_thread,(void*)pool);
					add++;
					pool->live_thread_num++;
				}
				i++;
			}
			pthread_mutex_unlock(&(pool->lock));
		}

		//判断线程是否过多，改变等待退出的线程数
		if((busy_thread_num * 2)<live_thread_num
		&& live_thread_num >pool->min_thread_num)
		{
			pthread_mutex_lock(&(pool->lock));
			pool->wait_exit_thread_num=DEFAULT_THREAD_VARIABLE;
			pthread_mutex_unlock(&(pool->lock));

			for(int i=0;i<DEFAULT_THREAD_VARIABLE;i++)
			{
				pthread_cond_signal(&(pool->queue_not_empty));
			}
		}
	}

	return NULL;
}

bool ThreadPool::threadpool_add(threadpool_type *pool,void*(*function)(void *arg),void *arg)
{
	assert(pool!=NULL);
	assert(function!=NULL);
	assert(arg!=NULL);

	pthread_mutex_lock(&(pool->lock));

	//任务队列若满，条件挂起该线程，等待通知为不满唤醒线程
	while(pool->queue_size==pool->queue_max_size && !pool->shutdonwn)
	{
		pthread_cond_wait(&(pool->queue_not_full),&(pool->lock));
	}
	//广播唤醒
	if(pool->shutdonwn)
	{
		pthread_mutex_unlock(&(pool->lock));
		return false;
	}
	threadpool_task_t temp;
	temp.function=function;
	temp.arg=arg;
	pool->task_queue->push(temp);

	pool->queue_size++;
	cout<<pthread_self()<<" push queue ok"<<pool->queue_size<<endl;
	pthread_cond_signal(&(pool->queue_not_empty));
	pthread_mutex_unlock(&(pool->lock));
	return true;
}

bool ThreadPool::threadpool_destroy(threadpool_type *pool)
{
	if(pool==NULL)
	{
		return -1;
	}
	pool->shutdonwn=true;
	//阻塞等待线程调整线程结束
	pthread_join(pool->adjust_tid,NULL);
	//广播，唤醒阻塞在队列为空的线程
	pthread_cond_broadcast(&(pool->queue_not_empty));

	threadpoolfree(pool);

	return true;
}
void ThreadPool::threadpoolfree(threadpool_type *pool)
{
	if(pool==NULL)
		return;
	if(pool->threads)
	{
		free(pool->threads);
		delete pool->task_queue;
		pthread_mutex_destroy(&(pool->lock));
		pthread_mutex_destroy(&(pool->thread_counter));
		pthread_cond_destroy(&(pool->queue_not_empty));
		pthread_cond_destroy(&(pool->queue_not_full));
	}
	free(pool);
	return;
}
