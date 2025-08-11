#include "clientTask.h"
#include <iostream>
#include "../agreement/agreement.h"  

ClientTask::ClientTask(int fd, char* buf) : 
    buf(buf),fd(fd),
    memoryPool(MemoryPool::getInstance())  // 在初始化列表中获取单例
{
    memset(&head,0,sizeof(HEAD));
}

ClientTask::~ClientTask() 
{
    // 不需要手动释放内存池
}

void ClientTask::working() 
{
    // 执行任务的逻辑
    std::cout << "--------处理客户端" << fd << "的请求-------"<< std::endl;
    // 获取包头判断包的类型
    memcpy(&head,buf,sizeof(HEAD));
    
    // 根据获取到的包头类型去处理对应的业务
    if(head.businessType == 1){// 获取验证码
        captchaBusiness();
    }else if(head.businessType == 3){// 注册请求
        registerBusiness();
    }else if(head.businessType == 5){// 登录请求
        loginBusiness();
    }else if(head.businessType == 9){// 入库请求
        enterStopCarBusiness();
    }else if(head.businessType == 7){// 文件传输请求
        saveCarPlateBusiness();
    }else if(head.businessType == 98){// 文件结束包
        endSaveCarPlateBusiness();
    }else if(head.businessType == 11){// 出库请求
        exitStopCarBusiness();
    }else if(head.businessType == 15){// 修改车牌请求
        modifyPlateBusiness();
    }else if(head.businessType == 13){// 视频上传请求
        videoUploadBusiness();
    }else if(head.businessType == 17){// 视频查询请求
        videoQueryBusiness();
    }else if(head.businessType == 19){// 视频下一页请求
        videoNextPageBusiness();
    }else if(head.businessType == 21){// 视频播放请求
        videoPlayBusiness();
    }else if(head.businessType == 23){// 视频播放结束请求
        videoPlayOverBusiness();
    }else if(head.businessType == 25){// 停车场请求
        parkBusiness();
    }else if(head.businessType == 27 || head.businessType == 28 || head.businessType == 29){// 车辆查询请求
        carNumberQueryBusiness(head.businessType);
    }else if(head.businessType == 31){
        //heartbeatBusiness();
    }

    std::cout << "处理客户端发来的业务:" << std::endl;
    std::cout << "head.businessType" << head.businessType << std::endl;
    std::cout << "发送返回包成功（前置->客户端）" << std::endl;
    std::cout << "-------------------------"<< std::endl;

    // 任务完成后释放内存
    delete this;
} 

