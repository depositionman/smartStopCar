#ifndef __CAR_H
#define __CAR_H

#include "mySql.h"
#include <string>
#include "../agreement/agreement.h"

//parkStatus = 1 (已入场) 0（已出场）
class Car : public MySql {
public:
    Car(std::string databaseName);
    ~Car() {}
    void addCar(std::string carNumber, int VIPStatus, int parkStatus, int userid); // 添加车辆
    void deleteCarById(int id); // 根据ID删除车辆
    bool findCarById(int id); // 根据ID查找车辆
    bool findCarByPlate(std::string carNumber); // 根据车牌号查找车辆
    int getParkStatus(std::string carNumber); // 根据车牌号获取停车状态
    void updateParkStatus(std::string carNumber, int parkStatus); // 更新停车状态
    void updateCarPlate(std::string oldPlate, std::string newPlate); // 更新车牌号
    // 根据用户 ID、车牌号分页查询车辆信息
    std::vector<CAR_QUERY> queryCarInfoByUserIdAndPlate(int userId, std::string carNumber, int page, int limit = 7);
    // 根据用户 ID、时间区间分页查询车辆信息
    std::vector<CAR_QUERY> queryCarInfoByUserIdAndTimeRange(int userId, std::string entryTime, std::string outTime, int page, int limit = 7);
    // 根据用户 ID、车牌号、入场时间和出场时间分页查询车辆信息
    std::vector<CAR_QUERY> queryCarInfoByUserIdPlateAndTimeRange(int userId, std::string carNumber, std::string entryTime, std::string outTime, int page, int limit = 7);
private:
    std::string databaseName;
};

#endif 