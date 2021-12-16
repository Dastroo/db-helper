cmake_minimum_required(VERSION 3.21)
project(db-helper)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} DBHelper.h main.cpp Utils.h Utils.cpp)

target_link_libraries(${PROJECT_NAME} SQLiteCpp sqlite3)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER DBHelper.h)

#   INSTALL
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

#   FOR TESTING
set(TEST_TARGET "TestExecutable")
add_executable(${TEST_TARGET} main.cpp)
target_link_libraries(${TEST_TARGET} ${PROJECT_NAME})