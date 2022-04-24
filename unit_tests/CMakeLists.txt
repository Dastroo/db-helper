add_executable(UnitTests main.cpp unit_tests.cpp doctest.h)
target_link_libraries(UnitTests db_helper SQLiteCpp sqlite3 my_utils)