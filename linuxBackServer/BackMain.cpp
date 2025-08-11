#include <iostream>
#include "../tool/memoryPool/memoryPool.h"
#include "../tool/agreement/agreement.h"
#include "../tool/agreement/crc.h"
#include "../MySQL/User.h"
#include "../MySQL/Car.h"
#include "../MySQL/CarIn.h"
#include "../MySQL/CarOut.h"
#include "../MySQL/ParkInfo.h"
#include "../MySQL/Video.h"
#include <string.h>
#include <semaphore.h>
#include <map>
#include <queue>
#include <sys/stat.h>  // 添加头文件
#include <ctime>  // 添加头文件
#include <iomanip>  // 添加头文件
#include <sstream>  // 添加头文件
#include <cmath>  // 添加头文件

#define SQL_NAME "stopcar"

#define ERROR(errinfo); \
    if (head.businessLength != errinfo) { \
        std::cerr << "数据长度不匹配" << std::endl; \
        std::cout << "head.businessType: " << head.businessType << std::endl; \
        std::cout << "head.businessLength: " << head.businessLength << std::endl; \
        std::cout << "期望的 businessLength: " << errinfo << std::endl; \
    }

// 通过电话号码查看验证码
std::map<std::string,std::string> userMap;
// 存储文件碎片
std::queue<FILEINFO> fileQueue;
std::map<int,FILEINFO> fileMap;
// 产生随机验证码
std::string generateCaptcha();

// 函数原型声明
std::string formatDuration(int durationSeconds);
std::tm parseTime(const std::string& timeStr);
double calculateParkingDuration(const std::string& entryTimeStr, const std::string& exitTimeStr);
int calculateParkingFee(double durationSeconds);

