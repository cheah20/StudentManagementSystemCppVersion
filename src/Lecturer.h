#ifndef LECTURER_H
#define LECTURER_H

#include <map>
#include <cmath>
#include "Report.h"

class LecturerModule {
private:
    string staffId;
    sql::Connection* conn;

    bool studentExists(const string& matric) {
        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT COUNT(*) AS cnt FROM students WHERE matric_number = ?"));
            pstmt->setString(1, matric);
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            if (res->next()) return res->getInt("cnt") > 0;
        }
        catch (sql::SQLException& e) {
            cout << "Error: " << e.what() << endl;
        }
        return false;
    }

    // prints my courses, false when there are none
    bool listMyCourses() {
        unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
            "SELECT course_code, course_name, credit_hours FROM courses WHERE staff_id = ? ORDER BY course_code"));
        pstmt->setString(1, staffId);
        unique_ptr<sql::ResultSet> courses(pstmt->executeQuery());

        cout << "--- Your Courses ---" << endl;
        bool hasCourses = false;
        while (courses->next()) {
            hasCourses = true;
            cout << "- " << courses->getString("course_code") << ": " << courses->getString("course_name")
                 << " (" << courses->getInt("credit_hours") << " credits)" << endl;
        }
        if (!hasCourses) cout << "You don't have any courses assigned yet." << endl;
        return hasCourses;
    }

    bool ownsCourse(const string& courseCode) {
        unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
            "SELECT COUNT(*) AS cnt FROM courses WHERE course_code = ? AND staff_id = ?"));
        pstmt->setString(1, courseCode);
        pstmt->setString(2, staffId);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        return res->next() && res->getInt("cnt") > 0;
    }

    // utem grading scale
    pair<string, double> calculateGrade(double totalMark) {
        if (totalMark >= 80) return { "A", 4.00 };
        if (totalMark >= 75) return { "A-", 3.70 };
        if (totalMark >= 70) return { "B+", 3.30 };
        if (totalMark >= 65) return { "B", 3.00 };
        if (totalMark >= 60) return { "B-", 2.70 };
        if (totalMark >= 55) return { "C+", 2.30 };
        if (totalMark >= 50) return { "C", 2.00 };
        if (totalMark >= 47) return { "C-", 1.70 };
        if (totalMark >= 44) return { "D+", 1.30 };
        if (totalMark >= 40) return { "D", 1.00 };
        return { "E", 0.00 };
    }

    // cgpa = sum(gpa x credit) / sum(credit)
    double updateCgpa(const string& matric) {
        unique_ptr<sql::PreparedStatement> cgpaPstmt(conn->prepareStatement(
            "SELECT SUM(e.gpa * c.credit_hours) AS total_grade_points, SUM(c.credit_hours) AS total_credits "
            "FROM enrollments e JOIN courses c ON e.course_code = c.course_code "
            "WHERE e.matric_number = ? AND e.grade IS NOT NULL"));
        cgpaPstmt->setString(1, matric);
        unique_ptr<sql::ResultSet> cgpaCalc(cgpaPstmt->executeQuery());

        double cgpa = 0;
        if (cgpaCalc->next() && !cgpaCalc->isNull("total_credits") && cgpaCalc->getInt("total_credits") > 0)
            cgpa = cgpaCalc->getDouble("total_grade_points") / cgpaCalc->getInt("total_credits");

        unique_ptr<sql::PreparedStatement> updateStudent(conn->prepareStatement(
            "UPDATE students SET cgpa = ?, gpa = ? WHERE matric_number = ?"));
        updateStudent->setDouble(1, cgpa);
        updateStudent->setDouble(2, cgpa);
        updateStudent->setString(3, matric);
        updateStudent->executeUpdate();
        return cgpa;
    }

