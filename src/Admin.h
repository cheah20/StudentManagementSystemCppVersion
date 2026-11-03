#ifndef ADMIN_H
#define ADMIN_H

#include <sstream>
#include "Report.h"

class AdminModule {
private:
    sql::Connection* conn;

    bool recordExists(const string& table, const string& key, const string& value) {
        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT COUNT(*) AS cnt FROM " + table + " WHERE " + key + " = ?"));
            pstmt->setString(1, value);
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            if (res->next()) return res->getInt("cnt") > 0;
        }
        catch (sql::SQLException& e) {
            cout << "Error: " << e.what() << endl;
        }
        return false;
    }

    // next id, e.g. B032410009 or L004
    string nextId(const string& table, const string& key, const string& prefix, int digits) {
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        unique_ptr<sql::ResultSet> res(stmt->executeQuery(
            "SELECT MAX(" + key + ") AS last FROM " + table + " WHERE " + key + " LIKE '" + prefix + "%'"));

        int next = 1;
        if (res->next() && !res->isNull("last"))
            next = stoi(string(res->getString("last")).substr(prefix.length())) + 1;

        ostringstream id;
        id << prefix << setw(digits) << setfill('0') << next;
        return id.str();
    }

    string generateStudentMatric() { return nextId("students", "matric_number", "B032410", 3); }
    string generateStaffId() { return nextId("lecturers", "staff_id", "L", 3); }

    void listLecturers() {
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT staff_id, name FROM lecturers ORDER BY staff_id"));
        cout << "\nAvailable Lecturers:" << endl;
        while (res->next())
            cout << "  " << res->getString("staff_id") << " - " << res->getString("name") << endl;
    }

