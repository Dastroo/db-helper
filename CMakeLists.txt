cmake_minimum_required(VERSION 3.16.3)
project(db-helper)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} include/DBHelper.h src/DBHelper.cpp include/TypeRange.h)
target_link_libraries(${PROJECT_NAME} SQLiteCpp sqlite3)

#set(LOL "*.h")
#set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER ${LOL})

#   INSTALL
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h"
)
