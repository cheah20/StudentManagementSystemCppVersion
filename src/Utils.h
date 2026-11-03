#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <io.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include "DBConnection.h"
using namespace std;

namespace Utils {
    inline void header(const string& title) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        cout << "==================================================\n";
        cout << "  STUDENT MANAGEMENT SYSTEM - " << title << "\n";
        cout << "==================================================\n";
    }

    // colours only show in a real console, not in piped output
    inline bool useColor() {
#ifdef _WIN32
        static bool ok = [] {
            HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode = 0;
            return _isatty(_fileno(stdout)) && GetConsoleMode(out, &mode) &&
                SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }();
#else
        static bool ok = isatty(STDOUT_FILENO);
#endif
        return ok;
    }

    inline string paint(const string& code) { return useColor() ? "\033[" + code + "m" : ""; }
    inline string reset() { return paint("0"); }

    // dean's bright cyan bold, pass cyan, near fail yellow, fail red
    inline string gradeColor(double gpa) {
        if (gpa >= 3.70) return paint("1;96");
        if (gpa >= 2.00) return paint("36");
        if (gpa >= 1.00) return paint("33");
        return paint("31");
    }

    inline string cgpaColor(double cgpa) {
        if (cgpa >= 3.50) return paint("1;96");
        if (cgpa >= 2.50) return paint("36");
        if (cgpa >= 2.00) return paint("33");
        return paint("31");
    }

    inline string readLine(const string& prompt) {
        cout << prompt;
        string s;
        if (!getline(cin, s)) exit(0);
        return s;
    }

    // shows * for each character
    inline string readPassword(const string& prompt) {
#ifdef _WIN32
        if (!_isatty(_fileno(stdin))) return readLine(prompt);
        cout << prompt;
        string pass;
        int ch;
        while ((ch = _getch()) != '\r') {
            if (ch == 0 || ch == 224) _getch();
            else if (ch == '\b') {
                if (!pass.empty()) { pass.pop_back(); cout << "\b \b"; }
            }
            else { pass += (char)ch; cout << '*'; }
        }
#else
        termios old;
        if (tcgetattr(STDIN_FILENO, &old) != 0) return readLine(prompt);
        cout << prompt << flush;
        termios raw = old;
        raw.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        string pass;
        char ch;
        while (read(STDIN_FILENO, &ch, 1) == 1 && ch != '\n') {
            if (ch == 127 || ch == '\b') {
                if (!pass.empty()) { pass.pop_back(); cout << "\b \b" << flush; }
            }
            else { pass += ch; cout << '*' << flush; }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
        cout << endl;
        return pass;
    }

    inline void pause() {
        readLine("\nPress Enter to continue...");
    }

    inline int readInt(const string& prompt) {
        while (true) {
            string s = readLine(prompt);
            try {
                size_t used;
                int value = stoi(s, &used);
                if (used == s.length()) return value;
            }
            catch (...) {}
            cout << "Please enter a number.\n";
        }
    }

    // marks must be 0 - 100
    inline double readMark(const string& prompt) {
        while (true) {
            string s = readLine(prompt);
            try {
                size_t used;
                double mark = stod(s, &used);
                if (used == s.length() && mark >= 0 && mark <= 100) return mark;
            }
            catch (...) {}
            cout << "Mark must be between 0 and 100.\n";
        }
    }

    inline string upper(string s) {
        transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    inline bool confirm(const string& prompt) {
        string ans = readLine(prompt + " (Y/N): ");
        return ans == "Y" || ans == "y";
    }

    // recount all cgpa after a course changes
    inline void refreshAllCgpa(sql::Connection* conn) {
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        stmt->execute(
            "UPDATE students s LEFT JOIN ("
            "SELECT e.matric_number, SUM(e.gpa * c.credit_hours) / SUM(c.credit_hours) AS cgpa "
            "FROM enrollments e JOIN courses c ON e.course_code = c.course_code "
            "WHERE e.grade IS NOT NULL GROUP BY e.matric_number"
            ") t ON s.matric_number = t.matric_number "
            "SET s.cgpa = COALESCE(t.cgpa, 0), s.gpa = COALESCE(t.cgpa, 0)");
    }

    // same password is kept in users and in the role table
    inline void changePassword(sql::Connection* conn, const string& username, const string& table, const string& key) {
        header("Change Password");
        try {
            unique_ptr<sql::PreparedStatement> check(conn->prepareStatement(
                "SELECT COUNT(*) AS cnt FROM users WHERE username = ? AND password = ?"));
            check->setString(1, username);
            check->setString(2, readPassword("Current password : "));
            unique_ptr<sql::ResultSet> res(check->executeQuery());

            if (!res->next() || res->getInt("cnt") == 0) {
                cout << "\n[ERROR] Current password is wrong." << endl;
                pause();
                return;
            }

            string newPass = readPassword("New password     : ");
            if (newPass.length() < 6) {
                cout << "\n[ERROR] Password needs at least 6 characters." << endl;
                pause();
                return;
            }
            if (readPassword("Confirm password : ") != newPass) {
                cout << "\n[ERROR] Passwords do not match." << endl;
                pause();
                return;
            }

            unique_ptr<sql::PreparedStatement> user(conn->prepareStatement("UPDATE users SET password = ? WHERE username = ?"));
            user->setString(1, newPass);
            user->setString(2, username);
            user->executeUpdate();

            unique_ptr<sql::PreparedStatement> role(conn->prepareStatement(
                "UPDATE " + table + " SET password = ? WHERE " + key + " = ?"));
            role->setString(1, newPass);
            role->setString(2, username);
            role->executeUpdate();

            cout << "\n[SUCCESS] Password changed." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        pause();
    }
}

#endif