void ClientTask::captchaBusiness()
{
    std::cout << "---------处理验证码业务-----------" << std::endl;
    // 初始化需要使用的变量
    GETCAPTCHA getcaptcha;// 获取验证码请求包
    memcpy(&getcaptcha,this->buf+sizeof(HEAD),sizeof(GETCAPTCHA));
    BACKCAPTCHA backcaptcha;// 返回验证码包
    memset(&backcaptcha,0,sizeof(BACKCAPTCHA));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[40] = {0};
    memcpy(crcbuf,&getcaptcha,sizeof(GETCAPTCHA));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf,sizeof(GETCAPTCHA));

    if(crc == head.crc){// 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    }else{// 包体数据错误，进行处理
        backcaptcha.flag = 2;   // 失败
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[20] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&backcaptcha,buf+sizeof(HEAD),sizeof(backcaptcha));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[20] = {0};
    HEAD head;
    memset(&head,0,sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf,&backcaptcha,sizeof(BACKCAPTCHA));
    //配置包体
    head.businessType = 2;
    head.businessLength = sizeof(BACKCAPTCHA);
    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKCAPTCHA));
    //拼接返回包
    memcpy(buf,&head,sizeof(HEAD));
    memcpy(buf+sizeof(HEAD),&backcaptcha,sizeof(BACKCAPTCHA));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::registerBusiness()
{
    std::cout << "---------处理注册业务-----------" << std::endl;
    // 初始化需要使用的变量
    SIGN sign;// 获取验证码请求包
    memcpy(&sign,this->buf+sizeof(HEAD),sizeof(SIGN));
    BACKSIGN backsign;// 返回验证码包
    memset(&backsign,0,sizeof(BACKSIGN));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[120] = {0};
    memcpy(crcbuf,&sign,sizeof(SIGN));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf,sizeof(SIGN));

    if(crc == head.crc){// 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    }else{// 包体数据错误，进行处理
        backsign.flag = 2;   // 失败
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[70] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&backsign,buf+sizeof(HEAD),sizeof(BACKSIGN));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[70] = {0};
    HEAD head;
    memset(&head,0,sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf,&backsign,sizeof(BACKSIGN));
    //配置包体
    head.businessType = 4;
    head.businessLength = sizeof(BACKSIGN);
    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKSIGN));
    //拼接返回包
    memcpy(buf,&head,sizeof(HEAD));
    memcpy(buf+sizeof(HEAD),&backsign,sizeof(BACKSIGN));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::loginBusiness()
{
    std::cout << "---------处理登录业务-----------" << std::endl;
    // 初始化需要使用的变量
    LOGIN login;// 获取登录请求包
    memcpy(&login,this->buf+sizeof(HEAD),sizeof(LOGIN));
    BACKLOGIN backlogin;// 返回登录包
    memset(&backlogin,0,sizeof(BACKLOGIN));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[100] = {0};
    memcpy(crcbuf,&login,sizeof(LOGIN));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf,sizeof(LOGIN));

    if(crc == head.crc){// 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    }else{// 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[500] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&backlogin,buf+sizeof(HEAD),sizeof(BACKLOGIN));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[500] = {0};
    HEAD head;
    memset(&head,0,sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf,&backlogin,sizeof(BACKLOGIN));
    //配置包体
    head.businessType = 6;
    head.businessLength = sizeof(BACKLOGIN);
    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKLOGIN));
    //拼接返回包
    memcpy(buf,&head,sizeof(HEAD));
    memcpy(buf+sizeof(HEAD),&backlogin,sizeof(BACKLOGIN));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::enterStopCarBusiness()
{
    std::cout << "---------处理入库业务-----------" << std::endl;
    // 初始化需要使用的变量
    STORE store;// 获取入库请求包
    memcpy(&store,this->buf+sizeof(HEAD),sizeof(STORE));
    BACKSTORE backstore;// 返回入库包
    memset(&backstore,0,sizeof(BACKSTORE));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[300] = {0};
    memcpy(crcbuf,&store,sizeof(STORE));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf,sizeof(STORE));

    if(crc == head.crc){// 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    }else{// 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[300] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&backstore,buf+sizeof(HEAD),sizeof(BACKSTORE));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[300] = {0};
    HEAD head;
    memset(&head,0,sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf,&backstore,sizeof(BACKSTORE));
    //配置包体
    head.businessType = 10;
    head.businessLength = sizeof(BACKSTORE);
    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKSTORE));
    //拼接返回包
    memcpy(buf,&head,sizeof(HEAD));
    memcpy(buf+sizeof(HEAD),&backstore,sizeof(BACKSTORE));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::saveCarPlateBusiness()
{
    std::cout << "---------处理传输文件业务-----------" << std::endl;
    // 初始化需要使用的变量
    FILEINFO fileInfo;// 获取入库请求包
    memcpy(&fileInfo,buf+sizeof(HEAD),sizeof(FILEINFO));
    LOSTPACKAGE lostPackage;// 文件返回包
    memset(&lostPackage,0,sizeof(LOSTPACKAGE));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[1200] = {0};
    memcpy(crcbuf,&fileInfo,sizeof(FILEINFO));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf,sizeof(FILEINFO));

    if(crc == head.crc){// 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    }else{// 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }
}

