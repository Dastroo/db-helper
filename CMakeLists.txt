cmake_minimum_required(VERSION 3.16)
project(db-helper)

set(CMAKE_CXX_STANDARD 17)

enable_testing()
add_subdirectory(unit_tests)

add_library(${PROJECT_NAME} include/DBHelper.h include/DBHelper.inl src/DBHelper.cpp)
target_link_libraries(${PROJECT_NAME} SQLiteCpp sqlite3 my_utils)

#   INSTALL
if (UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
endif ()