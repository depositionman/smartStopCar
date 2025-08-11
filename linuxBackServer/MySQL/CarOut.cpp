#include "CarOut.h"
#include <iostream>
#include <sstream>
#include <string>
#include "Car.h"

CarOut::CarOut(std::string databaseName) : MySql(databaseName), databaseName(databaseName) {}

void CarOut::addCarOut(std::string carNumber, std::string comeOutTime, std::string comeOutLocation, std::string comeOutFee, std::string parkTime, std::string platePath, int userid) {
    // 1. 检查车辆是否已存在
    Car car(databaseName);
    if (car.findCarByPlate(carNumber)) {
        // 如果车辆存在，更新停车状态
        car.updateParkStatus(carNumber, 0);  // parkStatus = 0 (已出场)
        std::cout << "车辆出场了" << std::endl;
    }

    // 2. 插入到 car_out 表
    std::ostringstream oss;
    oss << "INSERT INTO car_out (carNumber, comeOutTime, comeOutLocation, comeOutFee, parkTime, platePath) VALUES ('"
        << carNumber << "', '" << comeOutTime << "', '" << comeOutLocation << "', '" << comeOutFee << "', '" << parkTime << "', '" << platePath << "')";
    executeUpdate(oss.str());
}

void CarOut::deleteCarOutById(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM car_out WHERE id=" << id;
    executeUpdate(oss.str());
}

bool CarOut::findCarOutById(int id) {
    std::ostringstream oss;
    oss << "SELECT * FROM car_out WHERE id=" << id;
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
} 