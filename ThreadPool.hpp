#ifndef __THREAD_POOL_HPP__
#define __THREAD_POLL_HPP__

#include<iostream>
#include<queue>
#include<pthread.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include"Log.hpp"

//typedef event_base (*handler_t)(event_base);//函数指针
class Task
{
		private:
				struct bufferevent* bev;
				//handler_t handler;
		public:
				Task()
				{
						bev = NULL;
						//handler = NULL;
				};
				void SetTask(struct bufferevent* bev_)
				{
						bev = bev_;
						//handler = handler_;
				}
				void Run()
				{
						//handler(base);
						 Entry:: HandlerRequest(bev);
				}
				~Task()
				{}
};
#define NUM 5
class ThreadPool
{
		private: 
				int thread_total_num;
				int thread_idle_num;
				std::queue<Task> task_queue;
				pthread_mutex_t lock;
				pthread_cond_t cond;
				volatile bool is_quit;
		public:
				void LockQueue()
				{
						pthread_mutex_lock(&lock);
				}
				void UnlockQueue()
				{
						pthread_mutex_unlock(&lock);
				}
				bool IsEmpty()
				{
						return task_queue.size() == 0;
				}
				void ThreadIdle()
				{
						if(is_quit)
						{
								UnlockQueue();
								thread_total_num--;
								LOG(INFO,"thread quit .....");
								pthread_exit((void*)0);
						}
						thread_idle_num++;
						pthread_cond_wait(&cond, &lock);//会自动释放锁，被唤醒时自动获得锁
						thread_idle_num--;			
				}
				void WakeupOneThread()
				{
						pthread_cond_signal(&cond);
				}
				void WakeupAllThread()
				{
						pthread_cond_broadcast(&cond);
				}
				static void *thread_routine(void *arg)//参数必须是一个，但是函数时成员函数有this指针，所以必须static
				{
						ThreadPool *tp = (ThreadPool*)arg;
						pthread_detach(pthread_self());

						for(;;)
						{
								tp->LockQueue();
								while(tp->IsEmpty())
								{
										tp->ThreadIdle();
								}

								Task t;
								tp->PopTask(t);
								tp->UnlockQueue();
								LOG(INFO,"task has been taked,handler...");
								std::cout<< "thread id is:"<<pthread_self()<<std::endl;
								t.Run();
						}
				}
		public:
				ThreadPool(int num_ = NUM):thread_total_num(num_),thread_idle_num(0),is_quit(false)
		{
				pthread_mutex_init(&lock, NULL);
				pthread_cond_init(&cond, NULL);
		}
				void initThreadPool()
				{
						int i_ = 0;
						for(;i_ < thread_total_num;i_++)
						{
								pthread_t tid;
								pthread_create(&tid, NULL, thread_routine, this);
						}
				}
				void PopTask(Task &t_)
				{
						t_ = task_queue.front();
						task_queue.pop();
				}
				void PushTask(Task &t_)
				{
						LockQueue();
						if(is_quit)
						{
								UnlockQueue();
								return ;
						}
						task_queue.push(t_);

						WakeupOneThread();

						UnlockQueue();
				}
				void Stop()
				{
						LockQueue();
						is_quit = true;
						UnlockQueue();

						while(thread_idle_num > 0 )
						{
								WakeupAllThread();
						}
				}
				~ThreadPool()
				{
						pthread_mutex_destroy(&lock);
						pthread_cond_destroy(&cond);
				}
};


#endif