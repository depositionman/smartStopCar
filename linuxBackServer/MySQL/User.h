#ifndef __USER_H
#define __USER_H

#include "mySql.h"
#include <string>

class User : public MySql {
public:
    User(std::string databaseName);
    ~User(){}
    void addUser(std::string userName, std::string userPassword, std::string phoneNumber, std::string captcha); //添加用户
    void updateUserOnName(std::string userName);                        // 修改用户名
    void updateUserOnPhoneNumber(std::string phoneNumber);              // 修改用户电话
    void updateUserOnPassword(std::string userPassword);                // 修改用户密码
    void updateUserOnCaptcha(std::string captcha);                      // 修改用户验证码
    void deleteUserOnName(std::string userName);                        // 删除用户名
    void deleteUserOnPhoneNumber(std::string phoneNumber);              // 删除用户电话
    bool findUserOnName(std::string userName);                          // 查找用户名
    bool findUserOnPhoneNumber(std::string phoneNumber);                // 查找用户电话
    std::string findUserOnPwdByPhoneNumber(std::string phoneNumber);    // 通过电话查找对应的密码
private:
    std::string phoneNumber;  // 添加 phoneNumber 成员变量
};

#endif 