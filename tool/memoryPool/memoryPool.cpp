#include "memoryPool.h"
#include <thread>
#include <chrono>

MemoryPool::MemoryPool(const char* shmName) :
    shmName(shmName), shmaddr(NULL) {
    // 尝试以读写方式打开共享内存
    int shmfd = shm_open(this->shmName.c_str(), O_RDWR, 0644);
    if (shmfd < 0) {
        // 如果共享内存不存在，创建新的
        shmfd = shm_open(this->shmName.c_str(), O_RDWR | O_CREAT, 0644);
        if (shmfd < 0) {
            perror("shm_open is error\n");
            exit(EXIT_FAILURE);
        }
        // 设置共享内存大小
        if(ftruncate(shmfd, SHM_SIZE) < 0){
            perror("ftruncate is error\n");
            close(shmfd);
            exit(EXIT_FAILURE); 
        }
        std::cout << "创建新的共享内存" << std::endl;
    } else {
        std::cout << "连接到已存在的共享内存" << std::endl;
    }
    
    // 进行内存映射
    if((this->shmaddr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == (void*) -1){
        perror("mmap is error\n");
        close(shmfd);
        exit(EXIT_FAILURE); 
    }
    close(shmfd);

    // 尝试打开已存在的信号量
    sem = sem_open(SEM_NAME, O_RDWR);
    if (sem == SEM_FAILED) {
        // 如果信号量不存在，创建新的，初始值为0
        sem = sem_open(SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
        if (sem == SEM_FAILED) {
            perror("sem_open failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "创建新的信号量" << std::endl;
        std::cout << "-------------------------"<< std::endl;
    } else {
        std::cout << "连接到已存在的信号量" << std::endl;
        std::cout << "-------------------------"<< std::endl;
    }
    
    // 初始化内存块
    size_t offset = 0;
    for(int i = 0; i < MAX_BLOCKS; i++) {
        BlockHeader* header = reinterpret_cast<BlockHeader*>(static_cast<char*>(shmaddr) + offset);
        header->blockId = i;
        header->status = BLOCK_FREE;
        header->dataSize = 0;
        header->clientId = -1;
        header->timestamp = 0;
        header->serverType = SERVER_HEAD;

        // 设置块大小
        if (i == 0) {
            header->blockSize = LARGE_BLOCK_SIZE;  // 大块
        } else {
            header->blockSize = SMALL_BLOCK_SIZE;  // 小块
        }

        offset += header->blockSize;
    }
}

MemoryPool& MemoryPool::getInstance() {
    static MemoryPool instance(SHM_NAME);
    return instance;
}

MemoryPool::~MemoryPool()
{
    // 先释放共享内存资源
    freeSharedMemory();
    //cleanup();
}

void MemoryPool::freeSharedMemory()
{
    // 1.取消内存映射
    if(munmap(this->shmaddr,SHM_SIZE) < 0){
        perror("munmap is error\n");
        shm_unlink(this->shmName.c_str());
        exit(EXIT_FAILURE);
    }
    // 2.取消共享内存的连接
    shm_unlink(this->shmName.c_str());
}

void MemoryPool::writeData(const char* buf, int clientId, ServerType serverType) {
    std::unique_lock<std::mutex> lock(shm_mutex); 

    if(serverType == SERVER_BACK){
        std::cout << "开始写入数据，服务器类型: 后置" << std::endl;
    }else{
        std::cout << "开始写入数据，服务器类型: 前置" << std::endl;
    }

    // 查找合适的空闲块
    BlockHeader* freeBlock = nullptr;
    size_t dataSize = sizeof(HEAD) + reinterpret_cast<const HEAD*>(buf)->businessLength;

    while (true) {
        size_t offset = 0;
        for(int i = 0; i < MAX_BLOCKS; i++) {
            BlockHeader* header = reinterpret_cast<BlockHeader*>(static_cast<char*>(shmaddr) + offset);
            if(header->status == BLOCK_FREE && header->blockSize >= dataSize) {
                freeBlock = header;
                break;
            }
            offset += header->blockSize;  // 移动到下一个块
        }

        if (freeBlock) {
            break;  // 找到合适的空闲块，退出循环
        }

        // 如果没有合适的空闲块，等待条件变量
        std::cout << "没有合适的空闲块，等待中..." << std::endl;
        cv.wait(lock);  // 释放锁并等待，直到被唤醒
    }

    // 写入数据
    char* blockData = reinterpret_cast<char*>(freeBlock) + sizeof(BlockHeader);
    memcpy(blockData, buf, dataSize);

    // 更新块头信息
    freeBlock->status = BLOCK_READY;
    freeBlock->dataSize = dataSize;
    freeBlock->clientId = clientId;
    freeBlock->timestamp = time(nullptr);
    freeBlock->serverType = (serverType == SERVER_HEAD) ? SERVER_BACK : SERVER_HEAD;  // 确保 serverType 正确设置

    // 添加调试日志
    // std::cout << "写入数据到块 " << freeBlock->blockId << std::endl;
    // std::cout << "块大小: " << freeBlock->blockSize << " 字节" << std::endl;
    // std::cout << "数据大小: " << dataSize << " 字节" << std::endl;
    // std::cout << "客户端ID: " << clientId << std::endl;
    // std::cout << "时间戳: " << freeBlock->timestamp << std::endl;
    // std::cout << "接收块的服务器类型: " << freeBlock->serverType << std::endl;

    // 信号量操作
    if(sem_post(sem) == -1){
        perror("sem_post failed");
        exit(EXIT_FAILURE);
    }

    // 增加短暂延迟，确保数据同步
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 打印信号量值
    int val;
    sem_getvalue(sem, &val);
    std::cout << "写入后信号量值: " << val << std::endl;
    std::cout << "-------------------------"<< std::endl;
}

bool MemoryPool::readData(char* buf, int& clientId, ServerType serverType, int timeoutMs) {
    auto startTime = std::chrono::steady_clock::now();  // 记录开始时间

    while (true) {
        // 检查是否超时
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        if (elapsedTime >= timeoutMs) {
            std::cout << "读取数据超时，服务器类型: " << serverType << std::endl;
            return false;  // 超时返回 false
        }

        std::cout << "开始读取数据，服务器类型: " << serverType << std::endl;
        sem_wait(sem);  // 等待信号量

        std::unique_lock<std::mutex> lock(shm_mutex);  // 使用 unique_lock 以便与条件变量配合

        // 查找可读块
        BlockHeader* readyBlock = nullptr;
        size_t offset = 0;
        for(int i = 0; i < MAX_BLOCKS; i++) {
            BlockHeader* header = reinterpret_cast<BlockHeader*>(static_cast<char*>(shmaddr) + offset);
            if(header->status == BLOCK_READY && header->serverType == serverType && header->clientId != -1) {  // 确保接收方类型匹配且 clientId 不为 -1
                readyBlock = header;
                break;
            }
            offset += header->blockSize;  // 移动到下一个块
        }

        if(!readyBlock) {
            // 如果没有找到匹配的块，释放信号量并继续等待
            sem_post(sem);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 等待100ms再重试
            continue;  // 继续下一次循环
        }

        // 添加调试日志
        // std::cout << "从块 " << readyBlock->blockId << " 读取数据" << std::endl;
        // std::cout << "数据大小: " << readyBlock->dataSize << " 字节" << std::endl;
        // std::cout << "客户端ID: " << readyBlock->clientId << std::endl;
        // std::cout << "时间戳: " << readyBlock->timestamp << std::endl;
        // std::cout << "接收块的服务器类型: " << readyBlock->serverType << std::endl;

        // 读取数据
        char* blockData = reinterpret_cast<char*>(readyBlock) + sizeof(BlockHeader);
        memcpy(buf, blockData, readyBlock->dataSize);

        // 返回客户端ID
        clientId = readyBlock->clientId;

        // 标记块为已处理
        readyBlock->status = BLOCK_FREE;
        readyBlock->dataSize = 0;
        readyBlock->clientId = -1;
        readyBlock->timestamp = 0;
        readyBlock->serverType = SERVER_HEAD;  // 重置为默认值

        // 清空内存区域
        memset(blockData, 0, readyBlock->blockSize - sizeof(BlockHeader));

        // 通知等待的线程有空闲块可用
        cv.notify_one();

        // 打印信号量值
        int val;
        sem_getvalue(sem, &val);
        std::cout << "读取后信号量值: " << val << std::endl;
        std::cout << "-------------------------"<< std::endl;
        return true;  // 成功读取数据后返回 true
    }
}

void MemoryPool::cleanup() {
    const char* shmName = SHM_NAME;
    // 删除共享内存
    if(shm_unlink(shmName) == -1 && errno != ENOENT) {
        perror("shm_unlink cleanup failed");
    }
    // 清理信号量
    if(sem_unlink(SEM_NAME) == -1 && errno != ENOENT) {
        perror("sem_unlink cleanup failed");
    }
    // 清理共享内存
    if(shm_unlink(SHM_NAME) == -1 && errno != ENOENT) {
        perror("shm_unlink cleanup failed");
    }
    std::cout << "清理共享内存和信号量完成" << std::endl;
}

void MemoryPool::checkTimeouts() {
    std::lock_guard<std::mutex> lock(shm_mutex);
    time_t now = time(nullptr);
    
    size_t offset = 0;
    for(int i = 0; i < MAX_BLOCKS; i++) {
        BlockHeader* header = reinterpret_cast<BlockHeader*>(static_cast<char*>(shmaddr) + offset);
        if(header->status == BLOCK_READY && (now - header->timestamp) > TIMEOUT_SECONDS) {
            // 处理超时
            header->status = BLOCK_FREE;
            header->dataSize = 0;
            header->clientId = -1;
            header->timestamp = 0;
            header->serverType = SERVER_HEAD;  // 重置为默认值
            std::cerr << "Timeout detected for block " << header->blockId << std::endl;
        }
        offset += header->blockSize;  // 移动到下一个块
    }
}