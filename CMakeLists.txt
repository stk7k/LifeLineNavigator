cmake_minimum_required(VERSION 3.20)
project(LifeLineNavigator LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE BOOL "" FORCE)
set(HTTPLIB_REQUIRE_ZLIB OFF CACHE BOOL "" FORCE)
FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
    GIT_SHALLOW TRUE
)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(httplib json)
find_package(Threads REQUIRED)

set(HOSPITAL_SOURCES
    src/bed.cpp src/bedStack.c
    src/bill.cpp src/billing.c
    src/doctor.cpp src/doctorList.c
    src/patient.cpp src/queue.c
)

add_executable(lifeline_server ${HOSPITAL_SOURCES} src/hospital_service.cpp src/api.cpp)
target_include_directories(lifeline_server PRIVATE src)
target_link_libraries(lifeline_server PRIVATE httplib::httplib nlohmann_json::nlohmann_json Threads::Threads)
if(WIN32)
    target_link_libraries(lifeline_server PRIVATE ws2_32)
endif()

# Keep the original terminal application available as a separate target.
add_executable(lifeline_cli ${HOSPITAL_SOURCES} src/main.cpp)
target_include_directories(lifeline_cli PRIVATE src)
