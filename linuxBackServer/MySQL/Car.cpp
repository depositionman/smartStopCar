#include "Car.h"
#include <iostream>
#include <sstream>
#include <string>
#include "CarIn.h"
#include "CarOut.h"

Car::Car(std::string databaseName) : MySql(databaseName) {}

void Car::addCar(std::string carNumber, int VIPStatus, int parkStatus, int userid) {
    std::ostringstream oss;
    oss << "INSERT INTO car (carNumber, VIPStatus, parkStatus, userid) VALUES ('"
    << carNumber << "', " << VIPStatus << ", " << parkStatus << ", " << userid << ")";
    executeUpdate(oss.str());
}

void Car::deleteCarById(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM car WHERE id=" << id;
    executeUpdate(oss.str());
}

bool Car::findCarById(int id) {
    std::ostringstream oss;
    oss << "SELECT * FROM car WHERE id=" << id;
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
}

int Car::getParkStatus(std::string carNumber) {
    std::ostringstream oss;
    oss << "SELECT parkStatus FROM car WHERE carNumber='" << carNumber << "'";
    int parkStatus = 0;
    executeQuery(oss.str(), [&parkStatus](sql::ResultSet* res) {
        if (res->next()) {
            parkStatus = res->getInt("parkStatus");
        }
    });
    return parkStatus;
}

bool Car::findCarByPlate(std::string carNumber) {
    std::ostringstream oss;
    oss << "SELECT * FROM car WHERE carNumber='" << carNumber << "'";
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
}

void Car::updateParkStatus(std::string carNumber, int parkStatus) {
    std::ostringstream oss;
    oss << "UPDATE car SET parkStatus=" << parkStatus << " WHERE carNumber='" << carNumber << "'";
    executeUpdate(oss.str());
}

void Car::updateCarPlate(std::string oldPlate, std::string newPlate) {
    // 更新 car 表
    std::ostringstream ossCar;
    ossCar << "UPDATE car SET carNumber='" << newPlate << "' WHERE carNumber='" << oldPlate << "'";
    executeUpdate(ossCar.str());

    // 更新 car_in 表
    std::ostringstream ossCarIn;
    ossCarIn << "UPDATE car_in SET carNumber='" << newPlate << "' WHERE carNumber='" << oldPlate << "'";
    executeUpdate(ossCarIn.str());

    // 更新 car_out 表
    std::ostringstream ossCarOut;
    ossCarOut << "UPDATE car_out SET carNumber='" << newPlate << "' WHERE carNumber='" << oldPlate << "'";
    executeUpdate(ossCarOut.str());
} 

std::vector<CAR_QUERY> Car::queryCarInfoByUserIdAndPlate(int userId, std::string carNumber, int page, int limit) {
    std::vector<CAR_QUERY> carQueries;
    int offset = (page - 1) * limit;
    limit = std::min(limit, 7); // 最多查询 7 条数据

    std::ostringstream oss;
    oss << "SELECT ci.carNumber, ci.entryTime, co.comeOutTime, ci.platePath AS entryPlatePath, co.platePath AS outPlatePath, co.comeOutFee "
        << "FROM car_in ci "
        << "LEFT JOIN car_out co ON ci.carNumber = co.carNumber "
        << "JOIN car c ON ci.carNumber = c.carNumber "
        << "WHERE c.userid = " << userId << " AND ci.carNumber = '" << carNumber << "' "
        << "LIMIT " << limit << " OFFSET " << offset;

    executeQuery(oss.str(), [&carQueries](sql::ResultSet* res) {
        int count = 0;
        while (res->next() && count < 7) {
             // 打印原始数据
        std::cout << "DEBUG - carNumber: " << res->getString("carNumber") 
                  << ", entryTime: " << res->getString("entryTime")
                  << ", comeOutFee: " << res->getInt("comeOutFee") << std::endl;
            CAR_QUERY carQuery;
            memset(&carQuery, 0, sizeof(CAR_QUERY));

            strcpy(carQuery.car_number, res->getString("carNumber").c_str());

            strcpy(carQuery.entry_time, res->getString("entryTime").c_str());

            if (!res->isNull("comeOutTime")) {
                strcpy(carQuery.out_time, res->getString("comeOutTime").c_str());
            } else {
                carQuery.out_time[0] = '\0';
            }

            strcpy(carQuery.entry_picture_path, res->getString("entryPlatePath").c_str());

            strcpy(carQuery.out_picture_path, res->getString("outPlatePath").c_str());

            if (!res->isNull("comeOutFee")) {
                try {
                    carQuery.fee = std::stoi(res->getString("comeOutFee"));
                } catch (const std::invalid_argument& e) {
                    carQuery.fee = 0;
                } catch (const std::out_of_range& e) {
                    carQuery.fee = 0;
                }
            } else {
                carQuery.fee = 0;
            }

            carQueries.push_back(carQuery);
            ++count;
        }
    });

    return carQueries;
}

