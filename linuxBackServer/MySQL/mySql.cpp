#include "mySql.h"

MySql::MySql(std::string databaseName)
{
    try {
        // 初始化数据库连接
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", USER, PWD);
        con->setSchema(databaseName);
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL error: " << e.what() << std::endl;
        delete con;
    }
}

MySql::~MySql()
{
    // 清理资源
    delete con;
}

void MySql::executeUpdate(std::string sql)
{
    sql::Statement* stmt = nullptr;
    try {
        stmt = con->createStatement();
        stmt->execute(sql);
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL error: " << e.what() << std::endl;
    }
    delete stmt;
}

void MySql::executeQuery(std::string sql, std::function<void(sql::ResultSet*)> callback)
{
    sql::Statement* stmt = nullptr;
    sql::ResultSet* res = nullptr;
    try {
        stmt = con->createStatement();
        res = stmt->executeQuery(sql);
        callback(res);
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL error: " << e.what() << std::endl;
    }
    delete res;
    delete stmt;
}