public:
    LecturerModule(const string& id, sql::Connection* c) : staffId(id), conn(c) {}

    void viewMyCourses() {
        Utils::header("My Courses");
        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT c.course_code, c.course_name, c.credit_hours, c.semester, c.year, COUNT(e.enrollment_id) AS students "
                "FROM courses c LEFT JOIN enrollments e ON c.course_code = e.course_code "
                "WHERE c.staff_id = ? GROUP BY c.course_code, c.course_name, c.credit_hours, c.semester, c.year "
                "ORDER BY c.course_code"));
            pstmt->setString(1, staffId);
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            cout << left << setw(11) << "Code" << setw(40) << "Course Name" << setw(8) << "Credit"
                 << setw(5) << "Sem" << setw(6) << "Year" << "Students" << endl;
            cout << string(78, '-') << endl;

            bool found = false;
            while (res->next()) {
                found = true;
                cout << left << setw(11) << res->getString("course_code") << setw(40) << res->getString("course_name")
                     << setw(8) << res->getInt("credit_hours") << setw(5) << res->getInt("semester")
                     << setw(6) << res->getInt("year") << res->getInt("students") << endl;
            }
            if (!found) cout << "You don't have any courses assigned yet." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void viewMyStudents() {
        Utils::header("My Students");
        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT e.course_code, s.matric_number, s.name, e.total_mark, e.grade, e.gpa, e.status "
                "FROM enrollments e JOIN courses c ON e.course_code = c.course_code "
                "JOIN students s ON e.matric_number = s.matric_number "
                "WHERE c.staff_id = ? ORDER BY e.course_code, s.matric_number"));
            pstmt->setString(1, staffId);
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            cout << left << setw(11) << "Course" << setw(13) << "Matric No" << setw(30) << "Name"
                 << setw(8) << "Mark" << setw(7) << "Grade" << "Status" << endl;
            cout << string(80, '-') << endl;

            bool found = false;
            while (res->next()) {
                found = true;
                cout << left << setw(11) << res->getString("course_code") << setw(13) << res->getString("matric_number")
                     << setw(30) << res->getString("name");
                if (res->isNull("total_mark")) cout << setw(8) << "-" << setw(7) << "-";
                else cout << setw(8) << fixed << setprecision(2) << res->getDouble("total_mark")
                          << Utils::gradeColor(res->getDouble("gpa")) << setw(7) << res->getString("grade") << Utils::reset();
                cout << res->getString("status") << endl;
            }
            if (!found) cout << "No students enrolled in your courses yet." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void enrollStudent() {
        Utils::header("Enroll Student");
        try {
            if (!listMyCourses()) {
                Utils::pause();
                return;
            }

            string courseCode = Utils::upper(Utils::readLine("\nEnter Course Code: "));
            if (!ownsCourse(courseCode)) {
                cout << "[ERROR] Course not found or not assigned to you!" << endl;
                Utils::pause();
                return;
            }

            string matric = Utils::upper(Utils::readLine("Enter Student Matric Number: "));
            if (!studentExists(matric)) {
                cout << "[ERROR] Student not found!" << endl;
                Utils::pause();
                return;
            }

            unique_ptr<sql::PreparedStatement> check(conn->prepareStatement(
                "SELECT COUNT(*) AS cnt FROM enrollments WHERE matric_number = ? AND course_code = ?"));
            check->setString(1, matric);
            check->setString(2, courseCode);
            unique_ptr<sql::ResultSet> res(check->executeQuery());
            if (res->next() && res->getInt("cnt") > 0) {
                cout << "[ERROR] Student is already enrolled in " << courseCode << "." << endl;
                Utils::pause();
                return;
            }

            // enrollment uses the course semester and year
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "INSERT INTO enrollments (matric_number, course_code, semester, year, status) "
                "SELECT ?, course_code, semester, year, 'ENROLLED' FROM courses WHERE course_code = ?"));
            pstmt->setString(1, matric);
            pstmt->setString(2, courseCode);
            pstmt->executeUpdate();

            cout << "\n[SUCCESS] " << matric << " enrolled in " << courseCode << "." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void dropStudent() {
        Utils::header("Drop Student");
        try {
            if (!listMyCourses()) {
                Utils::pause();
                return;
            }

            string courseCode = Utils::upper(Utils::readLine("\nEnter Course Code: "));
            if (!ownsCourse(courseCode)) {
                cout << "[ERROR] Course not found or not assigned to you!" << endl;
                Utils::pause();
                return;
            }
            string matric = Utils::upper(Utils::readLine("Enter Student Matric Number: "));

            if (!Utils::confirm("Drop " + matric + " from " + courseCode + "?")) {
                cout << "\nCancelled." << endl;
                Utils::pause();
                return;
            }

            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "DELETE FROM enrollments WHERE matric_number = ? AND course_code = ?"));
            pstmt->setString(1, matric);
            pstmt->setString(2, courseCode);

            if (pstmt->executeUpdate() == 0) {
                cout << "\n[ERROR] This student is not enrolled in " << courseCode << "." << endl;
            }
            else {
                updateCgpa(matric);
                cout << "\n[SUCCESS] Student dropped from " << courseCode << "." << endl;
            }
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    // grade management
    void keyInMarks() {
        Utils::header("Key In Student Marks");
        string matric = Utils::upper(Utils::readLine("Enter Student Matric Number: "));

        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT e.course_code, c.course_name, c.credit_hours FROM enrollments e "
                "JOIN courses c ON e.course_code = c.course_code "
                "WHERE e.matric_number = ? AND c.staff_id = ?"));
            pstmt->setString(1, matric);
            pstmt->setString(2, staffId);
            unique_ptr<sql::ResultSet> courses(pstmt->executeQuery());

            cout << "\n--- Student's Courses (Under You) ---" << endl;
            map<string, int> courseCredits;
            while (courses->next()) {
                string code = courses->getString("course_code");
                courseCredits[code] = courses->getInt("credit_hours");
                cout << "- " << code << ": " << courses->getString("course_name")
                     << " (" << courseCredits[code] << " credits)" << endl;
            }

            if (courseCredits.empty()) {
                cout << "This student is not enrolled in any of your courses!" << endl;
                Utils::pause();
                return;
            }

            string courseCode = Utils::upper(Utils::readLine("\nEnter Course Code: "));
            if (!courseCredits.count(courseCode)) {
                cout << "[ERROR] Course not in the list above." << endl;
                Utils::pause();
                return;
            }

            int creditHours = courseCredits[courseCode];
            double totalMark = 0;
            cout << "\n--- Enter Marks for " << creditHours << " Credit Course ---" << endl;

            if (creditHours == 3) {
                // assignment 10, quiz 5 + 5, midterm 20, project 20, lab 10, final 30
                double assignment = Utils::readMark("Assignment (0-100): ");
                double quiz1 = Utils::readMark("Quiz 1 (0-100): ");
                double quiz2 = Utils::readMark("Quiz 2 (0-100): ");
                double midterm = Utils::readMark("Midterm (0-100): ");
                double project = Utils::readMark("Project (0-100): ");
                double lab = Utils::readMark("Lab Test (0-100): ");
                double finalExam = Utils::readMark("Final Exam (0-100): ");

                totalMark = (assignment * 0.10) + (quiz1 * 0.05) + (quiz2 * 0.05) +
                    (midterm * 0.20) + (project * 0.20) + (lab * 0.10) + (finalExam * 0.30);
            }
            else if (creditHours == 2) {
                // assignment 1 30, assignment 2 40, final 30
                double assignment1 = Utils::readMark("Assignment 1 (0-100): ");
                double assignment2 = Utils::readMark("Assignment 2 (0-100): ");
                double finalExam = Utils::readMark("Final Exam (0-100): ");

                totalMark = (assignment1 * 0.30) + (assignment2 * 0.40) + (finalExam * 0.30);
            }
            else {
                // co-curriculum: one test
                totalMark = Utils::readMark("Test (0-100): ");
            }

            // round first so 40.00 does not become 39.99
            totalMark = round(totalMark * 100) / 100;
            auto gradeResult = calculateGrade(totalMark);
            cout << "\nCalculated Total Mark: " << fixed << setprecision(2) << totalMark << endl;
            cout << "Grade: " << Utils::gradeColor(gradeResult.second) << gradeResult.first
                 << " (GPA: " << gradeResult.second << ")" << Utils::reset() << endl;

            unique_ptr<sql::PreparedStatement> updatePstmt(conn->prepareStatement(
                "UPDATE enrollments SET total_mark = ?, grade = ?, gpa = ?, status = 'COMPLETED' "
                "WHERE matric_number = ? AND course_code = ?"));
            updatePstmt->setDouble(1, totalMark);
            updatePstmt->setString(2, gradeResult.first);
            updatePstmt->setDouble(3, gradeResult.second);
            updatePstmt->setString(4, matric);
            updatePstmt->setString(5, courseCode);
            updatePstmt->executeUpdate();

            double cgpa = updateCgpa(matric);
            cout << "\n[SUCCESS] Marks updated successfully!" << endl;
            cout << "Student's Updated CGPA: " << Utils::cgpaColor(cgpa) << fixed << setprecision(2) << cgpa << Utils::reset() << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    // report generation
    void generateCourseReport() {
        Utils::header("Generate Course Performance Report");
        try {
            if (!listMyCourses()) {
                Utils::pause();
                return;
            }

            string courseCode = Utils::upper(Utils::readLine("\nEnter Course Code for Report (or press Enter to cancel): "));
            if (courseCode.empty()) {
                cout << "\nOperation cancelled." << endl;
                Utils::pause();
                return;
            }

            unique_ptr<sql::PreparedStatement> courseInfoPstmt(conn->prepareStatement(
                "SELECT c.course_name, c.credit_hours, l.name AS lecturer_name FROM courses c "
                "JOIN lecturers l ON c.staff_id = l.staff_id WHERE c.course_code = ? AND c.staff_id = ?"));
            courseInfoPstmt->setString(1, courseCode);
            courseInfoPstmt->setString(2, staffId);
            unique_ptr<sql::ResultSet> courseInfo(courseInfoPstmt->executeQuery());

            if (!courseInfo->next()) {
                cout << "[ERROR] Course not found or not assigned to you!" << endl;
                Utils::pause();
                return;
            }

            string courseName = courseInfo->getString("course_name");
            int credits = courseInfo->getInt("credit_hours");
            string lecturerName = courseInfo->getString("lecturer_name");

            GradeStats stats = Report::loadGrades(conn, courseCode);
            if (stats.total == 0) {
                cout << "No graded students in this course yet." << endl;
                Utils::pause();
                return;
            }

            Report::print(cout, "COURSE PERFORMANCE REPORT", courseCode + " - " + courseName, lecturerName, credits, stats);

            cout << "\nExport report?  1. CSV   2. Text file   0. Skip" << endl;
            int exportChoice = Utils::readInt("Select: ");
            if (exportChoice == 1) Report::exportCsv(courseCode, courseName, lecturerName, stats);
            else if (exportChoice == 2) Report::exportTxt(courseCode, courseName, lecturerName, credits, stats);
            else cout << "\nExport skipped." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void menu() {
        while (true) {
            Utils::header("Lecturer Dashboard");
            cout << "1. View My Courses" << endl;
            cout << "2. View My Students" << endl;
            cout << "3. Enroll Student" << endl;
            cout << "4. Drop Student" << endl;
            cout << "5. Key In Marks" << endl;
            cout << "6. Generate Course Performance Report" << endl;
            cout << "7. Change Password" << endl;
            cout << "8. Logout" << endl;

            switch (Utils::readInt("Select: ")) {
            case 1: viewMyCourses(); break;
            case 2: viewMyStudents(); break;
            case 3: enrollStudent(); break;
            case 4: dropStudent(); break;
            case 5: keyInMarks(); break;
            case 6: generateCourseReport(); break;
            case 7: Utils::changePassword(conn, staffId, "lecturers", "staff_id"); break;
            case 8: return;
            default:
                cout << "Invalid choice." << endl;
                Utils::pause();
            }
        }
    }
};

#endif
