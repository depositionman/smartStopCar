#ifndef AGREEMENT_H
#define AGREEMENT_H
#include <stdint.h>
/*
 * 1.获取验证码          2.返回验证码
 * 3.用户注册发送        4.用户注册返回
 * 5.用户登录发送        6.用户登陆返回
 * 7 文件协议发送包      8 文件协议返回包
 * 9 入场发送包         10 入场返回包
 * 11出场请求包体        12.出场返回包
 * 13 视频上传发送包     14 视频上传返回包
 * 15 修改车牌请求包	   16 修改车牌返回包
 * 17 视频查询请求包	   18 视频查询返回包
 * 19 视频查询下一页请求包（包括点击）
 * 20 视频查询下一页返回包（包括点击）
 * 21 视频播放请求包      22 视频播放返回包
 * 23 视频播放结束请求包	24视频播放结束返回包
 * 25 停车场请求包        26停车场请求返回包
 * 27 车辆查询信息请求包（车牌号查询）
 * 28 车辆查询信息请求包（时间区间查询）
 * 29 车辆查询请求包（车牌号+时间区间查询）
 * 30 车辆信息查询返回包
 * 31 心跳包
 * 32 断线重连服务器发送包
 * 33 断线重连客户端发送包
 * 98结束包
 * 99crc检验错误返回类型
*/

/****************非协议包结构体******************/
//剩余车位结构体
typedef struct RemainCar{
    int remainspace;    //剩余空间
    int id;             //当前的用户id
}REMAINCAR;

//入库车辆结构体
typedef struct InboundVehicles{
    int id;             //序号
    char plate[20];     //车牌
    char entryTime[25]; //入场时间
    char location[20];  //入场位置
}INBOUND;

//图片传输结构体
typedef struct fileInfo
{
    char buf[1000];     //文件内容
    char userid[20];    //文件所属用户
    char fileName[30];  //文件名
    int fileindex;      //零碎文件序号
    int num;            //零碎文件总个数
    int length;         //零碎文件长度(字节数)
    int fileLength;     //源文件总长度(字节数)
}FILEINFO;

//存储视频查询左侧日期结构体
typedef struct videoList_Date {
    char DateTime[12];
}VIDEO_LIST_DATE;

//视频查询 视频名字+封面路径结构体
typedef struct VideoItem {
    char videoName[30];
    char coverPath[75];
}VIDEO_ITEM;

//车辆信息查询结构体
typedef struct CarQuery {
    char car_number[30]; //车牌号
    char entry_time[50]; //入场时间
    char out_time[50]; //出场时间
    char entry_picture_path[100]; //入场图片
    char out_picture_path[100]; //出场图片
    int fee; //停车费用
}CAR_QUERY;
/******************协议包***********************/
// 协议头
typedef struct Head {
    int businessType;    
    int businessLength; 
    uint32_t crc;      
} HEAD;

//获取验证码请求包-------1
typedef struct getCaptcha {
    char PhoneNumber[12];//手机号
}GETCAPTCHA;

//返回验证码包-----------2
typedef struct BackCaptcha {
    int flag;                   //1成功 2失败
}BACKCAPTCHA;

//用户注册发送包----------3
typedef struct UserSignIn {
    char userName[20];			//用户名
    char userPassword[35];		//用户密码
    char PhoneNumber[12];       //手机号码
    char captcha[7];           //验证码
}SIGN;

//用户注册返回包----------4
typedef struct backSign {
    int flag;               //1成功2失败
    char captcha[30];       //返回语句
}BACKSIGN;

//用户登录发送-----------5
typedef struct UserLogin{
    char phoneNum[12];      //手机号码
    char userPassword[35];  //用户密码
}LOGIN;

//用户登录返回-----------6
typedef struct BackLogin{
    int flag;               //1.成功 2.失败 3.未注册
    REMAINCAR remaincar;    //剩余车位结构体
    INBOUND inbound[4];     //入库车辆结构体数组
}BACKLOGIN;

//文件协议发送包---7

//文件协议返回包---8
typedef struct lostPackage {
    int lostId[1000];//lostId[0] = -1表明文件传输过去正确
}LOSTPACKAGE;

//入库请求包------------9
typedef struct store
{
    int user_id;          //用户id
    char plate[20];       //车牌
    char time[25];        //时间
    char location[20];    //入场位置
    char path[100];       //路径
}STORE;

//入库返回包------------10
typedef struct backstore
{
    int flag;//1.成功 2.失败
    REMAINCAR reamincar;    //剩余车位结构体
    INBOUND inbound[4];     //入库车辆结构体
}BACKSTORE;

// 车辆出场请求包体 -------------11
typedef struct carOutRequest {
    int user_id;                //用户id
    char carNumber[20];          // 车牌号码
    char comeOutLocation[50];    // 出场位置
    char platePath[100];         // 车牌图片路径
    char time[20];              //出场时间
} CAR_OUT_REQUEST;

