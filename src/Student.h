#ifndef STUDENT_H
#define STUDENT_H

#include <fstream>
#include <ctime>
#include "Utils.h"

class StudentModule {
private:
    string matricNumber;
    sql::Connection* conn;

    string getCurrentDateTime() {
        time_t now = time(0);
        tm ltm;
#ifdef _WIN32
        localtime_s(&ltm, &now);
#else
        localtime_r(&now, &ltm);
#endif
        char buffer[100];
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &ltm);
        return string(buffer);
    }

    string classify(double cgpa) {
        if (cgpa >= 3.75) return "First Class Honours";
        if (cgpa >= 3.00) return "Second Class Honours (Upper Division)";
        if (cgpa >= 2.75) return "Second Class Honours (Lower Division)";
        if (cgpa >= 2.00) return "Pass";
        return "On Academic Probation";
    }

    // kept open so the result sets stay valid
    unique_ptr<sql::PreparedStatement> infoStmt, coursesStmt;

    unique_ptr<sql::ResultSet> myInfo() { return unique_ptr<sql::ResultSet>(infoStmt->executeQuery()); }
    unique_ptr<sql::ResultSet> myCourses() { return unique_ptr<sql::ResultSet>(coursesStmt->executeQuery()); }

public:
    StudentModule(const string& matric, sql::Connection* c) : matricNumber(matric), conn(c) {
        infoStmt.reset(conn->prepareStatement("SELECT * FROM students WHERE matric_number = ?"));
        infoStmt->setString(1, matricNumber);

        coursesStmt.reset(conn->prepareStatement(
            "SELECT e.course_code, c.course_name, c.credit_hours, e.semester, e.year, e.total_mark, e.grade, e.gpa, e.status "
            "FROM enrollments e JOIN courses c ON e.course_code = c.course_code "
            "WHERE e.matric_number = ? ORDER BY e.year, e.semester, e.course_code"));
        coursesStmt->setString(1, matricNumber);
    }

    void viewProfile() {
        Utils::header("My Profile");
        try {
            auto res = myInfo();
            if (!res->next()) {
                cout << "Student information not found." << endl;
                Utils::pause();
                return;
            }

            cout << "Matric Number    : " << res->getString("matric_number") << endl;
            cout << "Name             : " << res->getString("name") << endl;
            cout << "IC Number        : " << res->getString("ic_number") << endl;
            cout << "Gender           : " << res->getString("gender") << endl;
            cout << "Race             : " << res->getString("race") << endl;
            cout << "Religion         : " << res->getString("religion") << endl;
            cout << "Country of Birth : " << res->getString("country_birth") << endl;
            cout << "Marital Status   : " << res->getString("marital_status") << endl;
            cout << "Address          : " << res->getString("address") << endl;
            cout << "Phone            : " << res->getString("phone") << endl;
            cout << "Personal Email   : " << res->getString("personal_email") << endl;
            cout << "University Email : " << res->getString("uni_email") << endl;
            cout << "Semester / Year  : " << res->getInt("semester") << " / " << res->getInt("year") << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void viewEnrolledCourses() {
        Utils::header("My Enrolled Courses");
        try {
            auto res = myCourses();
            cout << left << setw(11) << "Code" << setw(40) << "Course Name" << setw(8) << "Credit"
                 << setw(10) << "Sem/Year" << "Status" << endl;
            cout << string(80, '-') << endl;

            bool found = false;
            while (res->next()) {
                found = true;
                cout << left << setw(11) << res->getString("course_code") << setw(40) << res->getString("course_name")
                     << setw(8) << res->getInt("credit_hours")
                     << setw(10) << (to_string(res->getInt("semester")) + "/" + to_string(res->getInt("year")))
                     << res->getString("status") << endl;
            }
            if (!found) cout << "You are not enrolled in any course yet." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void viewPerformance() {
        Utils::header("My Academic Performance");
        try {
            auto res = myCourses();
            bool graded = false, enrolled = false;
            cout << left << setw(11) << "Code" << setw(40) << "Course Name" << setw(8) << "Mark"
                 << setw(7) << "Grade" << "GPA" << endl;
            cout << string(72, '-') << endl;

            while (res->next()) {
                enrolled = true;
                cout << left << setw(11) << res->getString("course_code") << setw(40) << res->getString("course_name");
                if (res->isNull("grade")) {
                    cout << "(not graded yet)" << endl;
                    continue;
                }
                graded = true;
                cout << setw(8) << fixed << setprecision(2) << res->getDouble("total_mark")
                     << setw(7) << res->getString("grade") << res->getDouble("gpa") << endl;
            }

            if (!enrolled) cout << "You are not enrolled in any course yet." << endl;

            auto info = myInfo();
            if (info->next()) {
                double cgpa = info->getDouble("cgpa");
                cout << string(72, '-') << endl;
                cout << "CGPA           : " << fixed << setprecision(2) << cgpa << endl;
                cout << "Classification : " << (graded ? classify(cgpa) : "No results yet") << endl;
            }
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    // transcript is saved as a text file
    void generateTranscript() {
        Utils::header("Generate Academic Transcript");
        try {
            auto studentInfo = myInfo();
            if (!studentInfo->next()) {
                cout << "Student information not found." << endl;
                Utils::pause();
                return;
            }

            string studentName = studentInfo->getString("name");
            string icNumber = studentInfo->getString("ic_number");
            string email = studentInfo->getString("uni_email");
            int semester = studentInfo->getInt("semester");
            int year = studentInfo->getInt("year");
            double cgpa = studentInfo->getDouble("cgpa");

            string filename = "Transcript_" + matricNumber + ".txt";
            ofstream file(filename);
            if (!file.is_open()) {
                cout << "[ERROR] Unable to create transcript file!" << endl;
                Utils::pause();
                return;
            }

            string line(80, '=');
            file << line << "\n";
            file << "                      UNIVERSITI TEKNIKAL MALAYSIA MELAKA\n";
            file << "            FACULTY OF ARTIFICIAL INTELLIGENCE AND CYBER SECURITY\n";
            file << "                             ACADEMIC TRANSCRIPT\n";
            file << line << "\n\n";

            file << "Student Information:\n";
            file << "--------------------\n";
            file << "Name             : " << studentName << "\n";
            file << "Matric Number    : " << matricNumber << "\n";
            file << "IC Number        : " << icNumber << "\n";
            file << "Email            : " << email << "\n";
            file << "Current Semester : " << semester << "\n";
            file << "Current Year     : " << year << "\n\n";

            file << "Course Performance:\n";
            file << "-------------------\n";
            file << left << setw(12) << "Course" << setw(45) << "Course Name" << setw(10) << "Credits"
                 << setw(8) << "Grade" << setw(8) << "GPA" << "\n";
            file << string(80, '-') << "\n";

            int totalCredits = 0;
            bool hasRecords = false;
            auto courses = myCourses();
            while (courses->next()) {
                if (courses->isNull("grade")) continue;
                hasRecords = true;
                int credits = courses->getInt("credit_hours");
                string grade = courses->getString("grade");
                if (grade != "E") totalCredits += credits;

                file << left << setw(12) << courses->getString("course_code") << setw(45) << courses->getString("course_name")
                     << setw(10) << credits << setw(8) << grade
                     << setw(8) << fixed << setprecision(2) << courses->getDouble("gpa") << "\n";
            }
            if (!hasRecords) file << "No courses completed yet.\n";

            string classification = hasRecords ? classify(cgpa) : "No results yet";
            file << string(80, '-') << "\n";
            file << "Total Credits Earned: " << totalCredits << "\n\n";
            file << "Academic Standing:\n";
            file << "------------------\n";
            file << "Cumulative Grade Point Average (CGPA): " << fixed << setprecision(2) << cgpa << "\n\n";
            file << "Classification: " << classification << "\n\n";
            file << line << "\n";
            file << "Generated on: " << getCurrentDateTime() << "\n";
            file << "This transcript is for student viewing only. No signature is required.\n";
            file << line << "\n";
            file.close();

            cout << "\n[SUCCESS] Transcript generated successfully!" << endl;
            cout << "Filename: " << filename << endl;
            cout << "\n--- Transcript Summary ---" << endl;
            cout << "Credits Earned : " << totalCredits << endl;
            cout << "CGPA           : " << fixed << setprecision(2) << cgpa << endl;
            cout << "Classification : " << classification << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void menu() {
        while (true) {
            Utils::header("Student Dashboard");
            cout << "Welcome, " << matricNumber << "!\n" << endl;
            cout << "1. View My Profile" << endl;
            cout << "2. View Enrolled Courses" << endl;
            cout << "3. View Academic Performance" << endl;
            cout << "4. Generate Academic Transcript" << endl;
            cout << "5. Change Password" << endl;
            cout << "6. Logout" << endl;

            switch (Utils::readInt("Select: ")) {
            case 1: viewProfile(); break;
            case 2: viewEnrolledCourses(); break;
            case 3: viewPerformance(); break;
            case 4: generateTranscript(); break;
            case 5: Utils::changePassword(conn, matricNumber, "students", "matric_number"); break;
            case 6: return;
            default:
                cout << "Invalid choice." << endl;
                Utils::pause();
            }
        }
    }
};

#endif