std::vector<CAR_QUERY> Car::queryCarInfoByUserIdAndTimeRange(int userId, std::string entryTime, std::string outTime, int page, int limit) {
    std::vector<CAR_QUERY> carQueries;
    int offset = (page - 1) * limit;

    std::ostringstream oss;
    oss << "SELECT ci.carNumber, ci.entryTime, co.comeOutTime, ci.platePath AS entryPlatePath, co.platePath AS outPlatePath, co.comeOutFee "
        << "FROM car_in ci "
        << "LEFT JOIN car_out co ON ci.carNumber = co.carNumber "
        << "JOIN car c ON ci.carNumber = c.carNumber "
        << "WHERE c.userid = " << userId << " "
        << "AND (ci.entryTime >= '" << entryTime << "' AND (co.comeOutTime IS NULL OR co.comeOutTime <= '" << outTime << "')) "
        << "LIMIT " << limit << " OFFSET " << offset;

    executeQuery(oss.str(), [&carQueries](sql::ResultSet* res) {
        while (res->next()) {
            CAR_QUERY carQuery;
            memset(&carQuery, 0, sizeof(CAR_QUERY));

            strcpy(carQuery.car_number, res->getString("carNumber").c_str());
            strcpy(carQuery.entry_time, res->getString("entryTime").c_str());
            strcpy(carQuery.out_time, res->getString("comeOutTime").c_str());
            strcpy(carQuery.entry_picture_path, res->getString("entryPlatePath").c_str());
            strcpy(carQuery.out_picture_path, res->getString("outPlatePath").c_str());
            carQuery.fee = std::stoi(res->getString("comeOutFee"));

            carQueries.push_back(carQuery);
        }
    });

    return carQueries;
}

std::vector<CAR_QUERY> Car::queryCarInfoByUserIdPlateAndTimeRange(int userId, std::string carNumber, std::string entryTime, std::string outTime, int page, int limit) {
    std::vector<CAR_QUERY> carQueries;
    int offset = (page - 1) * limit;

    std::ostringstream oss;
    oss << "SELECT ci.carNumber, ci.entryTime, co.comeOutTime, ci.platePath AS entryPlatePath, co.platePath AS outPlatePath, co.comeOutFee "
        << "FROM car_in ci "
        << "LEFT JOIN car_out co ON ci.carNumber = co.carNumber "
        << "JOIN car c ON ci.carNumber = c.carNumber "
        << "WHERE c.userid = " << userId << " "
        << "AND ci.carNumber = '" << carNumber << "' "
        << "AND (ci.entryTime >= '" << entryTime << "' AND (co.comeOutTime IS NULL OR co.comeOutTime <= '" << outTime << "')) "
        << "LIMIT " << limit << " OFFSET " << offset;

    executeQuery(oss.str(), [&carQueries](sql::ResultSet* res) {
        while (res->next()) {
            CAR_QUERY carQuery;
            memset(&carQuery, 0, sizeof(CAR_QUERY));

            strcpy(carQuery.car_number, res->getString("carNumber").c_str());
            strcpy(carQuery.entry_time, res->getString("entryTime").c_str());
            strcpy(carQuery.out_time, res->getString("comeOutTime").c_str());
            strcpy(carQuery.entry_picture_path, res->getString("entryPlatePath").c_str());
            strcpy(carQuery.out_picture_path, res->getString("outPlatePath").c_str());
            carQuery.fee = std::stoi(res->getString("comeOutFee"));

            carQueries.push_back(carQuery);
        }
    });

    return carQueries;
}