// 车辆出场返回包体 -----------12
typedef struct carOutResponse {
    int flag;//1成功 2失败
    char Parktime[50];        //停车时间
    int money;//停车费用
    char Storetime[25]; //时间
    REMAINCAR reamincar;    //剩余车位结构体
} CAR_OUT_RESPONSE;

// 视频上传请求包体-----------13
typedef struct videoUpload {
    int userId;                  // 用户 ID
    char videoFilename[100];     // 视频文件名
    char videoPath[100];         // 视频路径
    char coverPath[100];         // 封面路径
    int videoTotalFrame;         // 视频总帧数
    char time[25];              //创建时间
} VIDEO_UPLOAD;

// 视频上传返回包体 --------14
typedef struct videoUploadResponse {
    int flag;                    // 1: 成功, 2: 失败
} VIDEO_UPLOAD_RESPONSE;

//修改车牌请求包----------15
typedef struct changePlate
{
    int usr_id;            // 用户 ID
    char oldPlate[20];    //原车牌
    char newPlate[20];    //新车牌
}CHANGEPLATE;

//返回修改车牌返回包-------------16
typedef struct backChangePlate
{
    int flag;   //1.成功 2.失败
}BACKCHANGEPLATE;

//视频查询请求包---17
typedef struct videoQuery {
    int user_id;
    int type;//1按月 2按天
}VIDEOQUERY;

//视频查询返回包 ---18
typedef struct videoQueryBack {
    int flag;//1成功 0失败
    VIDEO_ITEM videoList[15];//15个视频的路径和名字
    VIDEO_LIST_DATE videoTime[31];//按日显示就返回三十个日期 按月就返回十二个月
}VIDEO_QUERY_BACK;

//视频查询下一页请求包（包括点击） ---19
typedef struct videoNextPage {
    int use_id;//用户id
    int type;//1按月 2按日
    int next_page;//下一页的页数
    char query_time[50];//查询日期
}VIDEO_NEXT_PAGE;

//视频查询下一页返回包（包括点击） ---20
typedef struct videoNextPageBack {
    int flag;//1成功 0失败
    VIDEO_ITEM videoList[15];//15个视频的路径和名字
}VIDEO_NEXT_PAGE_BACK;

//视频播放请求包    ---21
typedef struct videoPlay {
    int userid; //用户名
    char videoName[100];    //视频文件名
}VIDEO_PLAY;

//视频播放返回包 ---22
typedef struct BackVideoPlay {
    char videoFilename[100];     // 视频文件名
    char videoPath[100];         // 视频路径
    int videoTotalFrame;         // 视频总帧数
    int record_frame;           // 当前视频播放帧
}BACK_VIDEO_PLAY;

//视频播放结束请求包 ---23
typedef struct VideoPlayOver {
    int user_id;//用户id
    char videoName[100];    //视频文件名
    int record_frame;   //当前视频播放帧
}VIDEO_PLAY_OVER;

//视频播放结束返回包 ---24
typedef struct VideoPlayOverBack {
    int flag;//1成功 0失败
}VIDEO_PLAY_OVER_BACK;

//停车场请求包---25

//停车场请求返回包---26
typedef struct parkBack{
    int totalCarNum;    //总车辆
    int freeSpaceNum;   //空闲车位
}PARKBACK;

//车辆查询信息请求包（车牌号查询）---27
typedef struct CarNumberQuery {
    int user_id;        //用户id
    char carNumber[20];//车牌号
    int page;//页数
}CAR_NUMBER_QUERY;

//车辆查询信息请求包（时间区间查询）---28
typedef struct CarTimeAreaQuery {
    int user_id;         //用户id
    char entry_time[50]; //入场时间
    char out_time[50];	//出场时间
    int page;//页数
}CAR_TIME_AREA_QUERY;

//车辆查询请求包（车牌号+时间区间查询）---29
typedef struct CarTimeAreaCarNumberQuery {
    int user_id;                //用户id
    char carNumber[20];//车牌号
    char entry_time[50]; //入场时间
    char out_time[50];    //出场时间
    int page;//页数
}CAR_TIME_AREA_CAR_NUMBER_QUERY;

//车辆查询返回包 ---30
typedef struct CarQueryBack {
    CAR_QUERY carquery[7];
}CAR_QUERY_BACK;

//心跳包发送包 ---31
typedef struct HeartClient {
    char username[30];
}HEART_CLIENT;

//断线重连服务器发送包 ---32
typedef struct lostConnectBack {
    char fileName[50];
}LOSTCONNECTBACK;

//断线重连客户端发送包 --33
typedef struct usrChoice {
    int flag;       //1--用户选择需要重传 2--用户选择不重传
}USRCHOICE;

//结束包--------98
typedef struct endBag{
    char fileName[30];        //文件名
    char userid[20];          //用户id
}ENDBAG;

#endif // AGREEMENT_H