public:
    AdminModule(sql::Connection* c) : conn(c) {}

    // students
    void addStudent() {
        Utils::header("Add New Student");

        string matric = generateStudentMatric();
        string uni_email = matric + "@student.utem.edu.my";
        transform(uni_email.begin(), uni_email.end(), uni_email.begin(), ::tolower);

        cout << "Matric Number    : " << matric << endl;
        cout << "University Email : " << uni_email << endl;
        cout << "--------------------------------------------------" << endl;

        string name = Utils::readLine("Enter Full Name (empty to cancel): ");
        if (name.empty()) return;

        string ic = Utils::readLine("Enter IC Number: ");
        ic.erase(remove(ic.begin(), ic.end(), '-'), ic.end());
        if (ic.length() < 6) {
            cout << "\n[ERROR] IC number is too short." << endl;
            Utils::pause();
            return;
        }
        string gender = Utils::readLine("Enter Gender (LELAKI / PEREMPUAN): ");
        string race = Utils::readLine("Enter Race: ");
        string religion = Utils::readLine("Enter Religion: ");
        string country = Utils::readLine("Enter Country of Birth: ");
        string marital = Utils::readLine("Enter Marital Status (BUJANG / KAHWIN): ");
        string address = Utils::readLine("Enter Address: ");
        string personal_email = Utils::readLine("Enter Personal Email: ");
        string phone = Utils::readLine("Enter Phone Number: ");
        int age = Utils::readInt("Enter Age: ");
        int semester = Utils::readInt("Enter Current Semester: ");
        int year = Utils::readInt("Enter Current Year: ");

        // first password = last 6 digits of ic
        string defaultPassword = ic.substr(ic.length() - 6);

        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "INSERT INTO students (matric_number, name, ic_number, gender, race, religion, country_birth, "
                "marital_status, address, personal_email, uni_email, phone, age, semester, year, password, gpa, cgpa) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0.00, 0.00)"));
            string values[] = { matric, name, ic, Utils::upper(gender), race, religion, country,
                                Utils::upper(marital), address, personal_email, uni_email, phone };
            for (int i = 0; i < 12; i++) pstmt->setString(i + 1, values[i]);
            pstmt->setInt(13, age);
            pstmt->setInt(14, semester);
            pstmt->setInt(15, year);
            pstmt->setString(16, defaultPassword);
            pstmt->executeUpdate();

            unique_ptr<sql::PreparedStatement> userStmt(conn->prepareStatement(
                "INSERT INTO users (username, password, role) VALUES (?, ?, 'STUDENT')"));
            userStmt->setString(1, matric);
            userStmt->setString(2, defaultPassword);
            userStmt->executeUpdate();

            cout << "\n[SUCCESS] Student added successfully!" << endl;
            cout << "       Matric No : " << matric << endl;
            cout << "       Uni Email : " << uni_email << endl;
            cout << "       Password  : " << defaultPassword << endl;
            cout << "(Student should change password after first login)" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "\n[ERROR] Failed to add student: " << e.what() << endl;
        }
        Utils::pause();
    }

    void viewStudents() {
        Utils::header("View All Students");
        try {
            unique_ptr<sql::Statement> stmt(conn->createStatement());
            unique_ptr<sql::ResultSet> res(stmt->executeQuery(
                "SELECT matric_number, name, gender, semester, year, cgpa FROM students ORDER BY matric_number"));

            cout << left << setw(13) << "Matric No" << setw(32) << "Name" << setw(12) << "Gender"
                 << setw(6) << "Sem" << setw(6) << "Year" << "CGPA" << endl;
            cout << string(75, '-') << endl;

            int count = 0;
            while (res->next()) {
                count++;
                cout << left << setw(13) << res->getString("matric_number") << setw(32) << res->getString("name")
                     << setw(12) << res->getString("gender") << setw(6) << res->getInt("semester")
                     << setw(6) << res->getInt("year") << fixed << setprecision(2) << res->getDouble("cgpa") << endl;
            }
            if (count == 0) cout << "No student records found." << endl;
            else cout << "\nTotal: " << count << " students" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void searchStudent() {
        Utils::header("Search Student");
        string matric = Utils::upper(Utils::readLine("Enter Matric Number: "));

        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT * FROM students WHERE matric_number = ?"));
            pstmt->setString(1, matric);
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                cout << "\n=== STUDENT DETAILS ===" << endl;
                cout << "Matric Number    : " << res->getString("matric_number") << endl;
                cout << "Name             : " << res->getString("name") << endl;
                cout << "IC Number        : " << res->getString("ic_number") << endl;
                cout << "Gender           : " << res->getString("gender") << endl;
                cout << "Race             : " << res->getString("race") << endl;
                cout << "Religion         : " << res->getString("religion") << endl;
                cout << "Phone            : " << res->getString("phone") << endl;
                cout << "Personal Email   : " << res->getString("personal_email") << endl;
                cout << "University Email : " << res->getString("uni_email") << endl;
                cout << "Semester         : " << res->getInt("semester") << endl;
                cout << "Year             : " << res->getInt("year") << endl;
                cout << "GPA              : " << fixed << setprecision(2) << res->getDouble("gpa") << endl;
                cout << "CGPA             : " << fixed << setprecision(2) << res->getDouble("cgpa") << endl;
            }
            else {
                cout << "\n[ERROR] Student not found!" << endl;
            }
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void updateStudent() {
        Utils::header("Update Student");
        string matric = Utils::upper(Utils::readLine("Enter Matric Number: "));

        if (!recordExists("students", "matric_number", matric)) {
            cout << "\n[ERROR] Student not found!" << endl;
            Utils::pause();
            return;
        }

        const string fields[][2] = {
            {"name", "Name"}, {"phone", "Phone"}, {"personal_email", "Personal Email"},
            {"uni_email", "University Email"}, {"address", "Address"}, {"semester", "Semester"}, {"year", "Year"}
        };

        cout << "\nWhat do you want to update?" << endl;
        for (int i = 0; i < 7; i++) cout << i + 1 << ". " << fields[i][1] << endl;
        cout << "0. Back" << endl;

        int choice = Utils::readInt("Select: ");
        if (choice == 0) return;
        if (choice < 1 || choice > 7) {
            cout << "Invalid choice!" << endl;
            Utils::pause();
            return;
        }

        try {
            string column = fields[choice - 1][0];
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "UPDATE students SET " + column + " = ? WHERE matric_number = ?"));

            if (choice >= 6) pstmt->setInt(1, Utils::readInt("Enter New " + fields[choice - 1][1] + ": "));
            else pstmt->setString(1, Utils::readLine("Enter New " + fields[choice - 1][1] + ": "));
            pstmt->setString(2, matric);
            pstmt->executeUpdate();

            cout << "\n[SUCCESS] Student updated successfully!" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void deleteStudent() {
        Utils::header("Delete Student");
        string matric = Utils::upper(Utils::readLine("Enter Student Matric Number to DELETE: "));

        try {
            unique_ptr<sql::PreparedStatement> find(conn->prepareStatement(
                "SELECT name FROM students WHERE matric_number = ?"));
            find->setString(1, matric);
            unique_ptr<sql::ResultSet> res(find->executeQuery());

            if (!res->next()) {
                cout << "[ERROR] Student not found!" << endl;
                Utils::pause();
                return;
            }

            cout << "\n[WARNING] You are deleting:" << endl;
            cout << "Student: " << res->getString("name") << " (" << matric << ")" << endl;
            cout << "This will also delete all enrollment records and the user account." << endl;

            if (Utils::confirm("\nAre you sure?")) {
                // enrollments first because of the foreign key
                const string queries[] = {
                    "DELETE FROM enrollments WHERE matric_number = ?",
                    "DELETE FROM students WHERE matric_number = ?",
                    "DELETE FROM users WHERE username = ?"
                };
                for (const string& q : queries) {
                    unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(q));
                    pstmt->setString(1, matric);
                    pstmt->executeUpdate();
                }
                cout << "\n[SUCCESS] Student record deleted successfully!" << endl;
            }
            else {
                cout << "\nDeletion cancelled." << endl;
            }
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    // lecturers
    void addLecturer() {
        Utils::header("Add New Lecturer");

        string staffId = generateStaffId();
        cout << "Staff ID : " << staffId << endl;
        cout << "--------------------------------------------------" << endl;

        string name = Utils::readLine("Enter Full Name (empty to cancel): ");
        if (name.empty()) return;
        string department = Utils::readLine("Enter Department: ");
        string defaultPassword = "123456";

        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "INSERT INTO lecturers (staff_id, name, department, password) VALUES (?, ?, ?, ?)"));
            pstmt->setString(1, staffId);
            pstmt->setString(2, name);
            pstmt->setString(3, department);
            pstmt->setString(4, defaultPassword);
            pstmt->executeUpdate();

            unique_ptr<sql::PreparedStatement> userStmt(conn->prepareStatement(
                "INSERT INTO users (username, password, role) VALUES (?, ?, 'LECTURER')"));
            userStmt->setString(1, staffId);
            userStmt->setString(2, defaultPassword);
            userStmt->executeUpdate();

            cout << "\n[SUCCESS] Lecturer added! Staff ID: " << staffId << ", Password: " << defaultPassword << endl;
        }
        catch (sql::SQLException& e) {
            cout << "\n[ERROR] Failed to add lecturer: " << e.what() << endl;
        }
        Utils::pause();
    }

    void viewLecturers() {
        Utils::header("View All Lecturers");
        try {
            unique_ptr<sql::Statement> stmt(conn->createStatement());
            unique_ptr<sql::ResultSet> res(stmt->executeQuery(
                "SELECT staff_id, name, department FROM lecturers ORDER BY staff_id ASC"));

            cout << left << setw(15) << "Staff ID" << setw(30) << "Name" << setw(25) << "Department" << endl;
            cout << string(70, '-') << endl;

            bool found = false;
            while (res->next()) {
                found = true;
                cout << left << setw(15) << res->getString("staff_id") << setw(30) << res->getString("name")
                     << setw(25) << res->getString("department") << endl;
            }
            if (!found) cout << "No lecturers record found." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "Error loading lecturers: " << e.what() << endl;
        }
        Utils::pause();
    }

    void deleteLecturer() {
        Utils::header("Delete Lecturer");
        string staffId = Utils::upper(Utils::readLine("Enter Staff ID to DELETE: "));

        if (!recordExists("lecturers", "staff_id", staffId)) {
            cout << "[ERROR] Lecturer not found!" << endl;
            Utils::pause();
            return;
        }

        cout << "\nCourses taught by " << staffId << " will have no lecturer." << endl;
        if (!Utils::confirm("Are you sure?")) {
            cout << "\nDeletion cancelled." << endl;
            Utils::pause();
            return;
        }

        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("DELETE FROM lecturers WHERE staff_id = ?"));
            pstmt->setString(1, staffId);
            pstmt->executeUpdate();

            unique_ptr<sql::PreparedStatement> userStmt(conn->prepareStatement("DELETE FROM users WHERE username = ?"));
            userStmt->setString(1, staffId);
            userStmt->executeUpdate();

            cout << "\n[SUCCESS] Lecturer deleted." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    // courses
    void addCourse() {
        Utils::header("Add New Course");

        string courseCode = Utils::upper(Utils::readLine("Enter Course Code (e.g. BITP1113): "));
        if (courseCode.empty()) return;
        if (recordExists("courses", "course_code", courseCode)) {
            cout << "\n[ERROR] Course " << courseCode << " already exists!" << endl;
            Utils::pause();
            return;
        }

        string courseName = Utils::readLine("Enter Course Name: ");
        if (courseName.empty()) return;
        int credits = Utils::readInt("Enter Credit Hours (1-3): ");
        if (credits < 1 || credits > 3) {
            cout << "\n[ERROR] Credit hours must be 1, 2 or 3." << endl;
            Utils::pause();
            return;
        }
        int semester = Utils::readInt("Enter Semester: ");
        int year = Utils::readInt("Enter Year: ");

        try {
            listLecturers();
            string staffId = Utils::upper(Utils::readLine("Lecturer Staff ID (empty for none): "));
            if (!staffId.empty() && !recordExists("lecturers", "staff_id", staffId)) {
                cout << "\n[ERROR] Lecturer not found!" << endl;
                Utils::pause();
                return;
            }

            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "INSERT INTO courses (course_code, course_name, credit_hours, semester, year, staff_id) "
                "VALUES (?, ?, ?, ?, ?, ?)"));
            pstmt->setString(1, courseCode);
            pstmt->setString(2, courseName);
            pstmt->setInt(3, credits);
            pstmt->setInt(4, semester);
            pstmt->setInt(5, year);
            if (staffId.empty()) pstmt->setNull(6, sql::DataType::VARCHAR);
            else pstmt->setString(6, staffId);
            pstmt->executeUpdate();

            cout << "\n[SUCCESS] Course " << courseCode << " added!" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] Failed to add course: " << e.what() << endl;
        }
        Utils::pause();
    }

    void viewCourses() {
        Utils::header("View All Courses");
        try {
            unique_ptr<sql::Statement> stmt(conn->createStatement());
            unique_ptr<sql::ResultSet> res(stmt->executeQuery(
                "SELECT c.course_code, c.course_name, c.credit_hours, c.semester, c.year, l.name AS lecturer_name "
                "FROM courses c LEFT JOIN lecturers l ON c.staff_id = l.staff_id ORDER BY c.course_code"));

            cout << left << setw(11) << "Code" << setw(40) << "Course Name" << setw(8) << "Credit"
                 << setw(5) << "Sem" << setw(6) << "Year" << "Lecturer" << endl;
            cout << string(95, '-') << endl;

            bool found = false;
            while (res->next()) {
                found = true;
                cout << left << setw(11) << res->getString("course_code") << setw(40) << res->getString("course_name")
                     << setw(8) << res->getInt("credit_hours") << setw(5) << res->getInt("semester")
                     << setw(6) << res->getInt("year")
                     << (res->isNull("lecturer_name") ? "Unassigned" : string(res->getString("lecturer_name"))) << endl;
            }
            if (!found) cout << "No course records found." << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void updateCourse() {
        Utils::header("Update Course Information");
        string courseCode = Utils::upper(Utils::readLine("Enter Course Code to update: "));

        try {
            unique_ptr<sql::PreparedStatement> checkStmt(conn->prepareStatement(
                "SELECT c.*, l.name AS lecturer_name FROM courses c "
                "LEFT JOIN lecturers l ON c.staff_id = l.staff_id WHERE c.course_code = ?"));
            checkStmt->setString(1, courseCode);
            unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());

            if (!res->next()) {
                cout << "\n[ERROR] Course with code '" << courseCode << "' not found!" << endl;
                Utils::pause();
                return;
            }

            string currentName = res->getString("course_name");
            int currentCredits = res->getInt("credit_hours");
            string currentStaffId = res->isNull("staff_id") ? "" : string(res->getString("staff_id"));

            cout << "\n--- Current Information ---" << endl;
            cout << "Course Code     : " << courseCode << endl;
            cout << "Course Name     : " << currentName << endl;
            cout << "Credit Hours    : " << currentCredits << endl;
            cout << "Current Lecturer: " << (res->isNull("lecturer_name") ? "N/A" : string(res->getString("lecturer_name"))) << endl;

            cout << "\n--- Enter New Information (press Enter to keep current) ---" << endl;
            string newName = Utils::readLine("New Course Name: ");
            if (newName.empty()) newName = currentName;

            string creditsInput = Utils::readLine("New Credit Hours (" + to_string(currentCredits) + "): ");
            int newCredits = creditsInput.empty() ? currentCredits : stoi(creditsInput);
            if (newCredits < 1 || newCredits > 3) {
                cout << "\n[ERROR] Credit hours must be 1, 2 or 3." << endl;
                Utils::pause();
                return;
            }

            listLecturers();
            string newStaffId = Utils::upper(Utils::readLine("New Lecturer Staff ID (leave empty for no change): "));
            if (newStaffId.empty()) newStaffId = currentStaffId;
            else if (!recordExists("lecturers", "staff_id", newStaffId)) {
                cout << "\n[ERROR] Lecturer not found!" << endl;
                Utils::pause();
                return;
            }

            unique_ptr<sql::PreparedStatement> updateStmt(conn->prepareStatement(
                "UPDATE courses SET course_name = ?, credit_hours = ?, staff_id = ? WHERE course_code = ?"));
            updateStmt->setString(1, newName);
            updateStmt->setInt(2, newCredits);
            if (newStaffId.empty()) updateStmt->setNull(3, sql::DataType::VARCHAR);
            else updateStmt->setString(3, newStaffId);
            updateStmt->setString(4, courseCode);
            updateStmt->executeUpdate();
            if (newCredits != currentCredits) Utils::refreshAllCgpa(conn);

            cout << "\n[SUCCESS] Course updated successfully!" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] Failed to update course: " << e.what() << endl;
        }
        catch (exception& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    void deleteCourse() {
        Utils::header("Delete Course");
        string courseCode = Utils::upper(Utils::readLine("Enter Course Code to DELETE: "));

        if (!recordExists("courses", "course_code", courseCode)) {
            cout << "[ERROR] Course not found!" << endl;
            Utils::pause();
            return;
        }

        cout << "\nAll enrollments and marks for " << courseCode << " will be deleted too." << endl;
        if (Utils::confirm("Are you sure?")) {
            try {
                unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("DELETE FROM courses WHERE course_code = ?"));
                pstmt->setString(1, courseCode);
                pstmt->executeUpdate();
                Utils::refreshAllCgpa(conn);
                cout << "\n[SUCCESS] Course deleted." << endl;
            }
            catch (sql::SQLException& e) {
                cout << "[ERROR] " << e.what() << endl;
            }
        }
        else {
            cout << "\nDeletion cancelled." << endl;
        }
        Utils::pause();
    }

    // report
    void generateCourseReport() {
        Utils::header("Generate Course Grade Report");

        string courseCode = Utils::upper(Utils::readLine("Enter Course Code (or press Enter to cancel): "));
        if (courseCode.empty()) {
            cout << "\nOperation cancelled." << endl;
            Utils::pause();
            return;
        }

        try {
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT c.course_name, l.name AS lecturer_name FROM courses c "
                "LEFT JOIN lecturers l ON c.staff_id = l.staff_id WHERE c.course_code = ?"));
            pstmt->setString(1, courseCode);
            unique_ptr<sql::ResultSet> courseInfo(pstmt->executeQuery());

            if (!courseInfo->next()) {
                cout << "[ERROR] Course not found!" << endl;
                Utils::pause();
                return;
            }

            string courseName = courseInfo->getString("course_name");
            string lecturerName = courseInfo->isNull("lecturer_name") ? "Unassigned" : string(courseInfo->getString("lecturer_name"));

            GradeStats stats = Report::loadGrades(conn, courseCode);
            if (stats.total == 0) {
                cout << "No graded students in this course yet." << endl;
                Utils::pause();
                return;
            }

            Report::print(cout, "COURSE GRADE REPORT", courseCode + " - " + courseName, lecturerName, 0, stats);
            Report::exportCsv(courseCode, courseName, lecturerName, stats);
        }
        catch (sql::SQLException& e) {
            cout << "[ERROR] " << e.what() << endl;
        }
        Utils::pause();
    }

    // menus
    void managePeople() {
        while (true) {
            Utils::header("User Add On");
            cout << "1. Add Student" << endl;
            cout << "2. View All Students" << endl;
            cout << "3. Search Student" << endl;
            cout << "4. Update Student" << endl;
            cout << "5. Delete Student" << endl;
            cout << "6. Add Lecturer" << endl;
            cout << "7. View All Lecturers" << endl;
            cout << "8. Delete Lecturer" << endl;
            cout << "0. Back" << endl;

            switch (Utils::readInt("Select: ")) {
            case 1: addStudent(); break;
            case 2: viewStudents(); break;
            case 3: searchStudent(); break;
            case 4: updateStudent(); break;
            case 5: deleteStudent(); break;
            case 6: addLecturer(); break;
            case 7: viewLecturers(); break;
            case 8: deleteLecturer(); break;
            case 0: return;
            default:
                cout << "Invalid choice." << endl;
                Utils::pause();
            }
        }
    }

    void manageCourses() {
        while (true) {
            Utils::header("Courses Add On");
            cout << "1. Add Course" << endl;
            cout << "2. View All Courses" << endl;
            cout << "3. Update Course" << endl;
            cout << "4. Delete Course" << endl;
            cout << "0. Back" << endl;

            switch (Utils::readInt("Select: ")) {
            case 1: addCourse(); break;
            case 2: viewCourses(); break;
            case 3: updateCourse(); break;
            case 4: deleteCourse(); break;
            case 0: return;
            default:
                cout << "Invalid choice." << endl;
                Utils::pause();
            }
        }
    }

    void menu() {
        while (true) {
            Utils::header("Admin Dashboard");
            cout << "1. User add on" << endl;
            cout << "2. Courses add on" << endl;
            cout << "3. Generate Course Grade Report" << endl;
            cout << "4. Logout" << endl;

            switch (Utils::readInt("Select: ")) {
            case 1: managePeople(); break;
            case 2: manageCourses(); break;
            case 3: generateCourseReport(); break;
            case 4: return;
            default:
                cout << "Invalid choice." << endl;
                Utils::pause();
            }
        }
    }
};

#endif
