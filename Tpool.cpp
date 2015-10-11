#include"EZweb.h"
#include"Tpool.h"
void* thread_ctrl(void* arg)
{
	Tpool* pool = reinterpret_cast<Tpool*> (arg);
	while(1)
	{
		pthread_mutex_lock(&(pool->jobQueue_lock));
#ifdef DEBUG_THREAD
		EZ_INFO("starting thread: ");
		EZ_INFO(pthread_self());
		EZ_INFO('\n');
#endif
		
		while(pool->jobQueue.empty() && pool->shutdown == false)
		{
#ifdef DEBUG_THREAD
			EZ_INFO("thread: ");
			EZ_INFO(pthread_self());
			EZ_INFO(" is waiting\n");
#endif
			pthread_cond_wait(&(pool->jobQueue_cond), &(pool->jobQueue_lock));
		}

		if(pool->shutdown)
		{
			pthread_mutex_unlock(&(pool->jobQueue_lock));
			pthread_exit(NULL);
		}

#ifdef DEBUG_THREAD
		EZ_INFO("thread: ");
		EZ_INFO(pthread_self());
		EZ_INFO("is starting working\n");
#endif

		J_* todo = pool->jobQueue.front();
		pool->jobQueue.pop();
		pthread_mutex_unlock(&(pool->jobQueue_lock));

		(*(todo->process))(todo->arg);
		delete todo;
		todo = NULL;
	}
}
