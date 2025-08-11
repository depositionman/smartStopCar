#ifndef PROTOCOL_H
#define PROTOCOL_H

enum BusinessType{
    LOGINREQUEST = 1,   //登录请求
    CHATREQUEST,    //聊天请求
    LOGINBACK,      //登录返回
    CHATBACK        //聊天返回
};

enum LoginFlag{
    SUCCESS = 1,
    FAIL,
    NOTEXIST
};

//协议头
typedef struct head {
    enum BusinessType businessType;		//业务类型
    int businessLength;		//业务长度
}HEAD;
//登录请求包体
typedef struct user {
    int userID;
    char password[20];
}USER;
//登录返回包体
typedef struct backmsg {
    enum LoginFlag flag;			//1成功 2失败 3查无此账号
    char message[20];
}BACKMSG;
//聊天业务请求包体（返回包体）
typedef struct chatMsg {
    int sendID;			//发送者id
    int rcvID;			//接收者id
    char context[150];	//发的信息
}CHATMSG;

#endif // PROTOCOL_H
