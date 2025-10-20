#include "User.h"
#include <iostream>
#include <sstream>
#include <string>

User::User(std::string databaseName) : MySql(databaseName) {}

void User::addUser(std::string userName, std::string userPassword, std::string phoneNumber, std::string captcha){
    std::cout << "Inserting password: " << userPassword << std::endl;  // 打印密码
    std::ostringstream oss;
    oss << "INSERT INTO user (name, password, phone, captcha) VALUES ('" << userName << "', '" << userPassword << "', '" << phoneNumber << "', '" << captcha << "')";
    executeUpdate(oss.str());
}

void User::updateUserOnName(std::string userName){
    std::ostringstream oss;
    oss << "UPDATE user SET name='" << userName << "' WHERE phone='" << phoneNumber << "'";
    executeUpdate(oss.str());
}

void User::updateUserOnPhoneNumber(std::string phoneNumber){
    std::ostringstream oss;
    oss << "UPDATE user SET phone='" << phoneNumber << "' WHERE phone='" << phoneNumber << "'";
    executeUpdate(oss.str());
}

void User::updateUserOnPassword(std::string userPassword){
    std::ostringstream oss;
    oss << "UPDATE user SET password='" << userPassword << "' WHERE phone='" << phoneNumber << "'";
    executeUpdate(oss.str());
}

void User::updateUserOnCaptcha(std::string captcha){
    std::ostringstream oss;
    oss << "UPDATE user SET captcha='" << captcha << "' WHERE phone='" << phoneNumber << "'";
    executeUpdate(oss.str());
}

void User::deleteUserOnName(std::string userName){
    std::ostringstream oss;
    oss << "DELETE FROM user WHERE name='" << userName << "'";
    executeUpdate(oss.str());
}

bool User::findUserOnName(std::string userName){
    std::ostringstream oss;
    oss << "SELECT * FROM user WHERE name='" << userName << "'";
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
}

bool User::findUserOnPhoneNumber(std::string phoneNumber){
    std::ostringstream oss;
    oss << "SELECT * FROM user WHERE phone='" << phoneNumber << "'";
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
}

std::string User::findUserOnPwdByPhoneNumber(std::string phoneNumber) {
    std::ostringstream oss;
    oss << "SELECT password FROM user WHERE phone='" << phoneNumber << "'";
    std::string password = "";
    executeQuery(oss.str(), [&password](sql::ResultSet* res) {
        if (res->next()) {  // 如果查询到结果
            password = res->getString("password");  // 获取密码字段
        }
    });
    return password;
}