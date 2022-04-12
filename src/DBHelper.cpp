//
// Created by dawid on 22.12.2021.
//

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <filesystem>
#include <my_utils/OSUtils.h>

#include "../include/DBHelper.h"


[[maybe_unused]]
DBHelper::DBHelper() {
    this->db_full_path = get_default_dir_path("database.db3");
    set_db_dir_path(db_full_path);
    set_db_name(db_full_path);
    create_db_dir();
    try {
        database = new SQLite::Database(db_full_path,
                                        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_NOMUTEX);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::DBHelper -> failed to open database: " << e.what() << std::endl;
    }
}

[[maybe_unused]]
DBHelper::DBHelper(const std::string &db_path) {
    this->db_full_path = db_path;
    set_db_dir_path(db_path);
    set_db_name(db_path);
    create_db_dir();
    try {
        database = new SQLite::Database(db_full_path,
                                        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_NOMUTEX);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::DBHelper -> failed to open database: " << e.what() << std::endl;
    }
}

[[maybe_unused]]
DBHelper::DBHelper(const int &permissions) {
    this->db_full_path = get_default_dir_path("database.db3");
    set_db_dir_path(db_full_path);
    set_db_name(db_full_path);
    create_db_dir();
    try {
        database = new SQLite::Database(db_full_path,
                                        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_NOMUTEX);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::DBHelper -> failed to open database: " << e.what() << std::endl;
    }
}

[[maybe_unused]]
DBHelper::DBHelper(const std::string &db_path,
                   const int &permissions) {
    try {
        database = new SQLite::Database(db_full_path, permissions);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::DBHelper -> failed to open database: " << e.what() << std::endl;
    }
}

DBHelper::~DBHelper() { delete database; }

void DBHelper::set_db_name(const std::string &full_path) {
    this->db_name = full_path.substr(full_path.find_last_of('/') + 1);
}

void DBHelper::set_db_dir_path(std::string full_path) {
    this->db_dir_path = full_path.erase(full_path.find_last_of('/') + 1);
}

std::string DBHelper::get_default_dir_path(const std::string &db_name) {
    if (is_linux()) {
        if (is_elevated())
            //TODO: change /usr to /var because /usr is for readonly files and db is readwrite
            return mutl::concatenate("/var/", mutl::to_lower(program_invocation_short_name), '/', db_name);
        else
            return mutl::concatenate(get_home_path(), "/.local", "/share/",
                                     mutl::to_lower(program_invocation_short_name), '/',
                                     db_name);
    } else if (is_windows()) {
        //  TODO: windows support
        return mutl::concatenate(get_home_path(), "needswinsupport");
    } else
        throw std::invalid_argument("unsupported operating system");
}

void DBHelper::create_db_dir() {
    if (db_dir_path.empty()) {
        std::cerr << "DBHelper::create_db_dir -> db_dir_path not set" << std::endl;
        return;
    }

    std::filesystem::create_directories(db_dir_path.c_str());
}

std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name) {
    try {
        std::string sql = mutl::concatenate(
                "SELECT * FROM ", table_name);

        return std::make_shared<SQLite::Statement>(*database, sql);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name, const std::string &column) {
    try {
        std::string sql = mutl::concatenate(
                "SELECT ", column,
                " FROM ", table_name);

        return std::make_shared<SQLite::Statement>(*database, sql);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 const std::vector<std::string> &columns) {
    try {
        std::string sql = mutl::concatenate(
                "SELECT ", columns.empty() ? "*" : mutl::format_with_comma<std::string>(columns),
                " FROM ", table_name);

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 std::initializer_list<std::string> columns) {
    try {
        std::string sql = mutl::concatenate(
                "SELECT ", empty(columns) ? "*" : mutl::format_with_comma(columns),
                " FROM ", table_name);

        return std::make_shared<SQLite::Statement>(*database, sql);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

std::shared_ptr<SQLite::Statement> DBHelper::execute(const std::string &sql) {
    if (!database) {
        std::cerr << "DBHelper::execute -> " << "database is nullptr" << std::endl;
        return {};
    }

    try {
        std::shared_ptr<SQLite::Statement> query = std::make_shared<SQLite::Statement>(*database, sql);
        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::execute -> " << e.what() << std::endl;
        return {};
    }
}

bool DBHelper::table_exists(const std::string &table_name) {
    if (!database) {
        std::cerr << "DBHelper::table_exists -> " << "database is nullptr" << std::endl;
        return {};
    }

    try {
        return database->tableExists(table_name);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::table_exists -> " << e.what() << std::endl;
        return false;
    }
}

std::string DBHelper::drop(const std::string &table_name) {
    if (!database) {
        std::cerr << "DBHelper::drop -> " << "database is nullptr" << std::endl;
        return {};
    }

    try {
        std::string sql = "DROP TABLE IF EXISTS " + table_name;
        database->exec(sql);
        return sql;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::drop -> " << e.what() << std::endl;
        return {};
    }
}

void DBHelper::write_to_cli(const std::string &table_name) {
    if (!database) {
        std::cerr << "DBHelper::write_to_cli -> " << "database is nullptr" << std::endl;
        return;
    }

    try {
        SQLite::Statement query(*database, "SELECT * FROM " + table_name);
        while (query.executeStep()) {
            for (int i = 0; i < query.getColumnCount(); ++i)
                std::cout << query.getColumnName(i) << ": " << query.getColumn(i) << "\t";
            std::cout << std::endl;
        }
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::write_to_cli -> " << e.what() << std::endl;
    }
}

/**
 * @brief dont delete is used for DBHelper::create()
 */
std::string DBHelper::assign_value(const std::string &s) {
    return ", " + s;
}

std::string DBHelper::assign_value(type t) {
    switch (t) {
        case INTEGER:
            return " INTEGER";
        case TEXT:
            return " TEXT";
        case BLOB:
            return " BLOB";
        case PRIMARY_KEY:
            return " PRIMARY KEY";
        case AUTO_INCREMENT:
            return " AUTOINCREMENT";
        default:
            throw SQLite::Exception("tried to use nonexistent enum type");
    }
}

bool DBHelper::table_empty(const std::string &table_name) {
    int count = database->execAndGet(mutl::concatenate(
            "SELECT COUNT(*) FROM ", table_name)).getInt();

    return count == 0;
}

std::string DBHelper::intersected_questionmarks(int num) {
    if (num < 1)
        return {};

    std::string result = "?";
    if (num == 1)
        return result;
    else
        for (int i = 1; i < num; ++i)
            result.append(", ?");

    return result;
}