void ClientTask::endSaveCarPlateBusiness()
{
    std::cout << "---------处理传输文件（接收到了结束包）业务-----------" << std::endl;
    // 初始化需要使用的变量
    ENDBAG endBag;// 获取入库请求包
    memcpy(&endBag,this->buf+sizeof(HEAD),sizeof(ENDBAG));
    LOSTPACKAGE lostPackage;// 文件返回包
    memset(&lostPackage,0,sizeof(LOSTPACKAGE));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[60] = {0};
    memcpy(crcbuf,&endBag,sizeof(ENDBAG));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf,sizeof(ENDBAG));

    if(crc == head.crc){// 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    }else{// 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[4200] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&lostPackage,buf+sizeof(HEAD),sizeof(LOSTPACKAGE));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[4096] = {0};
    HEAD head;
    memset(&head,0,sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf,&lostPackage,sizeof(LOSTPACKAGE));
    //配置包体
    head.businessType = 8;
    head.businessLength = sizeof(LOSTPACKAGE);
    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(LOSTPACKAGE));
    //拼接返回包
    memcpy(buf,&head,sizeof(HEAD));
    memcpy(buf+sizeof(HEAD),&lostPackage,sizeof(LOSTPACKAGE));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::exitStopCarBusiness()
{
    std::cout << "---------处理出库业务-----------" << std::endl;
    // 初始化需要使用的变量
    CAR_OUT_REQUEST carOutRequest;// 获取出库请求包
    memcpy(&carOutRequest,this->buf+sizeof(HEAD),sizeof(CAR_OUT_REQUEST));
    CAR_OUT_RESPONSE carOutResponse;// 返回出库包
    memset(&carOutResponse,0,sizeof(CAR_OUT_RESPONSE));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[210] = {0};
    memcpy(crcbuf,&carOutRequest,sizeof(CAR_OUT_REQUEST));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf,sizeof(CAR_OUT_REQUEST));

    if(crc == head.crc){// 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    }else{// 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[120] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&carOutResponse,buf+sizeof(HEAD),sizeof(CAR_OUT_RESPONSE));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[120] = {0};
    HEAD head;
    memset(&head,0,sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf,&carOutResponse,sizeof(CAR_OUT_RESPONSE));
    //配置包体
    head.businessType = 12;
    head.businessLength = sizeof(CAR_OUT_RESPONSE);
    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(CAR_OUT_RESPONSE));
    //拼接返回包
    memcpy(buf,&head,sizeof(HEAD));
    memcpy(buf+sizeof(HEAD),&carOutResponse,sizeof(CAR_OUT_RESPONSE));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::modifyPlateBusiness()
{
    std::cout << "---------处理修改车牌业务-----------" << std::endl;
    // 初始化需要使用的变量
    CHANGEPLATE changePlate;// 获取修改车牌请求包
    memcpy(&changePlate, this->buf + sizeof(HEAD), sizeof(CHANGEPLATE));
    BACKCHANGEPLATE backChangePlate;// 返回修改车牌包
    memset(&backChangePlate, 0, sizeof(BACKCHANGEPLATE));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[80] = {0};
    memcpy(crcbuf, &changePlate, sizeof(CHANGEPLATE));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf, sizeof(CHANGEPLATE));

    if (crc == head.crc) { // 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    } else { // 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[30] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&backChangePlate, buf + sizeof(HEAD), sizeof(BACKCHANGEPLATE));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[30] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &backChangePlate, sizeof(BACKCHANGEPLATE));
    //配置包体
    head.businessType = 16;
    head.businessLength = sizeof(BACKCHANGEPLATE);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(BACKCHANGEPLATE));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &backChangePlate, sizeof(BACKCHANGEPLATE));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::videoUploadBusiness()
{
    std::cout << "---------处理视频上传业务-----------" << std::endl;
    // 初始化需要使用的变量
    VIDEO_UPLOAD videoUpload;// 获取视频上传请求包
    memcpy(&videoUpload, this->buf + sizeof(HEAD), sizeof(VIDEO_UPLOAD));
    VIDEO_UPLOAD_RESPONSE videoUploadResponse;// 返回视频上传包
    memset(&videoUploadResponse, 0, sizeof(VIDEO_UPLOAD_RESPONSE));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[360] = {0};
    memcpy(crcbuf, &videoUpload, sizeof(VIDEO_UPLOAD));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf, sizeof(VIDEO_UPLOAD));

    if (crc == head.crc) { // 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    } else { // 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[30] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&videoUploadResponse, buf + sizeof(HEAD), sizeof(VIDEO_UPLOAD_RESPONSE));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[30] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &videoUploadResponse, sizeof(VIDEO_UPLOAD_RESPONSE));
    //配置包体
    head.businessType = 14;
    head.businessLength = sizeof(VIDEO_UPLOAD_RESPONSE);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(VIDEO_UPLOAD_RESPONSE));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &videoUploadResponse, sizeof(VIDEO_UPLOAD_RESPONSE));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::videoQueryBusiness()
{
    std::cout << "---------处理视频查询业务-----------" << std::endl;
    // 初始化需要使用的变量
    VIDEOQUERY videoQuery;// 获取视频查询请求包
    memcpy(&videoQuery, this->buf + sizeof(HEAD), sizeof(VIDEOQUERY));
    VIDEO_QUERY_BACK videoQueryBack;// 返回视频查询包
    memset(&videoQueryBack, 0, sizeof(VIDEO_QUERY_BACK));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[30] = {0};
    memcpy(crcbuf, &videoQuery, sizeof(VIDEOQUERY));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf, sizeof(VIDEOQUERY));

    if (crc == head.crc) { // 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    } else { // 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[2100] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&videoQueryBack, buf + sizeof(HEAD), sizeof(VIDEO_QUERY_BACK));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[2100] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &videoQueryBack, sizeof(VIDEO_QUERY_BACK));
    //配置包体
    head.businessType = 18;
    head.businessLength = sizeof(VIDEO_QUERY_BACK);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(VIDEO_QUERY_BACK));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &videoQueryBack, sizeof(VIDEO_QUERY_BACK));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::videoNextPageBusiness()
{
    std::cout << "---------处理视频下一页业务-----------" << std::endl;
    // 初始化需要使用的变量
    VIDEO_NEXT_PAGE videoNextPage;// 获取视频下一页请求包
    memcpy(&videoNextPage, this->buf + sizeof(HEAD), sizeof(VIDEO_NEXT_PAGE));
    VIDEO_NEXT_PAGE_BACK videoNextPageBack;// 返回视频上传包
    memset(&videoNextPageBack, 0, sizeof(VIDEO_NEXT_PAGE_BACK));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[100] = {0};
    memcpy(crcbuf, &videoNextPage, sizeof(VIDEO_NEXT_PAGE));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf, sizeof(VIDEO_NEXT_PAGE));

    if (crc == head.crc) { // 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    } else { // 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[1700] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&videoNextPageBack, buf + sizeof(HEAD), sizeof(VIDEO_NEXT_PAGE_BACK));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[1700] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &videoNextPageBack, sizeof(VIDEO_NEXT_PAGE_BACK));
    //配置包体
    head.businessType = 20;
    head.businessLength = sizeof(VIDEO_NEXT_PAGE_BACK);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(VIDEO_NEXT_PAGE_BACK));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &videoNextPageBack, sizeof(VIDEO_NEXT_PAGE_BACK));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::videoPlayBusiness()
{
    std::cout << "---------处理视频播放业务-----------" << std::endl;
    // 初始化需要使用的变量
    VIDEO_PLAY videoPlay;// 获取视频播放请求包
    memcpy(&videoPlay, this->buf + sizeof(HEAD), sizeof(VIDEO_PLAY));
    BACK_VIDEO_PLAY backVideoPlay;// 返回视频播放包
    memset(&backVideoPlay, 0, sizeof(BACK_VIDEO_PLAY));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[130] = {0};
    memcpy(crcbuf, &videoPlay, sizeof(VIDEO_PLAY));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf, sizeof(VIDEO_PLAY));

    if (crc == head.crc) { // 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    } else { // 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[250] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&backVideoPlay, buf + sizeof(HEAD), sizeof(BACK_VIDEO_PLAY));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[250] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &backVideoPlay, sizeof(BACK_VIDEO_PLAY));
    //配置包体
    head.businessType = 22;
    head.businessLength = sizeof(BACK_VIDEO_PLAY);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(BACK_VIDEO_PLAY));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &backVideoPlay, sizeof(BACK_VIDEO_PLAY));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::videoPlayOverBusiness()
{
    std::cout << "---------处理视频播放结束业务-----------" << std::endl;
    // 初始化需要使用的变量
    VIDEO_PLAY_OVER videoPlayOver;// 获取视频播放结束请求包
    memcpy(&videoPlayOver, this->buf + sizeof(HEAD), sizeof(VIDEO_PLAY_OVER));
    VIDEO_PLAY_OVER_BACK videoPlayOverBack;// 返回视频播放结束包
    memset(&videoPlayOverBack, 0, sizeof(VIDEO_PLAY_OVER_BACK));

    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    char crcbuf[130] = {0};
    memcpy(crcbuf, &videoPlayOver, sizeof(VIDEO_PLAY_OVER));
    uint32_t crc = CRC::getinterface()->CRC32(crcbuf, sizeof(VIDEO_PLAY_OVER));

    if (crc == head.crc) { // 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    } else { // 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[30] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&videoPlayOverBack, buf + sizeof(HEAD), sizeof(VIDEO_PLAY_OVER_BACK));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[30] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &videoPlayOverBack, sizeof(VIDEO_PLAY_OVER_BACK));
    //配置包体
    head.businessType = 24;
    head.businessLength = sizeof(VIDEO_PLAY_OVER_BACK);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(VIDEO_PLAY_OVER_BACK));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &videoPlayOverBack, sizeof(VIDEO_PLAY_OVER_BACK));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::parkBusiness()
{
    std::cout << "---------处理停车场业务-----------" << std::endl;
    // 初始化需要使用的变量
    PARKBACK parkBack;// 返回视频播放结束包
    memset(&parkBack, 0, sizeof(PARKBACK));

    // 将客户端的包写入共享内存中
    memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
    std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;

    // 读取后置服务器发来的数据
    char buf[30] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&parkBack, buf + sizeof(HEAD), sizeof(PARKBACK));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[30] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &parkBack, sizeof(PARKBACK));
    //配置包体
    head.businessType = 26;
    head.businessLength = sizeof(PARKBACK);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(PARKBACK));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &parkBack, sizeof(PARKBACK));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

