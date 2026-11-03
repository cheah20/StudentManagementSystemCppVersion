#ifndef REPORT_H
#define REPORT_H

#include <fstream>
#include <map>
#include "Utils.h"

// grade counts for one course
struct GradeStats {
    map<string, int> byGrade;
    int a = 0, b = 0, c = 0, d = 0, e = 0, total = 0;

    double percent(int count) const { return total ? (double)count / total * 100 : 0; }
    double passRate() const { return percent(total - e); }
};

namespace Report {
    // highest grade first
    const string GRADES[] = { "A", "A-", "B+", "B", "B-", "C+", "C", "C-", "D+", "D", "E" };

    inline GradeStats loadGrades(sql::Connection* conn, const string& courseCode) {
        GradeStats s;
        unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
            "SELECT grade, COUNT(*) AS count FROM enrollments "
            "WHERE course_code = ? AND grade IS NOT NULL "
            "GROUP BY grade"));
        pstmt->setString(1, courseCode);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            string grade = res->getString("grade");
            int count = res->getInt("count");
            s.byGrade[grade] = count;
            s.total += count;

            if (grade[0] == 'A') s.a += count;
            else if (grade[0] == 'B') s.b += count;
            else if (grade[0] == 'C') s.c += count;
            else if (grade[0] == 'D') s.d += count;
            else s.e += count;
        }
        return s;
    }

    inline void print(ostream& out, const string& title, const string& course, const string& lecturer,
                      int credits, const GradeStats& s) {
        out << "\n========================================\n";
        out << "   " << title << "\n";
        out << "========================================\n";
        out << "Course       : " << course << "\n";
        if (credits > 0) out << "Credit Hours : " << credits << "\n";
        out << "Lecturer     : " << lecturer << "\n";
        out << "========================================\n\n";

        out << "Grade Distribution:\n";
        out << "-------------------\n";
        for (const string& g : GRADES) {
            if (!s.byGrade.count(g)) continue;
            int count = s.byGrade.at(g);
            out << "Grade " << left << setw(3) << g << ": " << right << setw(3) << count
                << " students (" << fixed << setprecision(1) << s.percent(count) << "%)\n";
        }
        out << "\nTotal Students: " << s.total << "\n";

        out << "\n--- Performance Bar Chart ---\n";
        out << "Grade     : Students\n";
        out << "-----------------------------\n";
        pair<string, int> bars[] = { {"A", s.a}, {"B", s.b}, {"C", s.c}, {"D", s.d}, {"E", s.e} };
        for (auto& bar : bars)
            out << "Grade " << bar.first << "   : " << string(bar.second, '|') << " (" << bar.second << ")\n";
        out << "-----------------------------\n";

        out << "\nPass Rate: " << fixed << setprecision(1) << s.passRate() << "% ("
            << s.total - s.e << "/" << s.total << " students)\n";
    }

    inline void exportCsv(const string& code, const string& name, const string& lecturer, const GradeStats& s) {
        string filename = "Report_" + code + ".csv";
        ofstream file(filename);

        file << "Course Code,Course Name,Lecturer,Grade,Count,Percentage\n";
        for (const string& g : GRADES) {
            if (!s.byGrade.count(g)) continue;
            int count = s.byGrade.at(g);
            file << code << ",\"" << name << "\",\"" << lecturer << "\"," << g << "," << count << ","
                 << fixed << setprecision(2) << s.percent(count) << "\n";
        }
        file << "\nTotal Students," << s.total << "\n";
        file << "Pass Rate," << fixed << setprecision(2) << s.passRate() << "%\n";

        cout << "\n[SUCCESS] Report exported to '" << filename << "'" << endl;
    }

    inline void exportTxt(const string& code, const string& name, const string& lecturer, int credits, const GradeStats& s) {
        string filename = "Report_" + code + ".txt";
        ofstream file(filename);
        print(file, "COURSE PERFORMANCE REPORT", code + " - " + name, lecturer, credits, s);
        cout << "\n[SUCCESS] Report exported to '" << filename << "'" << endl;
    }
}

#endif
