#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <iostream>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/datatype.h>
using namespace std;

// xampp default login: root, no password
class DBConnection {
private:
    sql::mysql::MySQL_Driver* driver = nullptr;
    unique_ptr<sql::Connection> conn;

public:
    DBConnection() {
        try {
            driver = sql::mysql::get_mysql_driver_instance();
            conn = unique_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", "root", ""));
            conn->setSchema("student_managment_system");
        }
        catch (sql::SQLException& e) {
            cout << "Database Connection Failed: " << e.what() << endl;
            conn = nullptr;
        }
    }

    sql::Connection* get() { return conn.get(); }
};

#endif