int main() 
{
    system("clear");
    std::cout << "------------后置服务器启动-----------"<< std::endl;
    
    // 检查并创建 img 文件夹
    if (mkdir("img", 0777) == -1 && errno != EEXIST) {
        std::cerr << "无法创建 img 文件夹" << std::endl;
        return -1;
    }
    
    MemoryPool& memoryPool = MemoryPool::getInstance();
    
    HEAD head;
    char buf[LARGE_BLOCK_SIZE] = { 0 };
    
    while(1){
        try {
            int clientId = -1;
            if (!memoryPool.readData(buf, clientId, SERVER_BACK)) {
                std::cerr << "读取数据失败，可能超时" << std::endl;
                continue;  // 继续下一次循环
            }
            std::cout << "读取到了来自客户端 " << clientId << " 的数据" << std::endl;
            memcpy(&head, buf, sizeof(HEAD));  // 必须先解析包头
            char* businessData = buf + sizeof(HEAD);

            switch(head.businessType) {
                case 1: {// 获取验证码请求包                            
                    ERROR(sizeof(GETCAPTCHA));  // 检查包体大小                         
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    GETCAPTCHA captcha; // 获取验证码请求包
                    memcpy(&captcha, businessData, sizeof(GETCAPTCHA));
                    // 处理验证码业务...
                    // 1.向该手机号发送验证码
                    // 产生随机验证码
                    std::string randomCaptcha = generateCaptcha();
                    std::cout << "产生的验证码为：" << randomCaptcha << std::endl;
                    // 通过API发送验证码
                    // 暂略 pass
                    // 2.将验证码保存到map中    
                    userMap[captcha.PhoneNumber] = randomCaptcha;
                    // 3.向前置服务器发送返回包
                    // 初始化变量
                    const int bufsize = sizeof(BACKCAPTCHA)+2;
                    char tempbuf[bufsize] = {0};
                    BACKCAPTCHA backCaptcha;
                    // 写包体
                    backCaptcha.flag = 1;
                    memcpy(tempbuf,&backCaptcha,sizeof(BACKCAPTCHA));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKCAPTCHA));
                    // 写包头
                    head.businessLength = sizeof(BACKCAPTCHA);
                    head.businessType = 2;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&backCaptcha,sizeof(BACKCAPTCHA));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 3:{// 用户注册发送包
                    ERROR(sizeof(SIGN));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    SIGN sign;
                    memcpy(&sign, businessData, sizeof(SIGN));
                    // 处理注册业务...
                    // 初始化变量
                    const int bufsize = sizeof(BACKSIGN)+2;
                    char tempbuf[bufsize] = {0};
                    BACKSIGN backsign;
                    memset(&backsign,0,sizeof(BACKSIGN));
                    // 1.查看该用户是否已经存在
                    User user(SQL_NAME);
                    if(user.findUserOnPhoneNumber(sign.PhoneNumber)){
                        backsign.flag = 2; // 用户已存在
                        strcpy(backsign.captcha,"用户已存在");
                    }
                    // 2.验证码是否正确
                    if(strcmp(sign.captcha,userMap[sign.PhoneNumber].c_str()) != 0){
                        backsign.flag = 2; // 验证码错误
                        strcpy(backsign.captcha,"验证码错误");
                    }
                    // 3.将用户信息保存到数据库中   
                    user.addUser(sign.userName,sign.userPassword,sign.PhoneNumber,sign.captcha);
                    // 4.向前置服务器发送返回包
                    // 写包体
                    if(backsign.flag != 2){
                        backsign.flag = 1;// 注册成功
                        strcpy(backsign.captcha,"注册成功");
                        // 注册成功清除map容器
                        userMap.erase(sign.PhoneNumber);
                    }
                    memcpy(tempbuf,&backsign,sizeof(BACKSIGN));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKSIGN));
                    // 写包头
                    head.businessLength = sizeof(BACKSIGN);
                    head.businessType = 4;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&backsign,sizeof(BACKSIGN));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 5:{// 用户登录发送包
                    ERROR(sizeof(LOGIN));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    LOGIN login;
                    memcpy(&login, businessData, sizeof(LOGIN));
                    // 处理登录业务...
                    // 初始化变量
                    const int bufsize = sizeof(BACKLOGIN)+2;
                    char tempbuf[bufsize] = {0};
                    BACKLOGIN backLogin;
                    memset(&backLogin,0,sizeof(BACKLOGIN));
                    // 1.查看该用户是否已经存在
                    User user(SQL_NAME);
                    if(!user.findUserOnPhoneNumber(login.phoneNum)){
                        backLogin.flag = 3; // 用户未注册
                    }
                    // 2.密码是否正确
                    if(strcmp(user.findUserOnPwdByPhoneNumber(login.phoneNum).c_str(),login.userPassword) != 0
                    && backLogin.flag != 3){
                        backLogin.flag = 2;// 密码错误
                    }
                    // 3.向前置服务器发送返回包
                    // 写包体
                    if(backLogin.flag != 2 && backLogin.flag != 3){
                        backLogin.flag = 1;// 登录成功
                    }
                    // 4.获取剩余车位信息
                    ParkInfo parkInfo(SQL_NAME);
                    backLogin.remaincar.remainspace = parkInfo.getRemainSpace();
                    backLogin.remaincar.id = clientId;
                    // 5.获取停车场信息
                    CarIn carIn(SQL_NAME);
                    std::vector<INBOUND> inboundList = carIn.getLatestInboundInfo(4);
                    for (size_t i = 0; i < inboundList.size() && i < 4; i++) {
                        backLogin.inbound[i] = inboundList[i];
                    }
                    memcpy(tempbuf,&backLogin,sizeof(BACKLOGIN));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKLOGIN));
                    // 写包头
                    head.businessLength = sizeof(BACKLOGIN);
                    head.businessType = 6;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&backLogin,sizeof(BACKLOGIN));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 9:{// 入库请求包
                    ERROR(sizeof(STORE));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    STORE store; // 获取入库请求包
                    memcpy(&store, businessData, sizeof(STORE));
                    // 处理入库业务
                    // 初始化变量
                    const int bufsize = sizeof(BACKSTORE)+2;
                    char tempbuf[bufsize] = {0};
                    BACKSTORE backstore;
                    // 判断该车是否已经在车库内
                    CarIn carin(SQL_NAME);
                    ParkInfo parkInfo(SQL_NAME);
                    if(carin.findCarInByPlate(store.plate)){//该车已经入库，无法重复入库
                        backstore.flag = 2;     // 入库失败
                    } else{
                        backstore.flag = 1;     // 入库成功
                        // 更新数据库的数据
                        carin.addCarIn(store.plate,store.time,store.location,store.path,clientId);
                        parkInfo.updateParkInfo();
                    }
                    // 获取剩余车位信息
                    backstore.reamincar.remainspace = parkInfo.getRemainSpace();
                    backstore.reamincar.id = clientId;
                    // 获取停车场信息
                    CarIn carIn(SQL_NAME);
                    std::vector<INBOUND> inboundList = carIn.getLatestInboundInfo(4);
                    for (size_t i = 0; i < inboundList.size() && i < 4; i++) {
                        backstore.inbound[i] = inboundList[i];
                    }

                    memcpy(tempbuf,&backstore,sizeof(BACKSTORE));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKSTORE));
                    // 写包头
                    head.businessLength = sizeof(BACKSTORE);
                    head.businessType = 10;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&backstore,sizeof(BACKSTORE));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 7:{// 文件请求包
                    ERROR(sizeof(FILEINFO));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    FILEINFO fileInfo; // 获取文件请求包
                    memcpy(&fileInfo, businessData, sizeof(FILEINFO));  
                    // 处理文件业务
                    // 将文件暂时存入queue和map中等待结束包
                    fileQueue.push(fileInfo);
                    fileMap[fileInfo.fileindex] = fileInfo;
                    break;
                }
                case 98:{// 结束包
                    ERROR(sizeof(ENDBAG));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    ENDBAG endBag; // 获取文件结束包
                    memcpy(&endBag, businessData, sizeof(ENDBAG));                    
                    // 处理入库业务
                    // 初始化变量
                    const int bufsize = sizeof(LOSTPACKAGE)+2;
                    char tempbuf[bufsize] = {0};
                    LOSTPACKAGE lostPackage;
                    int index = 0;
                    // 检查文件包是否有漏,将漏掉的文件碎片数存入返回包中
                    if(fileQueue.size() == fileQueue.front().num){
                        lostPackage.lostId[0] = -1; // 文件完整
                        // 文件完整则进行拼接并清空map和queue为下一个文件做准备
                        // 拼接文件
                        std::string filePath = "img/" + std::string(fileQueue.front().fileName);  // 将文件保存到 img 文件夹下
                        int fileFd = open(filePath.c_str(), O_CREAT | O_WRONLY, 0777);
                        if (fileFd < 0) {
                            std::cerr << "无法创建文件: " << filePath << std::endl;
                            break;
                        }
                        for(int i = 1; i < fileQueue.front().num+1; ++i){
                            if (fileMap.find(i) != fileMap.end()) {
                                FILEINFO fragment = fileMap[i];
                                if (fragment.length <= 0 || fragment.buf == nullptr) {
                                    std::cerr << "文件碎片 " << fragment.fileindex << " 数据不完整" << std::endl;
                                    continue;
                                }
                                ssize_t bytesWritten = write(fileFd, fragment.buf, fragment.length);
                                if (bytesWritten != fragment.length) {
                                    std::cerr << "文件碎片 " << fragment.fileindex << " 写入不完整" << std::endl;
                                }
                            } 
                        }
                        close(fileFd);  // 关闭文件
                        std::cout << "拼接文件成功，保存路径: " << filePath << std::endl;
                        // 清除容器
                        fileMap.clear();
                        fileQueue = std::queue<FILEINFO>();  // 清空队列
                    }else{
                        for(int i = 1; i < fileQueue.front().num+1; ++i){
                            if(fileMap.find(i) == fileMap.end()){// 缺少的文件
                                lostPackage.lostId[index++] = i;
                            }
                        }
                    }
                    memcpy(tempbuf,&lostPackage,sizeof(LOSTPACKAGE));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(LOSTPACKAGE));
                    // 写包头
                    head.businessLength = sizeof(LOSTPACKAGE);
                    head.businessType = 8;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&lostPackage,sizeof(LOSTPACKAGE));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 11:{// 出场请求包
                    ERROR(sizeof(CAR_OUT_REQUEST));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    CAR_OUT_REQUEST carOutRequest; // 获取出库请求包
                    memcpy(&carOutRequest, businessData, sizeof(CAR_OUT_REQUEST));
                    // 出库业务
                    // 初始化变量
                    const int bufsize = sizeof(CAR_OUT_RESPONSE)+2;
                    char tempbuf[bufsize] = {0};
                    CAR_OUT_RESPONSE carOutResponse;
                    // 写包体
                    // 1.判断该车是否在停车场内
                    Car car(SQL_NAME);
                    if(car.findCarByPlate(carOutRequest.carNumber) == false){// 车不在停车场中
                        carOutResponse.flag = 2;    // 失败
                    }
                    // 2.判断该车的状态
                    if(carOutResponse.flag != 2 && car.getParkStatus(carOutRequest.carNumber) != 1){// 1--入场 0--出场
                        carOutResponse.flag = 2;    // 失败
                    }
                    if(carOutResponse.flag != 2){
                        carOutResponse.flag = 1;    // 成功
                        CarIn carIn(SQL_NAME);
                        // 获取入场时间
                        std::string entryTime = carIn.queryCarEntryTimeByPlate(carOutRequest.carNumber);
                        // 获取出场时间
                        std::string exitTime = carOutRequest.time;
                        // 计算停车时间
                        double durationSeconds = calculateParkingDuration(entryTime, exitTime);
                        std::string durationFormatted = formatDuration(durationSeconds);
                        // 将停车时间写入返回包
                        strcpy(carOutResponse.Parktime, durationFormatted.c_str());
                        strcpy(carOutResponse.Storetime,entryTime.c_str());
                        // 计算停车费用
                        int fee = calculateParkingFee(durationSeconds);
                        // 将费用写入返回包
                        carOutResponse.money = fee;
                        // 剩余车位结构体
                        ParkInfo parkInfo(SQL_NAME);
                        carOutResponse.reamincar.remainspace = parkInfo.getRemainSpace();
                        carOutResponse.reamincar.id = clientId;
                        // 更新数据库
                        CarOut carOut(SQL_NAME);
                        carOut.addCarOut(carOutRequest.carNumber, exitTime, carOutRequest.comeOutLocation, std::to_string(fee), entryTime, carOutRequest.platePath, clientId);
                    }
                    memcpy(tempbuf,&carOutResponse,sizeof(CAR_OUT_RESPONSE));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(CAR_OUT_RESPONSE));
                    // 写包头
                    head.businessLength = sizeof(CAR_OUT_RESPONSE);
                    head.businessType = 12;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&carOutResponse,sizeof(CAR_OUT_RESPONSE));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 15:{// 修改车牌请求包
                    ERROR(sizeof(CHANGEPLATE));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    CHANGEPLATE changePlate;
                    memcpy(&changePlate, businessData, sizeof(CHANGEPLATE));
                    // 处理修改车牌业务...
                    // 初始化变量
                    const int bufsize = sizeof(BACKCHANGEPLATE)+2;
                    char tempbuf[bufsize] = {0};
                    BACKCHANGEPLATE backChangePlate;
                    memset(&backChangePlate,0,sizeof(BACKCHANGEPLATE));
                    // 修改车牌 
                    Car car(SQL_NAME);
                    car.updateCarPlate(changePlate.oldPlate,changePlate.newPlate);
                    backChangePlate.flag = 1; // 修改成功
                    memcpy(tempbuf,&backChangePlate,sizeof(BACKCHANGEPLATE));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACKCHANGEPLATE));
                    // 写包头
                    head.businessLength = sizeof(BACKCHANGEPLATE);
                    head.businessType = 16;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&backChangePlate,sizeof(BACKCHANGEPLATE));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 13:{// 视频上传请求包
                    ERROR(sizeof(VIDEO_UPLOAD));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    VIDEO_UPLOAD videoUpload;
                    memcpy(&videoUpload, businessData, sizeof(VIDEO_UPLOAD));
                    // 处理视频上传业务...
                    // 初始化变量
                    const int bufsize = sizeof(VIDEO_UPLOAD_RESPONSE)+2;
                    char tempbuf[bufsize] = {0};
                    VIDEO_UPLOAD_RESPONSE videoUploadResponse;
                    memset(&videoUploadResponse,0,sizeof(VIDEO_UPLOAD_RESPONSE));
                    //更新数据库
                    Video video(SQL_NAME);
                    video.addVideo(videoUpload.videoFilename, videoUpload.videoPath, videoUpload.videoTotalFrame,
                    videoUpload.coverPath, videoUpload.time, videoUpload.userId);
                    //写包体
                    videoUploadResponse.flag = 1; // 上传成功
                    memcpy(tempbuf,&videoUploadResponse,sizeof(VIDEO_UPLOAD_RESPONSE));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(VIDEO_UPLOAD_RESPONSE));
                    // 写包头
                    head.businessLength = sizeof(VIDEO_UPLOAD_RESPONSE);
                    head.businessType = 14;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&videoUploadResponse,sizeof(VIDEO_UPLOAD_RESPONSE));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 17:{// 视频查询请求包
                    ERROR(sizeof(VIDEOQUERY));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    VIDEOQUERY videoQuery;
                    memcpy(&videoQuery, businessData, sizeof(VIDEOQUERY));
                    // 处理视频查询业务
                    // 初始化变量
                    const int bufsize = sizeof(VIDEO_QUERY_BACK)+2;
                    char tempbuf[bufsize] = {0};
                    VIDEO_QUERY_BACK videoQueryBack;
                    memset(&videoQueryBack,0,sizeof(VIDEO_QUERY_BACK));
                    // 根据用户ID获取视频列表
                    Video video(SQL_NAME);
                    std::vector<VIDEO_ITEM> videoList = video.getVideosByUserId(videoQuery.user_id);
                    // 将视频列表填充到返回包中
                    for (size_t i = 0; i < videoList.size() && i < 15; i++) {
                        videoQueryBack.videoList[i] = videoList[i];
                    }
                    // 按日显示就返回30个日期，按月就返回12个月
                    if (videoQuery.type == 1) { // 按月
                        // 获取当前系统时间
                        time_t now = time(nullptr);
                        struct tm* tm_now = localtime(&now);
                        // 填充12个月的日期
                        for (int i = 0; i < 12; i++) {
                            snprintf(videoQueryBack.videoTime[i].DateTime, sizeof(videoQueryBack.videoTime[i].DateTime), "%04d-%02d", tm_now->tm_year + 1900, i + 1);
                        }
                    } else if (videoQuery.type == 2) { // 按日
                        // 获取当前系统时间
                        time_t now = time(nullptr);
                        struct tm* tm_now = localtime(&now);
                        // 填充30天的日期
                        for (int i = 0; i < 30; i++) {
                            snprintf(videoQueryBack.videoTime[i].DateTime, sizeof(videoQueryBack.videoTime[i].DateTime), "%04d-%02d-%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, i + 1);
                        }
                    }
                    // 写包体
                    videoQueryBack.flag = 1; // 查询成功
                    memcpy(tempbuf,&videoQueryBack,sizeof(VIDEO_QUERY_BACK));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(VIDEO_QUERY_BACK));
                    // 写包头
                    head.businessLength = sizeof(VIDEO_QUERY_BACK);
                    head.businessType = 18;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&videoQueryBack,sizeof(VIDEO_QUERY_BACK));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 19:{// 视频下一页请求包
                    ERROR(sizeof(VIDEO_NEXT_PAGE));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    VIDEO_NEXT_PAGE videoNextPage;
                    memcpy(&videoNextPage, businessData, sizeof(VIDEO_NEXT_PAGE));
                    // 处理视频下一页业务...
                    // 初始化变量
                    const int bufsize = sizeof(VIDEO_NEXT_PAGE_BACK)+2;
                    char tempbuf[bufsize] = {0};
                    VIDEO_NEXT_PAGE_BACK videoNextPageBack;
                    memset(&videoNextPageBack,0,sizeof(VIDEO_NEXT_PAGE_BACK));
                    // 根据用户ID获取视频列表
                    Video video(SQL_NAME);
                    std::vector<VIDEO_ITEM> videoList = video.getVideosByUserIdAndPage(videoNextPage.use_id,videoNextPage.next_page,videoNextPage.query_time);
                    // 将视频列表填充到返回包中
                    for (size_t i = 0; i < videoList.size() && i < 15; i++) {
                        videoNextPageBack.videoList[i] = videoList[i];
                    }
                    //写包体
                    videoNextPageBack.flag = 1;// 成功
                    memcpy(tempbuf,&videoNextPageBack,sizeof(VIDEO_NEXT_PAGE_BACK));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(VIDEO_NEXT_PAGE_BACK));
                    // 写包头
                    head.businessLength = sizeof(VIDEO_NEXT_PAGE_BACK);
                    head.businessType = 20;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&videoNextPageBack,sizeof(VIDEO_NEXT_PAGE_BACK));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 21:{// 视频播放请求包
                    ERROR(sizeof(VIDEO_PLAY));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    VIDEO_PLAY videoPlay;
                    memcpy(&videoPlay, businessData, sizeof(VIDEO_PLAY));
                    // 处理视频播放业务...
                    // 初始化变量
                    const int bufsize = sizeof(BACK_VIDEO_PLAY)+2;
                    char tempbuf[bufsize] = {0};
                    BACK_VIDEO_PLAY backVideoPlay;
                    memset(&backVideoPlay,0,sizeof(BACK_VIDEO_PLAY));
                    // 视频播放返回包
                    Video video(SQL_NAME);
                    VideoInfo videoInfo = video.getVideoInfoByUserIdAndFilename(videoPlay.userid,videoPlay.videoName);
                    strcpy(backVideoPlay.videoFilename,videoInfo.filename.c_str());
                    strcpy(backVideoPlay.videoPath,videoInfo.path.c_str());
                    backVideoPlay.videoTotalFrame = videoInfo.totalFrame;
                    backVideoPlay.record_frame = videoInfo.currentFrame;
                    memcpy(tempbuf,&backVideoPlay,sizeof(BACK_VIDEO_PLAY));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(BACK_VIDEO_PLAY));
                    // 写包头
                    head.businessLength = sizeof(BACK_VIDEO_PLAY);
                    head.businessType = 22;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&backVideoPlay,sizeof(BACK_VIDEO_PLAY));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 23:{// 视频播放结束请求包
                    ERROR(sizeof(VIDEO_PLAY_OVER));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    VIDEO_PLAY_OVER videoPlayOver;
                    memcpy(&videoPlayOver, businessData, sizeof(VIDEO_PLAY_OVER));
                    // 初始化变量
                    const int bufsize = sizeof(VIDEO_PLAY_OVER_BACK)+2;
                    char tempbuf[bufsize] = {0};
                    VIDEO_PLAY_OVER_BACK videoPlayOverBack;
                    memset(&videoPlayOverBack,0,sizeof(VIDEO_PLAY_OVER_BACK));
                    Video video(SQL_NAME);
                    video.updateCurrentFrameByUserIdAndFilename(videoPlayOver.user_id,videoPlayOver.videoName,videoPlayOver.record_frame);
                    videoPlayOverBack.flag = 1; // 成功
                    memcpy(tempbuf,&videoPlayOverBack,sizeof(VIDEO_PLAY_OVER_BACK));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(VIDEO_PLAY_OVER_BACK));
                    // 写包头
                    head.businessLength = sizeof(VIDEO_PLAY_OVER_BACK);
                    head.businessType = 24;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&videoPlayOverBack,sizeof(VIDEO_PLAY_OVER_BACK));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 25:{// 停车场请求包
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    // 处理停车场
                    // 初始化变量
                    const int bufsize = sizeof(PARKBACK)+2;
                    char tempbuf[bufsize] = {0};
                    PARKBACK parkBack;
                    memset(&parkBack,0,sizeof(PARKBACK));
                    ParkInfo parkInfo(SQL_NAME);
                    parkBack.freeSpaceNum = parkInfo.getRemainSpace();
                    parkBack.totalCarNum = parkInfo.getTotalSpace();
                    memcpy(tempbuf,&parkBack,sizeof(PARKBACK));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(PARKBACK));
                    // 写包头
                    head.businessLength = sizeof(PARKBACK);
                    head.businessType = 26;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&parkBack,sizeof(PARKBACK));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 27:{// 车牌号查询  
                    ERROR(sizeof(CAR_NUMBER_QUERY));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    CAR_NUMBER_QUERY carNumberQuery;
                    memcpy(&carNumberQuery, businessData, sizeof(CAR_TIME_AREA_QUERY));
                    // 初始化变量
                    const int bufsize = sizeof(CAR_QUERY_BACK)+2;
                    char tempbuf[bufsize] = {0};
                    CAR_QUERY_BACK carQueryBack;
                    memset(&carQueryBack,0,sizeof(CAR_QUERY_BACK));
                    // 根据车牌号查询
                    Car car(SQL_NAME);
                    std::cout << "carNumberQuery.page" << carNumberQuery.page << std::endl;
                    std::vector<CAR_QUERY> carQueryBackVector = car.queryCarInfoByUserIdAndPlate(carNumberQuery.user_id,carNumberQuery.carNumber,carNumberQuery.page);
                    for(int i = 0; i < 7; ++i){
                        strcpy(carQueryBack.carquery[i].car_number, carQueryBackVector[i].car_number);
                        strcpy(carQueryBack.carquery[i].entry_time, carQueryBackVector[i].entry_time);
                        strcpy(carQueryBack.carquery[i].out_time, carQueryBackVector[i].out_time);
                        strcpy(carQueryBack.carquery[i].entry_picture_path, carQueryBackVector[i].entry_picture_path);
                        strcpy(carQueryBack.carquery[i].out_picture_path, carQueryBackVector[i].out_picture_path);
                        carQueryBack.carquery[i].fee = carQueryBackVector[i].fee;
                        std::cout << carQueryBackVector[i].car_number << std::endl;
                        std::cout << carQueryBackVector[i].entry_time << std::endl;
                        std::cout << carQueryBackVector[i].out_time << std::endl;
                        std::cout << carQueryBackVector[i].entry_picture_path << std::endl;
                        std::cout << carQueryBackVector[i].out_picture_path << std::endl;
                        std::cout << carQueryBackVector[i].fee << std::endl;
                    }
                    memcpy(tempbuf,&carQueryBack,sizeof(CAR_QUERY_BACK));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(CAR_QUERY_BACK));
                    // 写包头
                    head.businessLength = sizeof(CAR_QUERY_BACK);
                    head.businessType = 30;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&carQueryBack,sizeof(CAR_QUERY_BACK));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 28:{
                    ERROR(sizeof(CAR_TIME_AREA_QUERY));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    CAR_TIME_AREA_QUERY carTimeAreaQuery;
                    memcpy(&carTimeAreaQuery, businessData, sizeof(CAR_TIME_AREA_QUERY));
                    // 初始化变量
                    const int bufsize = sizeof(CAR_QUERY_BACK)+2;
                    char tempbuf[bufsize] = {0};
                    CAR_QUERY_BACK carQueryBack;
                    memset(&carQueryBack,0,sizeof(CAR_QUERY_BACK));
                    // 根据时间区域查询
                    Car car(SQL_NAME);
                    std::vector<CAR_QUERY> carQueryBackVector = car.queryCarInfoByUserIdAndTimeRange(carTimeAreaQuery.user_id,carTimeAreaQuery.entry_time,carTimeAreaQuery.out_time,carTimeAreaQuery.page);
                    for(int i = 0; i < 7; ++i){
                        strcpy(carQueryBack.carquery[i].car_number, carQueryBackVector[i].car_number);
                        strcpy(carQueryBack.carquery[i].entry_time, carQueryBackVector[i].entry_time);
                        strcpy(carQueryBack.carquery[i].out_time, carQueryBackVector[i].out_time);
                        strcpy(carQueryBack.carquery[i].entry_picture_path, carQueryBackVector[i].entry_picture_path);
                        strcpy(carQueryBack.carquery[i].out_picture_path, carQueryBackVector[i].out_picture_path);
                        carQueryBack.carquery[i].fee = carQueryBackVector[i].fee;
                    }
                    memcpy(tempbuf,&carQueryBack,sizeof(CAR_QUERY_BACK));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(CAR_QUERY_BACK));
                    // 写包头
                    head.businessLength = sizeof(CAR_QUERY_BACK);
                    head.businessType = 30;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&carQueryBack,sizeof(CAR_QUERY_BACK));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                case 29:{
                    ERROR(sizeof(CAR_TIME_AREA_CAR_NUMBER_QUERY));  // 检查包体大小
                    memset(buf,0,sizeof(SHM_SIZE));    
                    memset(&head,0,sizeof(head));
                    CAR_TIME_AREA_CAR_NUMBER_QUERY carTimeAreaCarNumberQuery;
                    memcpy(&carTimeAreaCarNumberQuery, businessData, sizeof(CAR_TIME_AREA_CAR_NUMBER_QUERY));
                    // 初始化变量
                    const int bufsize = sizeof(CAR_QUERY_BACK)+2;
                    char tempbuf[bufsize] = {0};
                    CAR_QUERY_BACK carQueryBack;
                    memset(&carQueryBack,0,sizeof(CAR_QUERY_BACK));
                    // 根据车牌号+时间区域查询
                    Car car(SQL_NAME);
                    std::vector<CAR_QUERY> carQueryBackVector = car.queryCarInfoByUserIdPlateAndTimeRange(carTimeAreaCarNumberQuery.user_id,carTimeAreaCarNumberQuery.carNumber,carTimeAreaCarNumberQuery.entry_time,carTimeAreaCarNumberQuery.out_time,carTimeAreaCarNumberQuery.page);
                    for(int i = 0; i < 7; ++i){
                        strcpy(carQueryBack.carquery[i].car_number, carQueryBackVector[i].car_number);
                        strcpy(carQueryBack.carquery[i].entry_time, carQueryBackVector[i].entry_time);
                        strcpy(carQueryBack.carquery[i].out_time, carQueryBackVector[i].out_time);
                        strcpy(carQueryBack.carquery[i].entry_picture_path, carQueryBackVector[i].entry_picture_path);
                        strcpy(carQueryBack.carquery[i].out_picture_path, carQueryBackVector[i].out_picture_path);
                        carQueryBack.carquery[i].fee = carQueryBackVector[i].fee;
                    }
                    memcpy(tempbuf,&carQueryBack,sizeof(CAR_QUERY_BACK));
                    //计算crc校验码
                    head.crc = CRC::getinterface()->CRC32(tempbuf,sizeof(CAR_QUERY_BACK));
                    // 写包头
                    head.businessLength = sizeof(CAR_QUERY_BACK);
                    head.businessType = 30;
                    // 拼接包进行发送
                    memcpy(buf,&head,sizeof(HEAD));
                    memcpy(buf+sizeof(HEAD),&carQueryBack,sizeof(CAR_QUERY_BACK));
                    memoryPool.writeData(buf,clientId,SERVER_BACK);
                    break;
                }
                default:
                    if(head.businessType == 0) {
                        std::cout << "包体有问题" << std::endl;
                        // 添加原始数据输出
                        std::cout << "原始包头数据：";
                        for(size_t i=0; i<sizeof(HEAD); ++i){
                            printf("%02X ", (unsigned char)buf[i]);
                        }
                        std::cout << std::endl;
                        continue;
                    }
            }
            std::cout << "发送包头的数据为：" << std::endl;
            std::cout << "head businessType:" << head.businessType << std::endl;
            std::cout << "head businessLength:" << head.businessLength << std::endl;
            std::cout << "接收数据总长度：" << sizeof(HEAD) + head.businessLength << "字节" << std::endl;
            std::cout << "CRC校验码: " << head.crc << std::endl;
            std::cout << "-------------------------" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            break;
        }
    }
    
    return 0;
}

