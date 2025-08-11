#include "threadPool.h"

ThreadPool::ThreadPool(size_t minNum)
{
    this->minNum = minNum;
    this->maxNum = THREAD_NUM_MAX;

    //初始化互斥锁
    pthread_mutex_init(&this->mutex, NULL);
    //初始化条件变量
    pthread_cond_init(&this->cond, NULL);
    //创建线程池的最小线程数 并将线程都添加到空闲链表中
    for (size_t i = 0; i < minNum; ++i) {
        pthread_t pthread_id;
        pthread_create(&pthread_id, NULL, thread_function, this);
        this->freeList.push_back(pthread_id);
    }
}

ThreadPool::~ThreadPool()
{
}

//添加任务 在主线程中执行 不需要上锁
void ThreadPool::pushTask(BaseTask* task)
{
    lock();
    this->taskQueue.push(task);
    size_t currentThreadCount = freeList.size() + busyList.size();

    // 扩容逻辑
    if (taskQueue.size() > currentThreadCount && currentThreadCount < maxNum) {
        size_t threadsToCreate = std::min(taskQueue.size() - currentThreadCount, maxNum - currentThreadCount);
        for (size_t i = 0; i < threadsToCreate; ++i) {
            pthread_t pthread_id;
            pthread_create(&pthread_id, NULL, thread_function, this);
            this->freeList.push_back(pthread_id);
        }
    }

    // 确保线程池不会缩小到小于 minNum
    while (freeList.size() + busyList.size() > minNum && taskQueue.empty()) {
        freeList.pop_back();
    }

    unlock();
    wakeup();
}

//取出任务 需要上锁 防止资源争抢
BaseTask* ThreadPool::popTask()
{
    BaseTask* backTask = this->taskQueue.front();
    this->taskQueue.pop();
    return backTask;
}

void ThreadPool::moveToBusyList(pthread_t threadID)
{
    std::list<pthread_t>::iterator it;
    it = std::find(this->freeList.begin(), this->freeList.end(), threadID);
    if (it != this->freeList.end()) {//找到了该线程
        this->freeList.erase(it);       //将该线程从空闲链表中删除
        this->busyList.push_back(*it);  //将该线程添加到忙碌链表中
    }
}

void ThreadPool::moveToFreeList(pthread_t threadID)
{
    std::list<pthread_t>::iterator it;
    it = std::find(this->busyList.begin(), this->busyList.end(), threadID);
    if (it != this->busyList.end()) {//找到了该线程
        this->busyList.erase(it);       //将该线程从忙碌链表中删除
        this->freeList.push_back(*it);  //将该线程添加到空闲链表中
    }
}

void ThreadPool::lock()
{
    pthread_mutex_lock(&this->mutex);
}

void ThreadPool::unlock()
{
    pthread_mutex_unlock(&this->mutex);
}

void ThreadPool::wait()
{
    pthread_cond_wait(&this->cond, &this->mutex);
}

void ThreadPool::wakeup()
{
    pthread_cond_signal(&this->cond);
}

bool ThreadPool::queueIsEmpty()
{
    return this->taskQueue.empty();
}

void* ThreadPool::thread_function(void* arg)
{
    ThreadPool* threadPool = (ThreadPool*)arg;
    //获取线程的id
    pthread_t threadID = pthread_self();
    //将主线程与其它线程进行分离
    pthread_detach(threadID);

    while (1) {
        //给线程上锁 防止线程之间的数据干扰
        threadPool->lock();
        //如果任务队列为空则将线程阻塞住
        if (threadPool->queueIsEmpty()) {
            threadPool->wait();
        }
        // std::cout << "任务的数量：" << threadPool->taskQueue.size() << std::endl;
        // std::cout << "空闲链表的数量：" << threadPool->freeList.size() << std::endl;
        // std::cout << "忙碌链表的数量：" << threadPool->busyList.size() << std::endl;
        //如果任务队列不为空 将该线程放入忙碌链表 取出任务执行相应的业务
        threadPool->moveToBusyList(threadID);
        BaseTask* baseTask = threadPool->popTask();
        //解锁
        threadPool->unlock();

        //不能锁在里面 不然就是按照顺序去执行业务逻辑了
        baseTask->working();

        //执行完业务逻辑后 将该线程放入空闲链表中
        threadPool->lock();
        threadPool->moveToFreeList(threadID);
        threadPool->unlock();
    }

    return nullptr;
}
