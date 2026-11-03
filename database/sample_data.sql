-- demo data, all names and ic numbers are made up
USE student_managment_system;

INSERT INTO users (username, password, role) VALUES
('admin', 'admin', 'ADMIN'),
('L001', '123456', 'LECTURER'),
('L002', '123456', 'LECTURER'),
('L003', '123456', 'LECTURER'),
('B032410001', '101234', 'STUDENT'),
('B032410002', '082345', 'STUDENT'),
('B032410003', '143456', 'STUDENT'),
('B032410004', '054567', 'STUDENT'),
('B032410005', '125678', 'STUDENT'),
('B032410006', '016789', 'STUDENT'),
('B032410007', '107890', 'STUDENT'),
('B032410008', '138901', 'STUDENT');

INSERT INTO lecturers (staff_id, name, department, password) VALUES
('L001', 'Dr. Ahmad Faizal bin Rahman', 'Software Engineering', '123456'),
('L002', 'Pn. Nurul Aina binti Hassan', 'Artificial Intelligence', '123456'),
('L003', 'En. Tan Wei Ming', 'Cyber Security', '123456');

INSERT INTO students (matric_number, name, ic_number, gender, race, religion, country_birth, marital_status,
    address, personal_email, uni_email, phone, age, semester, year, password) VALUES
('B032410001', 'Muhammad Hakim bin Zainal', '050314101234', 'LELAKI', 'Melayu', 'Islam', 'Malaysia', 'BUJANG',
    'Jalan Merdeka 12, Melaka', 'hakim.z@example.com', 'b032410001@student.utem.edu.my', '012-0000001', 20, 1, 2, '101234'),
('B032410002', 'Lee Jia Wen', '050722082345', 'PEREMPUAN', 'Cina', 'Buddha', 'Malaysia', 'BUJANG',
    'Taman Ixora 5, Ipoh', 'jiawen.lee@example.com', 'b032410002@student.utem.edu.my', '012-0000002', 20, 1, 2, '082345'),
('B032410003', 'Kavitha a/p Ramasamy', '051105143456', 'PEREMPUAN', 'India', 'Hindu', 'Malaysia', 'BUJANG',
    'Jalan Ampang 30, Kuala Lumpur', 'kavitha.r@example.com', 'b032410003@student.utem.edu.my', '012-0000003', 20, 1, 2, '143456'),
('B032410004', 'Nur Aisyah binti Omar', '050210054567', 'PEREMPUAN', 'Melayu', 'Islam', 'Malaysia', 'BUJANG',
    'Kampung Baru 8, Seremban', 'aisyah.omar@example.com', 'b032410004@student.utem.edu.my', '012-0000004', 20, 1, 2, '054567'),
('B032410005', 'Wong Kar Hoe', '050918125678', 'LELAKI', 'Cina', 'Kristian', 'Malaysia', 'BUJANG',
    'Jalan Gaya 21, Kota Kinabalu', 'karhoe.w@example.com', 'b032410005@student.utem.edu.my', '012-0000005', 20, 1, 2, '125678'),
('B032410006', 'Daniel Raj a/l Kumar', '050403016789', 'LELAKI', 'India', 'Kristian', 'Malaysia', 'BUJANG',
    'Taman Universiti 3, Johor Bahru', 'daniel.raj@example.com', 'b032410006@student.utem.edu.my', '012-0000006', 20, 1, 2, '016789'),
('B032410007', 'Siti Khadijah binti Yusof', '051230107890', 'PEREMPUAN', 'Melayu', 'Islam', 'Malaysia', 'BUJANG',
    'Jalan Tun Razak 9, Shah Alam', 'khadijah.y@example.com', 'b032410007@student.utem.edu.my', '012-0000007', 20, 1, 2, '107890'),
('B032410008', 'Ong Zhi Hao', '050617138901', 'LELAKI', 'Cina', 'Buddha', 'Malaysia', 'BUJANG',
    'Jalan Padungan 14, Kuching', 'zhihao.ong@example.com', 'b032410008@student.utem.edu.my', '012-0000008', 20, 1, 2, '138901');

INSERT INTO courses (course_code, course_name, credit_hours, semester, year, staff_id) VALUES
('BITP1113', 'Programming Technique', 3, 1, 1, 'L001'),
('BITI1213', 'Linear Algebra and Discrete Mathematics', 3, 1, 1, 'L002'),
('BLHW1702', 'Co-curriculum', 1, 1, 1, 'L003'),
('BITU2913', 'Workshop I', 2, 1, 2, 'L001'),
('BITS2313', 'Database Systems', 3, 1, 2, 'L002');

