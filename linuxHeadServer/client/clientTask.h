#ifndef __CLIENTTASK_H
#define __CLIENTTASK_H

#include <iostream>
#include "threadPool/baseTask.h"
#include "../../tool/agreement/agreement.h"
#include <string.h>
#include "../agreement/crc.h"
#include <unistd.h>
#include "../tool/memoryPool/memoryPool.h"

class ClientTask :public BaseTask 
{
public:
    ClientTask(int fd, char* buf);
    virtual ~ClientTask();
    void working() override;
    void captchaBusiness();                     // 验证码业务
    void registerBusiness();                    // 注册业务
    void loginBusiness();                       // 登录业务
    void enterStopCarBusiness();                // 入库业务
    void saveCarPlateBusiness();                // 保存入场车辆的车牌
    void endSaveCarPlateBusiness();             // 收到文件结束包的业务
    void exitStopCarBusiness();                 // 出库业务
    void modifyPlateBusiness();                 // 修改车牌业务
    void videoUploadBusiness();                 // 视频上传业务
    void videoQueryBusiness();                  // 视频查询业务
    void videoNextPageBusiness();               // 视频下一页业务
    void videoPlayBusiness();                   // 视频播放业务
    void videoPlayOverBusiness();               // 视频播放结束业务
    void parkBusiness();                        // 停车场业务
    void carNumberQueryBusiness(int type);      // 车辆查询业务
    void heartbeatBusiness();                   // 心跳业务
protected:
private:
    char *buf;                                              // 缓存区
    int fd;                                                 // 对应客户端的fd
    HEAD head;                                              // 包头
    MemoryPool& memoryPool;                                 // 改为引用
};

#endif