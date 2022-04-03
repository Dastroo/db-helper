cmake_minimum_required(VERSION 3.16)
project(db-helper)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} include/DBHelper.h src/DBHelper.cpp include/TypeRange.h include/DBHelper.inl)
target_link_libraries(${PROJECT_NAME} SQLiteCpp sqlite3)

#   INSTALL
if (UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
endif ()