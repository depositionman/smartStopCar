#include "CarIn.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Car.h"
#include <string>

CarIn::CarIn(std::string databaseName) : MySql(databaseName), databaseName(databaseName) {}

void CarIn::addCarIn(std::string carNumber, std::string entryTime, std::string entryLocation, std::string platePath, int userid) {
    // 1. 检查车辆是否已存在
    Car car(databaseName);
    if (!car.findCarByPlate(carNumber)) {
        // 如果车辆不存在，插入到 car 表
        car.addCar(carNumber, 0, 1, userid);  // VIPStatus = 0, parkStatus = 1 (已入场)
    } else {
        // 如果车辆已存在，更新停车状态
        car.updateParkStatus(carNumber, 1);  // parkStatus = 1 (已入场)
    }

    // 2. 插入到 car_in 表
    std::ostringstream oss;
    oss << "INSERT INTO car_in (carNumber, entryTime, entryLocation, platePath) VALUES ('"
        << carNumber << "', '" << entryTime << "', '" << entryLocation << "', '" << platePath << "')";
    executeUpdate(oss.str());
}

void CarIn::deleteCarInById(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM car_in WHERE id=" << id;
    executeUpdate(oss.str());
}

bool CarIn::findCarInById(int id) {
    std::ostringstream oss;
    oss << "SELECT * FROM car_in WHERE id=" << id;
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
}

std::vector<INBOUND> CarIn::getLatestInboundInfo(int maxCount) {
    std::ostringstream oss;
    oss << "SELECT id, carNumber, entryTime, entryLocation FROM car_in ORDER BY entryTime DESC LIMIT " << maxCount;
    std::vector<INBOUND> inboundList;
    executeQuery(oss.str(), [&inboundList](sql::ResultSet* res) {
        while (res->next()) {
            INBOUND inbound;
            inbound.id = res->getInt("id");
            strcpy(inbound.plate, res->getString("carNumber").c_str());
            strcpy(inbound.entryTime, res->getString("entryTime").c_str());
            strcpy(inbound.location, res->getString("entryLocation").c_str());
            inboundList.push_back(inbound);
        }
    });
    return inboundList;
}

bool CarIn::findCarInByPlate(std::string carNumber) {
    std::ostringstream oss;
    oss << "SELECT * FROM car_in WHERE carNumber=" << "'" << carNumber << "'";
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
}

std::string CarIn::queryCarEntryTimeByPlate(std::string carNumber){
    std::ostringstream oss;
    oss << "SELECT entryTime FROM car_in WHERE carNumber=" << "'" << carNumber << "'";
    std::string entryTime;
    executeQuery(oss.str(), [&entryTime](sql::ResultSet* res) {
        if (res->next()) {
            entryTime = res->getString("entryTime");
        }
    });
    return entryTime;
}