#include "ParkInfo.h"
#include <iostream>
#include <sstream>
#include <string>

int ParkInfo::occupancySpace = 0;
int ParkInfo::remainSpace = 500;


ParkInfo::ParkInfo(std::string databaseName) : MySql(databaseName) {}

void ParkInfo::updateParkInfo() {
    std::ostringstream oss;
    ++occupancySpace;
    remainSpace = totalSpace - occupancySpace;
    oss << "UPDATE park_info SET totalSpace=" << totalSpace << ", occupancySpace=" << occupancySpace << ", remainSpace=" << remainSpace;
    executeUpdate(oss.str());
}

int ParkInfo::getRemainSpace() {
    std::ostringstream oss;
    oss << "SELECT remainSpace FROM park_info";
    executeQuery(oss.str(), [this](sql::ResultSet* res) {
        if (res->next()) {
            remainSpace = res->getInt("remainSpace");
        }
    });
    return remainSpace;
} 

int ParkInfo::getTotalSpace()
{
    return this->totalSpace;
}