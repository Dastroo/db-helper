#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
//
// Created by dawid on 06.04.2022.
//

#include <fstream>
#include "doctest.h"
//#define private public
#include "../include/DBHelper.h"
//#undef private



//  PUBLIC FUNCTIONS TESTS
TEST_CASE("constructors") {
    SUBCASE("DBHelper()") {
        DBHelper db_helper;

        CHECK_EQ(db_helper.get_db_name(), "database.db3");
        CHECK_EQ(db_helper.get_db_dir_path(), "/home/dawid/.local/share/unittests/");
        CHECK_EQ(db_helper.get_db_full_path(), "/home/dawid/.local/share/unittests/database.db3");
    }

    SUBCASE("DBHelper(const std::string &db_path)") {
        DBHelper db_helper("/home/dawid/.local/share/unittests/test2.db3");

        CHECK_EQ(db_helper.get_db_name(), "test2.db3");
        CHECK_EQ(db_helper.get_db_dir_path(), "/home/dawid/.local/share/unittests/");
        CHECK_EQ(db_helper.get_db_full_path(), "/home/dawid/.local/share/unittests/test2.db3");
    }

    SUBCASE("DBHelper(const int &permissions)") {
        DBHelper db_helper(SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        CHECK_EQ(db_helper.get_db_name(), "database.db3");
        CHECK_EQ(db_helper.get_db_dir_path(), "/home/dawid/.local/share/unittests/");
        CHECK_EQ(db_helper.get_db_full_path(), "/home/dawid/.local/share/unittests/database.db3");
    }
}

TEST_CASE(R"(database path)") {
    DBHelper db_helper;
    SUBCASE(R"(get_default_dir_path(const std::string &db_name))") {
        CHECK_EQ(db_helper.get_default_dir_path("database.db3"), "/home/dawid/.local/share/unittests/database.db3");
    }

    SUBCASE(R"(set_db_name(const std::string &full_path))") {
        std::string path = db_helper.get_default_dir_path("database.db3");
        db_helper.set_db_name(path);
        CHECK_EQ(db_helper.get_db_name(), "database.db3");
    }

    SUBCASE(R"(set_db_dir_path(std::string full_path))") {
        std::string path = db_helper.get_default_dir_path("database.db3");
        db_helper.set_db_dir_path(path);
        CHECK_EQ(db_helper.get_db_dir_path(), "/home/dawid/.local/share/unittests/");
    }

    SUBCASE(R"(create_db_dir())") {
        DBHelper db_helper2("/home/dawid/.local/share/unittests/test3.db3");
        db_helper2.create_db_dir();
        std::ifstream file;
        file.open("/home/dawid/.local/share/unittests/test3.db3");
        CHECK_EQ(file.is_open(), true);
        //  TODO: delete databases at the end of the tests & do a proper create_db_dir() test this one is sht
    }
}

TEST_CASE(R"(sqlite statements construction)") {
    DBHelper db_helper;
    SUBCASE(R"(db_helper.parse_conditions(std::vector<std::tuple<std::string, std::string, T>> &conditions))") {
        std::vector<std::tuple<std::string, std::string, void *>> conditions;
        conditions.emplace_back(std::tuple{"a", "=", nullptr});
        conditions.emplace_back(std::tuple{"b", ">=", nullptr});

        std::vector<std::tuple<std::string, std::string, int>> conditions2;
        conditions2.emplace_back(std::tuple{"a", "=", int()});

        std::vector<std::tuple<std::string, std::string, std::string>> conditions3;
        conditions3.emplace_back(std::tuple{"a", "=", "asdf"});
        conditions3.emplace_back(std::tuple{"b", ">=", "int()"});
        conditions3.emplace_back(std::tuple{"c", ">", "j"});

        CHECK_EQ(db_helper.parse_conditions(conditions), "a=? AND b>=?");
        CHECK_EQ(db_helper.parse_conditions(conditions2), "a=?");
        CHECK_EQ(db_helper.parse_conditions(conditions3), "a=? AND b>=? AND c>?");
    }

    SUBCASE(R"(as_questionmark(const T &t))") {
        int *i = new int(1);
        CHECK_EQ(db_helper.as_questionmark(1), "?");
        CHECK_EQ(db_helper.as_questionmark(1245), "?");
        CHECK_EQ(db_helper.as_questionmark(1245.03), "?");
        CHECK_EQ(db_helper.as_questionmark("a"), "?");
        CHECK_EQ(db_helper.as_questionmark('b'), "?");
        CHECK_EQ(db_helper.as_questionmark(i), "?");
    }

    SUBCASE(R"(col_name_eq_question_mark(std::string const &str))") {
        std::string var = "default";
        const std::string const_var = "const";
        std::string &ref_var = var;
        typename integer_range_generate<std::size_t, 0, 2 - 2, 2>::type columns;
        typename integer_range_generate<std::size_t, 0, 4 - 2, 2>::type columns2;
        typename integer_range_generate<std::size_t, 0, 6 - 2, 2>::type columns3;
        typename integer_range_generate<std::size_t, 0, 8 - 2, 2>::type columns4;
        CHECK_EQ(db_helper.parse_columns_variadic(columns, std::forward_as_tuple("a", "b")), "a=?");
        CHECK_EQ(db_helper.parse_columns_variadic(columns2, std::forward_as_tuple("a", "b", "c", "d")), "a=?, c=?");
        CHECK_EQ(db_helper.parse_columns_variadic(columns3, std::forward_as_tuple("a", "b", "c", "d", 'e', 1)),
                 "a=?, c=?, e=?");
        CHECK_EQ(db_helper.parse_columns_variadic(columns4,
                                                  std::forward_as_tuple(var, "b", const_var, "d", ref_var, 1, "ptr")),
                 "default=?, const=?, default=?, ptr=?");
    }
}

TEST_CASE("drop") {
    DBHelper db_helper;

    SUBCASE(R"(drop(const std::string &table_name))") {
        std::string test2 = "test2";
        const std::string test3 = "test3";
        std::string a = "test4";
        const std::string &test4 = a;
        CHECK_EQ(db_helper.drop("test"), "DROP TABLE IF EXISTS test");
        CHECK_EQ(db_helper.drop(test2), "DROP TABLE IF EXISTS test2");
        CHECK_EQ(db_helper.drop(test3), "DROP TABLE IF EXISTS test3");
        CHECK_EQ(db_helper.drop(test4), "DROP TABLE IF EXISTS test4");
    }
}

TEST_CASE(R"(create)") {
    DBHelper db_helper;

    SUBCASE(R"(create(const std::string &table_name, Args &&...args))") {
        std::string sql2 = db_helper.create("test",
                                            "id", DBHelper::INTEGER, DBHelper::PRIMARY_KEY, DBHelper::AUTO_INCREMENT);
        std::string sql3 = db_helper.create("test2",
                                            "id", DBHelper::INTEGER, DBHelper::PRIMARY_KEY, DBHelper::AUTO_INCREMENT,
                                            "val", DBHelper::TEXT);
        std::string sql4 = db_helper.create("test3",
                                            "id", DBHelper::INTEGER, DBHelper::PRIMARY_KEY, DBHelper::AUTO_INCREMENT,
                                            "val", DBHelper::TEXT,
                                            "val2", DBHelper::BLOB);
        CHECK_EQ(sql2, "CREATE TABLE test (id INTEGER PRIMARY KEY AUTOINCREMENT)");
        CHECK_EQ(sql3, "CREATE TABLE test2 (id INTEGER PRIMARY KEY AUTOINCREMENT, val TEXT)");
        CHECK_EQ(sql4, "CREATE TABLE test3 (id INTEGER PRIMARY KEY AUTOINCREMENT, val TEXT, val2 BLOB)");
    }
}

TEST_CASE("insert") {
    DBHelper db_helper;
    SUBCASE(R"(insert(const std::string &table_name, Args ...args))") {
        //  ERROR
//        CHECK_EQ(db_helper.insert("test"), "");
//        CHECK_EQ(db_helper.insert("test", "id"), "");

        //  NORMAL
        std::string var = "id";
        CHECK_EQ(db_helper.insert("test", "id", "1"), "INSERT INTO test (id) VALUES (?)");
        CHECK_EQ(db_helper.insert("test", var, 2), "INSERT INTO test (id) VALUES (?)");
        CHECK_EQ(db_helper.insert("test", var, "3"), "INSERT INTO test (id) VALUES (?)");
        CHECK_EQ(db_helper.insert("test2", var, "val", 1, "a"), "INSERT INTO test2 (id, val) VALUES (?, ?)");
        CHECK_EQ(db_helper.insert("test2", var, "val", 2, "b"), "INSERT INTO test2 (id, val) VALUES (?, ?)");
        CHECK_EQ(db_helper.insert("test2", var, "val", 3, 'c'), "INSERT INTO test2 (id, val) VALUES (?, ?)");

    }
}

TEST_CASE("select") {
    DBHelper db_helper;
    SUBCASE(R"(select(const std::string &table_name))") {
        CHECK_EQ(db_helper.select("test")->getQuery(), "SELECT * FROM test");
    }

    //  TODO: delete
    SUBCASE(R"(select(const std::string &table_name, const std::string &column))") {
        CHECK_EQ(db_helper.select("test", "id")->getQuery(), "SELECT id FROM test");
    }

    SUBCASE(R"(select(const std::string &table_name, const std::vector<std::string> &columns))") {
        std::vector<std::string> columns;

        std::vector<std::string> columns2;
        columns2.emplace_back("id");

        std::vector<std::string> columns3;
        columns3.emplace_back("id");
        columns3.emplace_back("val");

        CHECK_EQ(db_helper.select("test", columns)->getQuery(), "SELECT * FROM test");
        CHECK_EQ(db_helper.select("test", columns2)->getQuery(), "SELECT id FROM test");
        CHECK_EQ(db_helper.select("test2", columns3)->getQuery(), "SELECT id, val FROM test2");
    }

    SUBCASE(R"(select(const std::string &table_name, std::initializer_list<std::string> columns))") {
        CHECK_EQ(db_helper.select("test2", {})->getQuery(), "SELECT * FROM test2");
        CHECK_EQ(db_helper.select("test2", {"id"})->getQuery(), "SELECT id FROM test2");
        CHECK_EQ(db_helper.select("test2", {"id", "val"})->getQuery(), "SELECT id, val FROM test2");
    }

    SUBCASE(R"(select(const std::string &table_name, const std::tuple<Col, Op, Val> &condition))") {
        std::string id = "id";
        CHECK_EQ(db_helper.select("test", std::make_tuple("id", "=", 2))->getQuery(), "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", std::make_tuple("id", '=', 2))->getQuery(), "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", std::make_tuple("id", '=', '2'))->getQuery(),
                 "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", std::make_tuple(id, '=', '2'))->getQuery(), "SELECT * FROM test WHERE id=?");
    }

    SUBCASE(R"(select(const std::string &table_name, const std::vector<std::tuple<std::string, std::string, T>> &conditions))") {
        std::vector<std::tuple<std::string, std::string, std::string>> conditions;

        std::vector<std::tuple<std::string, std::string, std::string>> conditions2;
        conditions2.emplace_back(std::tuple{"id", "=", "1"});

        std::vector<std::tuple<std::string, std::string, std::string>> conditions3;
        conditions3.emplace_back(std::tuple{"id", ">", "1"});
        conditions3.emplace_back(std::tuple{"id", "<", "3"});

        CHECK_EQ(db_helper.select("test", conditions)->getQuery(), "SELECT * FROM test");
        CHECK_EQ(db_helper.select("test", conditions2)->getQuery(), "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test2", conditions3)->getQuery(), "SELECT * FROM test2 WHERE id>? AND id<?");
    }

    SUBCASE(R"(select(const std::string &table_name, const std::tuple<Col, Op, Val> &condition, Args...columns))") {
        std::string id = "id", val = "val";
        CHECK_EQ(db_helper.select("test", std::make_tuple("id", '=', 2))->getQuery(),
                 "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", std::make_tuple("id", '=', 2), "id")->getQuery(),
                 "SELECT id FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", std::make_tuple("id", '=', 2), id)->getQuery(),
                 "SELECT id FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test2", std::make_tuple("id", '=', 2), "id", "val")->getQuery(),
                 "SELECT id, val FROM test2 WHERE id=?");
        CHECK_EQ(db_helper.select("test2", std::make_tuple("id", '=', 2), id, val)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id=?");
        CHECK_EQ(db_helper.select("test2", std::make_tuple("id", '=', 2), "id", val)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id=?");
    }

    SUBCASE(R"(select(const std::string &table_name, std::initializer_list<std::string> columns, const std::tuple<Col, Op, Val> &condition))") {
        CHECK_EQ(db_helper.select("test", {}, std::make_tuple("id", "=", 2))->getQuery(), "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", {"id"}, std::make_tuple("id", "=", 2))->getQuery(), "SELECT id FROM test WHERE id=?");

    }

    SUBCASE(R"(select(const std::string &table_name, std::vector<std::string> columns, std::vector<std::tuple<std::string, std::string, T>> conditions))") {
        std::vector<std::string> columns;
        std::vector<std::tuple<std::string, std::string, int>> conditions;

        std::vector<std::string> columns2;
        columns2.emplace_back("id");
        std::vector<std::tuple<std::string, std::string, int>> conditions2;
        conditions2.emplace_back(std::tuple{"id", "=", 1});

        std::vector<std::string> columns3;
        columns3.emplace_back("id");
        columns3.emplace_back("val");
        std::vector<std::tuple<std::string, std::string, std::string>> conditions3;
        conditions3.emplace_back(std::tuple{"id", ">", "1"});
        conditions3.emplace_back(std::tuple{"id", "<", "3"});

        auto query = db_helper.select("test", columns, conditions);
        CHECK_EQ(query->getQuery(), "SELECT * FROM test");

        auto query2 = db_helper.select("test", columns2, conditions2);
        CHECK_EQ(query2->getQuery(), "SELECT id FROM test WHERE id=?");
        query2->executeStep();
        CHECK_EQ(query2->getColumn("id").getInt(), 1);

        auto query3 = db_helper.select("test2", columns3, conditions3);
        CHECK_EQ(query3->getQuery(), "SELECT id, val FROM test2 WHERE id>? AND id<?");
        query3->executeStep();
        CHECK_EQ(query3->getColumn("id").getInt(), 2);
    }

    SUBCASE(R"(select(const std::string &table_name, const std::vector<std::tuple<std::string, std::string, T>> &conditions, Args...columns))") {
        std::vector<std::tuple<std::string, std::string, std::string>> conditions;

        std::vector<std::tuple<std::string, std::string, std::string>> conditions2;
        conditions2.emplace_back(std::tuple{"id", "=", "1"});

        std::vector<std::tuple<std::string, std::string, std::string>> conditions3;
        conditions3.emplace_back(std::tuple{"id", ">", "1"});
        conditions3.emplace_back(std::tuple{"id", "<", "3"});

        std::string val = "val", id = "id";
        CHECK_EQ(db_helper.select("test", conditions)->getQuery(), "SELECT * FROM test");
        CHECK_EQ(db_helper.select("test", conditions2)->getQuery(), "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", conditions3)->getQuery(), "SELECT * FROM test WHERE id>? AND id<?");
        CHECK_EQ(db_helper.select("test", conditions, "id")->getQuery(), "SELECT id FROM test");
        CHECK_EQ(db_helper.select("test", conditions2, "id")->getQuery(), "SELECT id FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", conditions3, id)->getQuery(), "SELECT id FROM test WHERE id>? AND id<?");
        CHECK_EQ(db_helper.select("test2", conditions3, "id", val)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id>? AND id<?");
    }

    SUBCASE(R"(select(const std::string &table_name, const std::vector<std::string> &columns, const std::tuple<Col, Op, Val> &condition))") {
        const std::vector<std::string> columns;

        std::vector<std::string> columns2;
        columns2.emplace_back("id");

        std::vector<std::string> columns3;
        columns3.emplace_back("id");
        columns3.emplace_back("val");

        std::string column = "id", op = "=";
        CHECK_EQ(db_helper.select("test", columns, std::make_tuple("id", "=", 2))->getQuery(),
                 "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", columns, std::make_tuple(column, op, 2))->getQuery(),
                 "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", columns, std::make_tuple(column, "=", 2))->getQuery(),
                 "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", columns, std::make_tuple(column, op, 2))->getQuery(),
                 "SELECT * FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", columns2, std::make_tuple(column, "=", 2))->getQuery(),
                 "SELECT id FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test2", columns3, std::make_tuple("id", op, 2))->getQuery(),
                 "SELECT id, val FROM test2 WHERE id=?");
    }

    SUBCASE(R"(select(const std::string &table_name, std::initializer_list<std::string> columns, const std::vector<std::tuple<Col, Op, Val>> &conditions))") {
        std::vector<std::tuple<std::string, std::string, std::string>> conditions;

        std::vector<std::tuple<std::string, std::string, std::string>> conditions2;
        conditions2.emplace_back(std::tuple{"id", "=", "1"});

        std::vector<std::tuple<std::string, std::string, std::string>> conditions3;
        conditions3.emplace_back(std::tuple{"id", ">", "1"});
        conditions3.emplace_back(std::tuple{"id", "<", "3"});

        std::string id = "id", val = "val";

        CHECK_EQ(db_helper.select("test", {}, conditions)->getQuery(), "SELECT * FROM test");
        CHECK_EQ(db_helper.select("test", {"id"}, conditions)->getQuery(), "SELECT id FROM test");
        CHECK_EQ(db_helper.select("test", {"id"}, conditions2)->getQuery(), "SELECT id FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test", {id}, conditions2)->getQuery(), "SELECT id FROM test WHERE id=?");
        CHECK_EQ(db_helper.select("test2", {"id", "val"}, conditions2)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id=?");
        CHECK_EQ(db_helper.select("test2", {"id", val}, conditions2)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id=?");
        CHECK_EQ(db_helper.select("test2", {id, val}, conditions2)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id=?");
        CHECK_EQ(db_helper.select("test2", {"id", "val"}, conditions3)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id>? AND id<?");
        CHECK_EQ(db_helper.select("test2", {id, "val"}, conditions3)->getQuery(),
                 "SELECT id, val FROM test2 WHERE id>? AND id<?");
    }
}

TEST_CASE("update") {
    DBHelper db_helper;
    SUBCASE(R"()") {

    }
}

TEST_CASE("get") {
    DBHelper db_helper;
    SUBCASE(R"()") {

    }
}

TEST_CASE("delete") {
    DBHelper db_helper;
    SUBCASE(R"()") {

    }
}

/*
TEST_CASE(R"()") {

}

SUBCASE(R"()") {

}
 */