INSERT INTO enrollments (matric_number, course_code, semester, year, total_mark, status) VALUES
('B032410001', 'BITP1113', 1, 1, 82.5, 'COMPLETED'),
('B032410002', 'BITP1113', 1, 1, 76.0, 'COMPLETED'),
('B032410003', 'BITP1113', 1, 1, 68.4, 'COMPLETED'),
('B032410004', 'BITP1113', 1, 1, 88.2, 'COMPLETED'),
('B032410005', 'BITP1113', 1, 1, 55.7, 'COMPLETED'),
('B032410006', 'BITP1113', 1, 1, 37.5, 'COMPLETED'),
('B032410007', 'BITP1113', 1, 1, 71.3, 'COMPLETED'),
('B032410008', 'BITP1113', 1, 1, 63.9, 'COMPLETED'),
('B032410001', 'BITI1213', 1, 1, 74.1, 'COMPLETED'),
('B032410002', 'BITI1213', 1, 1, 81.6, 'COMPLETED'),
('B032410003', 'BITI1213', 1, 1, 59.2, 'COMPLETED'),
('B032410004', 'BITI1213', 1, 1, 77.8, 'COMPLETED'),
('B032410005', 'BITI1213', 1, 1, 48.0, 'COMPLETED'),
('B032410006', 'BITI1213', 1, 1, 52.4, 'COMPLETED'),
('B032410007', 'BITI1213', 1, 1, 66.7, 'COMPLETED'),
('B032410008', 'BITI1213', 1, 1, 42.3, 'COMPLETED'),
('B032410001', 'BLHW1702', 1, 1, 90.0, 'COMPLETED'),
('B032410002', 'BLHW1702', 1, 1, 85.0, 'COMPLETED'),
('B032410003', 'BLHW1702', 1, 1, 78.0, 'COMPLETED'),
('B032410004', 'BLHW1702', 1, 1, 92.0, 'COMPLETED'),
('B032410005', 'BLHW1702', 1, 1, 70.0, 'COMPLETED'),
('B032410006', 'BLHW1702', 1, 1, 65.0, 'COMPLETED'),
('B032410007', 'BLHW1702', 1, 1, 88.0, 'COMPLETED'),
('B032410008', 'BLHW1702', 1, 1, 74.0, 'COMPLETED'),
('B032410001', 'BITU2913', 1, 2, 79.4, 'COMPLETED'),
('B032410002', 'BITU2913', 1, 2, 84.1, 'COMPLETED'),
('B032410003', 'BITU2913', 1, 2, 61.5, 'COMPLETED'),
('B032410004', 'BITU2913', 1, 2, 72.9, 'COMPLETED'),
('B032410005', 'BITU2913', 1, 2, 45.6, 'COMPLETED'),
('B032410006', 'BITU2913', 1, 2, NULL, 'ENROLLED'),
('B032410007', 'BITU2913', 1, 2, NULL, 'ENROLLED'),
('B032410008', 'BITU2913', 1, 2, NULL, 'ENROLLED'),
('B032410001', 'BITS2313', 1, 2, NULL, 'ENROLLED'),
('B032410002', 'BITS2313', 1, 2, NULL, 'ENROLLED'),
('B032410003', 'BITS2313', 1, 2, NULL, 'ENROLLED'),
('B032410004', 'BITS2313', 1, 2, NULL, 'ENROLLED');

-- same grading scale as the program
UPDATE enrollments SET grade = CASE
    WHEN total_mark >= 80 THEN 'A'  WHEN total_mark >= 75 THEN 'A-'
    WHEN total_mark >= 70 THEN 'B+' WHEN total_mark >= 65 THEN 'B'
    WHEN total_mark >= 60 THEN 'B-' WHEN total_mark >= 55 THEN 'C+'
    WHEN total_mark >= 50 THEN 'C'  WHEN total_mark >= 47 THEN 'C-'
    WHEN total_mark >= 44 THEN 'D+' WHEN total_mark >= 40 THEN 'D'
    ELSE 'E' END
WHERE total_mark IS NOT NULL;

UPDATE enrollments SET gpa = CASE grade
    WHEN 'A' THEN 4.00 WHEN 'A-' THEN 3.70 WHEN 'B+' THEN 3.30 WHEN 'B' THEN 3.00
    WHEN 'B-' THEN 2.70 WHEN 'C+' THEN 2.30 WHEN 'C' THEN 2.00 WHEN 'C-' THEN 1.70
    WHEN 'D+' THEN 1.30 WHEN 'D' THEN 1.00 ELSE 0.00 END
WHERE grade IS NOT NULL;

UPDATE students s JOIN (
    SELECT e.matric_number, SUM(e.gpa * c.credit_hours) / SUM(c.credit_hours) AS cgpa
    FROM enrollments e JOIN courses c ON e.course_code = c.course_code
    WHERE e.grade IS NOT NULL GROUP BY e.matric_number
) t ON s.matric_number = t.matric_number
SET s.cgpa = t.cgpa, s.gpa = t.cgpa;
