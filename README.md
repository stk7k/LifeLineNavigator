# LifeLine Navigator — Web Frontend + C++ API

This version keeps the original C/C++ hospital-management logic and adds a minimal browser interface.

## Features in this first web version

- Overview dashboard with patient counts and bed availability
- Patient registration and patient directory/search/filter
- FIFO admission queue and **Admit next patient** action
- Doctor availability
- General and ICU bed overview
- Patient bill lookup, extra charges, and discharge

The browser communicates with the C++ application through a local HTTP API. The queue, bed stack, doctor linked list, patient classes, and billing implementation remain C/C++.

## Requirements on Windows

Install:

1. **VS Code**
2. **Git for Windows** (CMake uses Git to download the two small header/library dependencies)
3. **CMake 3.20 or newer**
4. A C/C++ compiler. One straightforward option is **MSYS2 UCRT64** with GCC and MinGW Make:

   ```bash
   pacman -S --needed base-devel mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make git
   ```

   Run the build commands below from the **MSYS2 UCRT64 terminal**, opened in this project folder. Alternatively, use a configured Visual Studio C++ toolchain and the Visual Studio CMake generator.

## Build and run (MSYS2 UCRT64)

From the project root (the folder containing `CMakeLists.txt`):

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j
./build/lifeline_server.exe
```

Then open **http://127.0.0.1:8080** in your browser.

The first CMake configure downloads `cpp-httplib` and `nlohmann/json` from their official GitHub repositories, so an internet connection is needed for the first build. Later builds use the cached dependencies.

To stop the server, focus the terminal and press `Ctrl+C`.

## Keep using the original terminal version

The original command-line application is still built as a separate target:

```bash
./build/lifeline_cli.exe
```

## API endpoints

| Method | Endpoint | Purpose |
|---|---|---|
| GET | `/api/health` | Check API status |
| GET | `/api/dashboard` | Dashboard counters |
| GET | `/api/patients` | List patient records |
| POST | `/api/patients` | Register a patient |
| POST | `/api/admissions/next` | Admit the next waiting patient |
| GET | `/api/doctors` | List doctors and availability |
| GET | `/api/beds` | List general and ICU beds |
| GET | `/api/bills/{patientId}` | View a patient's bill |
| POST | `/api/bills/{patientId}/charges` | Add a charge (rupees) |
| POST | `/api/patients/{patientId}/discharge` | Discharge a patient and close the bill |

## Important project notes

- Data is currently held in memory, matching the original project design. Restarting the server resets patients, bills, and bed assignments.
- This is a local PBL/demo application, not a production hospital system. It has no login, database persistence, HTTPS, or production security hardening.
- Admission keeps the existing rules: severity 8–10 requires ICU; other patients use general beds. A matching free doctor is also required.
- Doctor roster and bill line-item read accessors were added so the API can return structured data without parsing terminal output.
- The original `main.cpp` remains available as the CLI entry point; the web server uses `api.cpp` and `hospital_service.cpp` instead.

## Project layout

```text
LifeLine-Navigator/
├── CMakeLists.txt
├── README.md
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── app.js
└── src/
    ├── api.cpp
    ├── hospital_service.cpp
    ├── hospital_service.h
    └── original .c, .cpp and .h source files
```
