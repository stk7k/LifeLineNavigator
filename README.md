# LifeLine Navigator

**A Hospital Management System built using C, C++, HTML, CSS, and JavaScript.**

LifeLine Navigator is a web-based hospital management system designed to simplify essential hospital operations, including patient registration, admission management, doctor management, bed allocation, and billing.

The project combines the efficiency of C and C++ data structures with a clean, interactive web interface..


## Features

- Dashboard: Get an overview of hospital operations.
- Patient Management: Register, view, and search patient records.
- Admission Queue: Manage patient admissions using a FIFO queue.
- Doctor Management: Maintain doctor information and availability.
- Bed Management: Manage general and ICU bed allocation.
- Billing & Discharge: Handle patient billing and discharge operations.
- Dark Mode: Easier on the eyes

## Data Structures Used

*The project demonstrates the practical application of fundamental data structures:*
    - Linked Lists: Patient and doctor record management.
    - Queues: FIFO-based patient admission processing.
    - Stacks: Bed allocation management.
    - Classes and Objects: Object-oriented implementation of hospital entities.

## Project Architecture

*The frontend communicates with the C++ backend through HTTP requests. The backend processes these requests using the existing C/C++ modules and returns responses to the frontend.*

          USER
            |
            v
     WEB INTERFACE
    (HTML / CSS / JS)
            |
            | HTTP Requests
            v
      C++ HTTP SERVER
            |
            v
     BACKEND LOGIC
       (C / C++)
            |
      +-----+------+
      |     |      |
   Patient Doctor  Bed
   Module  Module Module
            |
            v
      JSON Response
            |
            v
      WEB INTERFACE


## Project Information

- Project Name: LifeLine Navigator
- Project Type: Project-Based Learning (PBL)
- Domain: Hospital Management System
- Languages: C and C++
- Frontend: HTML, CSS and JavaScript


## Installation & Setup

*Prerequisites*

Make sure you have the following installed:
- Git
- CMake
- A C/C++ compiler (GCC/G++)
- MSYS2 with the UCRT64 environment on Windows

1. Clone the repository
    git clone [ https://github.com/stk7k/LifeLineNavigator.git ]
    - cd LifeLine-Navigator


2. Configure the project
    - cmake -S . -B build -G "MinGW Makefiles"

3. Build the project
    - cmake --build build -j4

4. Run the application
    - ./build/lifeline_server.exe

5. Open in your browser
    - Run the build to get a complete working frontend


## Contributors

The Team:
- Shorya Tamoli
- Shaurya Sharma
- Kunal Bisht
- Ujjwal Pilkhwal

**Consider Starring Our Repo If You Liked Our Product**