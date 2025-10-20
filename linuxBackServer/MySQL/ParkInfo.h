#ifndef __PARKINFO_H
#define __PARKINFO_H

#include "mySql.h"
#include <string>

class ParkInfo : public MySql {
public:
    ParkInfo(std::string databaseName);
    ~ParkInfo() {}
    void updateParkInfo(); // 更新停车信息
    int getRemainSpace(); // 获取剩余停车位
    int getTotalSpace();  // 获取总空间
private:
    std::string databaseName;
    int totalSpace = 500;               // 停车场总空间
    static int occupancySpace;      // 停车场已占用空间
    static int remainSpace;       // 停车场剩余空间
};

#endif 