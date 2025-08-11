#ifndef __CAROUT_H
#define __CAROUT_H

#include "mySql.h"
#include <string>

class CarOut : public MySql {
public:
    CarOut(std::string databaseName);
    ~CarOut() {}
    void addCarOut(std::string carNumber, std::string comeOutTime, std::string comeOutLocation, std::string comeOutFee, std::string parkTime, std::string platePath, int userid); // 添加车辆出场记录
    void deleteCarOutById(int id); // 根据ID删除车辆出场记录
    bool findCarOutById(int id); // 根据ID查找车辆出场记录
private:
    std::string databaseName;
};

#endif 