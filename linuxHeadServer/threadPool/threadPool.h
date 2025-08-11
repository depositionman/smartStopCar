#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <iostream>
#include <list>
#include <queue>
#include "baseTask.h"
#include <mutex>
#include <pthread.h>
#include <algorithm>

#define THREAD_NUM_MAX 10

class ThreadPool
{
public:
    ThreadPool(){}
    ThreadPool(size_t minNum);
    ~ThreadPool();
    //将线程移至空闲链表
    void moveToFreeList(pthread_t threadid);
    //将线程移至忙碌链表
    void moveToBusyList(pthread_t threadid);
    //放入任务
    void pushTask(BaseTask* task);
    //取出任务
    BaseTask* popTask();
    //上锁
    void lock();
    //解锁
    void unlock();
    //控制线程的条件变量
	void wait();
	void wakeup();
    //判断任务队列是否为空
	bool queueIsEmpty();
	//线程的执行函数
	static void* thread_function(void* arg);
protected:

private:
    pthread_mutex_t mutex;               // 互斥锁
    pthread_cond_t cond;		         // 条件变量
    size_t maxNum;                      // 最大线程量
    size_t minNum;                      // 最小线程量
    std::list<pthread_t> freeList;       // 空闲链表
    std::list<pthread_t> busyList;       // 忙碌链表
    std::queue<BaseTask*> taskQueue;     // 任务队列
};

#endif