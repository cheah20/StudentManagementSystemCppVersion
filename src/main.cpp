#include "Admin.h"
#include "Lecturer.h"
#include "Student.h"

int main() {
    DBConnection db;
    sql::Connection* conn = db.get();
    if (!conn) {
        cout << "Please start MySQL in XAMPP and import database/schema.sql first." << endl;
        Utils::pause();
        return 1;
    }

    while (true) {
        Utils::header("Login");
        cout << "(type 0 as username to exit)\n" << endl;

        string username = Utils::readLine("Username : ");
        if (username == "0") break;
        string password = Utils::readLine("Password : ");

        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT role FROM users WHERE username = ? AND password = ?"));
            pstmt->setString(1, username);
            pstmt->setString(2, password);
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (!res->next()) {
                cout << "\n[ERROR] Wrong username or password." << endl;
                Utils::pause();
                continue;
            }

            string role = res->getString("role");
            if (role != "ADMIN") username = Utils::upper(username);
            cout << "\nLogin Successful! Welcome " << role << ": " << username << endl;
            Utils::pause();

            // each role only gets its own module
            if (role == "ADMIN") AdminModule(conn).menu();
            else if (role == "LECTURER") LecturerModule(username, conn).menu();
            else if (role == "STUDENT") StudentModule(username, conn).menu();
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
            Utils::pause();
        }
    }

    cout << "\nThank you for using the Student Management System!" << endl;
    return 0;
}
