#ifndef __MEMORYPOOL_H
#define __MEMORYPOOL_H

#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>       
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <mutex> 
#include <stdexcept>
#include <thread>  
#include <chrono>  
#include "../agreement/agreement.h"
#include <semaphore.h>
#include <condition_variable> 

#define MQ_SIZE     1024   // 消息的最大大小
#define SEM_NAME    "/stopCar_sem"  // 信号量独立命名
#define SHM_NAME    "/stopCar_shm"  // 共享内存名称

#define TIMEOUT_SECONDS 30  // 超时时间，单位为秒

#define SHM_SIZE    8100   // 共享内存的大小

#define LARGE_BLOCK_SIZE 4060   // 大块大小
#define SMALL_BLOCK_SIZE 1000   // 小块大小

#define MAX_LARGE_BLOCKS 1  // 大块数量
#define MAX_SMALL_BLOCKS ((SHM_SIZE - (MAX_LARGE_BLOCKS * LARGE_BLOCK_SIZE)) / SMALL_BLOCK_SIZE)  // 小块数量
#define MAX_BLOCKS (MAX_LARGE_BLOCKS + MAX_SMALL_BLOCKS)  // 总块数

// 定义内存块状态
enum BlockStatus {
    BLOCK_FREE = 0,
    BLOCK_IN_USE,
    BLOCK_READY
};

//定义服务器的类型
enum ServerType{
    SERVER_HEAD = 0,
    SERVER_BACK
};

// 定义内存块头
struct BlockHeader {
    int blockId;            // 块ID
    BlockStatus status;     // 块状态
    size_t blockSize;      // 块大小
    size_t dataSize;        // 数据大小
    int clientId;           // 客户端ID
    time_t timestamp;       // 时间戳
    ServerType serverType;  // 接收信息服务器的类型
};

class MemoryPool
{
public:
    static MemoryPool& getInstance();

    // 删除拷贝构造函数和赋值运算符
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    ~MemoryPool();
    void freeSharedMemory();                                                // 释放共享内存的资源
    void writeData(const char* buf, int clientId, ServerType serverType);   // 将数据写入共享内存池
    bool readData(char* buf, int& clientId, ServerType serverType, int timeoutMs = 5000);         // 将共享内存池的数据读出来
    void checkTimeouts();
    static void cleanup();
protected:
private:
    explicit MemoryPool(const char* shmName);
    std::string shmName;    // 共享内存的名字
    void *shmaddr;          // 内存映射的起始地址
    std::mutex shm_mutex;  // 添加互斥锁
    sem_t* sem; // 添加信号量
    std::condition_variable cv;  // 条件变量，用于通知空闲块可用
};

#endif