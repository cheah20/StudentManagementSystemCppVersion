-- import in phpMyAdmin (XAMPP), then import sample_data.sql
CREATE DATABASE IF NOT EXISTS student_managment_system CHARACTER SET utf8mb4;
USE student_managment_system;

CREATE TABLE IF NOT EXISTS users (
    username VARCHAR(20) PRIMARY KEY,
    password VARCHAR(50) COLLATE utf8mb4_bin NOT NULL,
    role VARCHAR(10) NOT NULL
);

CREATE TABLE IF NOT EXISTS students (
    matric_number VARCHAR(10) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    ic_number VARCHAR(15) NOT NULL,
    gender VARCHAR(10) NOT NULL,
    race VARCHAR(30),
    religion VARCHAR(30),
    country_birth VARCHAR(50),
    marital_status VARCHAR(20),
    address TEXT,
    personal_email VARCHAR(100),
    uni_email VARCHAR(100) NOT NULL,
    phone VARCHAR(15),
    age INT,
    semester INT NOT NULL,
    year INT NOT NULL,
    password VARCHAR(50) COLLATE utf8mb4_bin NOT NULL,
    gpa DOUBLE DEFAULT 0,
    cgpa DOUBLE DEFAULT 0
);

CREATE TABLE IF NOT EXISTS lecturers (
    staff_id VARCHAR(10) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    department VARCHAR(50),
    password VARCHAR(50) COLLATE utf8mb4_bin NOT NULL
);

CREATE TABLE IF NOT EXISTS courses (
    course_code VARCHAR(10) PRIMARY KEY,
    course_name VARCHAR(100) NOT NULL,
    credit_hours INT NOT NULL,
    semester INT NOT NULL,
    year INT NOT NULL,
    staff_id VARCHAR(10),
    FOREIGN KEY (staff_id) REFERENCES lecturers(staff_id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS enrollments (
    enrollment_id INT AUTO_INCREMENT PRIMARY KEY,
    matric_number VARCHAR(10) NOT NULL,
    course_code VARCHAR(10) NOT NULL,
    semester INT,
    year INT,
    total_mark DOUBLE,
    grade VARCHAR(2),
    gpa DOUBLE,
    status VARCHAR(15) NOT NULL DEFAULT 'ENROLLED',
    UNIQUE (matric_number, course_code),
    FOREIGN KEY (matric_number) REFERENCES students(matric_number) ON DELETE CASCADE,
    FOREIGN KEY (course_code) REFERENCES courses(course_code) ON DELETE CASCADE
);
