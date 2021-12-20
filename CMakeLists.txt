cmake_minimum_required(VERSION 3.16.3)
project(db-helper)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} DBHelper.h Utils.h Utils.cpp test/main.cpp)
target_link_libraries(${PROJECT_NAME} SQLiteCpp sqlite3)

#   evil/bad practice but very comfortable indeed
file(GLOB HEADER_FILES CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/*.h)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")

#   INSTALL
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

