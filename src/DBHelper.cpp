//
// Created by dawid on 22.12.2021.
//

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <filesystem>

#include "../include/DBHelper.h"


[[maybe_unused]]
DBHelper::DBHelper(const std::string &db_name,
                   const int &permissions) {
    set_db_name(db_name);
    set_db_dir_path(db_dir_path);
    set_db_full_path(db_full_path);
    set_db_dir();

    try {
        database = new SQLite::Database(db_full_path, permissions);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::DBHelper -> failed to open database: " << e.what() << std::endl;
    }
};

DBHelper::~DBHelper() {
    delete database;
};

[[maybe_unused]] [[maybe_unused]] std::shared_ptr<SQLite::Statement> DBHelper::execute(const std::string &sql) {
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
        std::cerr << "DBHelper::execute -> " << "database is nullptr" << std::endl;
        return {};
    }

    try {
        return database->tableExists(table_name);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::table_exists -> " << e.what() << std::endl;
        return false;
    }
}

void DBHelper::drop(const std::string &table_name) {
    if (!database) {
        std::cerr << "DBHelper::drop -> " << "database is nullptr" << std::endl;
        return;
    }

    try {
        database->exec("DROP TABLE IF EXISTS " + table_name);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::drop -> " << e.what() << std::endl;
    }
}

void DBHelper::write_to_cli(const std::string &table_name) {
    if (!database) {
        std::cerr << "DBHelper::execute -> " << "database is nullptr" << std::endl;
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
        std::cerr << "DBHelper::write_to_console -> " << e.what() << std::endl;
    }
}

void DBHelper::set_db_name(const std::string &name) {
    this->db_name = name;
}

void DBHelper::set_db_dir_path(std::string &dir_path) {
    if (is_linux()) {
        if (is_elevated())
            dir_path = concatenate("/usr/local/share/data/", to_lower(program_invocation_short_name));
        else
            dir_path = concatenate(get_home_path(), "/.local", "/share", "/data/",
                                   to_lower(program_invocation_short_name));
    } else if (is_windows()) {
        //  TODO: windows support
        dir_path = get_home_path() += "needswinsupport";
    }
}

void DBHelper::set_db_full_path(std::string &full_path) {
    if (DBHelper::is_linux())
        if (DBHelper::is_elevated())
            full_path = DBHelper::concatenate("/usr/local/share/data", "/",
                                              DBHelper::to_lower(program_invocation_short_name), "/", db_name, ".db3");
        else
            full_path = DBHelper::concatenate(DBHelper::get_home_path(), "/.local", "/share", "/data", "/",
                                              DBHelper::to_lower(program_invocation_short_name), "/", db_name, ".db3");
    else if (DBHelper::is_windows())
        //  TODO: windows support
        full_path = DBHelper::get_home_path() + "\\dbhelperprealpha\\" + db_name + ".db3";
}

void DBHelper::set_db_dir() {
    if (db_dir_path.empty()) {
        std::cerr << "DBHelper::set_db_dir -> db_dir_path not set" << std::endl;
        return;
    }

    std::filesystem::create_directories(db_dir_path.c_str());
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

std::string DBHelper::get_home_path() {
#if __linux__
    return getenv("HOME");
#endif
#if _WIN32
    return getenv("USERPROFILE");
#endif
}

bool DBHelper::is_linux() {
#if __linux__
    return true;
#else
    return false;
#endif
}

bool DBHelper::is_windows() {
#if _WIN32
    return true;
#else
    return false;
#endif
}

bool DBHelper::is_elevated() {
#if __linux__
    return !getuid();
#endif
#if _WIN32
    return // TODO: windows support
#endif
}

std::string DBHelper::to_lower(const std::string &str) {
    std::string result;
    for (auto c: str)
        result += (char) std::tolower(c);
    return result;
}

bool DBHelper::table_empty(const std::string &table_name) {
    int count = database->execAndGet(concatenate(
            "SELECT COUNT(*) FROM ", table_name)).getInt();

    return count == 0;
}
