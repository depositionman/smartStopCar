#ifndef __MYSQL_H
#define __MYSQL_H

#include <iostream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <string>
#include <functional>

#define USER    "root"
#define PWD     "pASSWORD_123"

class MySql
{
public:
    MySql(std::string databaseName);
    virtual ~MySql();

    void executeUpdate(std::string sql);
    void executeQuery(std::string sql, std::function<void(sql::ResultSet*)> callback);

protected:
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;
};

#endif