void ClientTask::carNumberQueryBusiness(int type)
{
    std::cout << "---------处理车辆查询业务-----------" << std::endl;
    CAR_QUERY_BACK carQueryBack;// 返回车辆查询结束包
    memset(&carQueryBack, 0, sizeof(CAR_QUERY_BACK));

    uint32_t crc = 0;
    // 对包体进行crc校准
    // 根据包头的crc校验码和数据的长度去判断包体是否正确
    if(type == 27){
        CAR_NUMBER_QUERY carNumberQuery;// 获取车牌号查询请求包
        memcpy(&carNumberQuery, this->buf + sizeof(HEAD), sizeof(CAR_NUMBER_QUERY));
        char crcbuf[60] = {0};
        memcpy(crcbuf, &carNumberQuery, sizeof(CAR_NUMBER_QUERY));
        crc = CRC::getinterface()->CRC32(crcbuf, sizeof(CAR_NUMBER_QUERY));
    }else if(type == 28){
        CAR_TIME_AREA_QUERY carTimeAreaQuery;// 获取时间区间查询请求包
        memcpy(&carTimeAreaQuery, this->buf + sizeof(HEAD), sizeof(CAR_TIME_AREA_QUERY));
        char crcbuf[150] = {0};
        memcpy(crcbuf, &carTimeAreaQuery, sizeof(CAR_TIME_AREA_QUERY));
        crc = CRC::getinterface()->CRC32(crcbuf, sizeof(CAR_TIME_AREA_QUERY));
    }else{
        CAR_TIME_AREA_CAR_NUMBER_QUERY carTimeAreaCarNumberQuery;// 获取车牌号+时间区间查询请求包
        memcpy(&carTimeAreaCarNumberQuery, this->buf + sizeof(HEAD), sizeof(CAR_TIME_AREA_CAR_NUMBER_QUERY));
        char crcbuf[170] = {0};
        memcpy(crcbuf, &carTimeAreaCarNumberQuery, sizeof(CAR_TIME_AREA_CAR_NUMBER_QUERY));
        crc = CRC::getinterface()->CRC32(crcbuf, sizeof(CAR_TIME_AREA_CAR_NUMBER_QUERY));
    }
    

    if (crc == head.crc) { // 包体数据正确，进行处理
        std::cout << "接收的包体数据正确" << std::endl;
        // 将客户端的包写入共享内存中
        memoryPool.writeData(this->buf, fd, SERVER_HEAD);  // 前置服务器写入数据
        std::cout << "数据成功写入共享内存（前置->后置）" << std::endl;
    } else { // 包体数据错误，进行处理
        std::cout << "接收的包体数据错误" << std::endl;
        std::cout << "crc" << crc << std::endl;
        std::cout << "head.crc" << head.crc << std::endl;
    }

    // 读取后置服务器发来的数据
    char buf[2200] = {0};
    try {
        if (!memoryPool.readData(buf, fd, SERVER_HEAD)) {  // 前置服务器读取数据
            std::cerr << "读取后置服务器数据超时" << std::endl;
            // 这里可以添加超时包的处理逻辑
            return;  // 暂时直接返回，后续可以改为发送超时包
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;  // 读取失败时直接返回，避免继续处理
    }

    // 解析后置服务器返回的数据
    memcpy(&carQueryBack, buf + sizeof(HEAD), sizeof(CAR_QUERY_BACK));

    // 配置包头，拼接返回包
    // 初始化对应变量
    char tempbuf[2200] = {0};
    HEAD head;
    memset(&head, 0, sizeof(HEAD));
    // 方便计算包体CRC码
    memcpy(tempbuf, &carQueryBack, sizeof(CAR_QUERY_BACK));
    //配置包体
    head.businessType = 30;
    head.businessLength = sizeof(CAR_QUERY_BACK);
    head.crc = CRC::getinterface()->CRC32(tempbuf, sizeof(CAR_QUERY_BACK));
    //拼接返回包
    memcpy(buf, &head, sizeof(HEAD));
    memcpy(buf + sizeof(HEAD), &carQueryBack, sizeof(CAR_QUERY_BACK));
    size_t total_size = sizeof(HEAD) + head.businessLength;
    write(fd, buf, total_size);
}

// void heartbeatBusiness()
// {
//     std::cout << "---------处理心跳业务-----------" << std::endl;
//     // 初始化需要使用的变量
//     HEART_CLIENT heartClient;// 获取心跳包
//     memcpy(&heartClient, this->buf + sizeof(HEAD), sizeof(HEART_CLIENT));

//     // 对包体进行crc校准
//     // 根据包头的crc校验码和数据的长度去判断包体是否正确
//     char crcbuf[80] = {0};
//     memcpy(crcbuf, &heartClient, sizeof(HEART_CLIENT));
//     uint32_t crc = CRC::getinterface()->CRC32(crcbuf, sizeof(HEART_CLIENT));

//     if (crc == head.crc) { // 包体数据正确，进行处理
//         std::cout << "接收的包体数据正确" << std::endl;
//         // 收到该客户端的心跳包，重置时间

//     } else { // 包体数据错误，进行处理
//         std::cout << "接收的包体数据错误" << std::endl;
//     }
// }