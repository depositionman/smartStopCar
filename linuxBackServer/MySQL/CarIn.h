#ifndef __CARIN_H
#define __CARIN_H

#include "mySql.h"
#include <string>
#include <vector>  // 添加 vector 支持
#include "../agreement/agreement.h"
#include <string.h>

class CarIn : public MySql {
public:
    CarIn(std::string databaseName);
    ~CarIn() {}
    void addCarIn(std::string carNumber, std::string entryTime, std::string entryLocation, std::string platePath, int userid); // 添加车辆入场记录
    void deleteCarInById(int id); // 根据ID删除车辆入场记录
    bool findCarInById(int id); // 根据ID查找车辆入场记录
    bool findCarInByPlate(std::string carNumber);// 根据车牌号查找车辆入场记录
    std::string queryCarEntryTimeByPlate(std::string carNumber);// 根据车牌号查找车辆进场时间
    std::vector<INBOUND> getLatestInboundInfo(int maxCount = 4); // 获取最新的入场信息，最多返回 maxCount 条
private:
    std::string databaseName;
};

#endif 