std::string generateCaptcha() {
    // 设置随机数种子
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 产生4位随机数当验证码
    std::string captcha = "";
    for (int i = 0; i < 4; i++) {
        captcha += std::to_string(std::rand() % 10);
    }
    return captcha;
}

std::string formatDuration(int durationSeconds) {
    // 这里是函数的具体实现
    int hours = durationSeconds / 3600;
    int minutes = (durationSeconds % 3600) / 60;
    int seconds = durationSeconds % 60;

    std::string result = std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    return result;
}

// 解析时间字符串
std::tm parseTime(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        throw std::runtime_error("时间解析失败: " + timeStr);
    }
    return tm;
}

// 计算停车时间（单位：秒）
double calculateParkingDuration(const std::string& entryTimeStr, const std::string& exitTimeStr) {
    std::tm entryTm = parseTime(entryTimeStr);
    std::tm exitTm = parseTime(exitTimeStr);

    time_t entryTime = std::mktime(&entryTm);
    time_t exitTime = std::mktime(&exitTm);

    return std::difftime(exitTime, entryTime);
}

// 计算停车费用
int calculateParkingFee(double durationSeconds) {
    const double ratePerHour = 5.0;  // 每小时费用
    double hours = durationSeconds / 3600.0;  // 将秒转换为小时
    int fee = static_cast<int>(std::ceil(hours)) * ratePerHour;  // 向上取整并计算费用
    return fee;
}