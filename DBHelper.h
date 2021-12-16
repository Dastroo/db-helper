//
// Created by dawid on 11.12.2021.
//

#pragma once

#include <iostream>
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <iostream>
#include <utility>
#include <vector>
#include <sys/stat.h>

#include "Utils.h"

class DBHelper {

    SQLite::Database *database = nullptr;
    std::string db_name;
    std::string db_dir_path;
    std::string db_full_path;

public:
    explicit DBHelper(const std::string &db_name = "data_base") {
        set_db_name(db_name);
        set_db_dir_path(db_dir_path);
        set_db_full_path(db_full_path);
        set_db_dir();
        database = new SQLite::Database(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    };

    ~DBHelper() {
        delete database;
    };

    enum type {
        INTEGER,
        TEXT,
        BLOB,
        PRIMARY_KEY,
        AUTO_INCREMENT,
    };

    /**
     * use for more complicated queries that can't/are hard to be made generic
     * TODO: not working currently.. maybe..
     * @param sql
     * @return
     */
    SQLite::Statement execute(const std::string &sql) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement query(db, sql);
            return query;
        } catch (std::exception &e) {
            std::cerr << "DBHelper::execute -> " << e.what() << std::endl;
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            return {db, ""};
        }
    }

    /**
     * @brief function for creating all the annoying table creation syntax, you only need to give the data in the
     * right order and the rest will be done for you
     * @example
     * using type = DBHelper::type;
     * create("table_name", "id", type::INTEGER, type::PRIMARY_KEY, type::AUTO_INCREMENT, "value", type::TEXT)
     * @code CREATE TABLE @table_name (@args...)
     * @param table_name how should the table be named
     * @param args column names and types
     */
    template<typename ...Args>
    void create(const std::string &table_name, Args &&...args) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            std::string sql = "CREATE TABLE " + table_name + " (";

            std::string body;
            ((body += assign_value(args)), ...);
            body.erase(0, 2);

            sql += body += ')';
            SQLite::Statement query(db, sql);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::create -> " << e.what() << std::endl;
        }
    }

    /**
     * @brief DROP TABLE IF EXISTS @table_name
     * @example drop("table_name")
     * @param table_name name of table you want to delete
     */
    void drop(const std::string &table_name) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            db.exec("DROP TABLE IF EXISTS " + table_name);
        } catch (std::exception &e) {
            std::cerr << "DBHelper::drop -> " << e.what() << std::endl;
        }
    }

    /// INSERT INTO @table_name (@columns...) VALUES (@value...)
    template<typename ...C, typename ...V>
    void insert(const std::string &table_name, C &&...columns, V &&...values) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement query(db, ult::concatenate("INSERT INTO ",
                                                         table_name,
                                                         " (",
                                                         ult::intersect(columns...),
                                                         ") VALUES (",
                                                         ult::intersect(as_questionmark(values)...),
                                                         ")"
            ));
            SQLite::bind(query, std::forward<V>(values)...);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::insert -> " << e.what() << std::endl;
        }
    }

    /**
     * @brief sqlite DELETE function (cant name it delete for obvious reasons)
     * @param s
     */
    template<typename T>
    void
    remove(const std::string &table_name, const std::string &column, const std::string &condition, const T &value) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement query(db,
                                    ult::concatenate("DELETE FROM ", table_name, " WHERE ", column, condition, "?"));
            SQLite::bind(query, value);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::remove -> " << e.what() << std::endl;
        }
    }

    /// UPDATE @table_name SET @column='@value' WHERE @condition_column='@condition_value'
    template<typename T1, typename T2>
    void update(const std::string &table_name, const std::string &column, const T1 &value,
                const std::string &condition_column, const T2 &condition_value) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement query(db,
                                    ult::concatenate("UPDATE ", table_name, " SET ", column, "=? WHERE ",
                                                     condition_column, "=?"));
            SQLite::bind(query, value, condition_value);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::update -> " << e.what() << std::endl;
        }

    }

    /// UPDATE @table_name SET @column='@value' WHERE @condition_column @condition '@condition_value'
    template<typename T1, typename T2>
    void update(const std::string &table_name, const std::string &column, const T1 &value,
                const std::string &condition_column, const std::string &condition, const T2 &condition_value) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement query(db,
                                    ult::concatenate("UPDATE ", table_name, " SET ", column, "=? WHERE ",
                                                     condition_column, condition, "?"));
            SQLite::bind(query, value, condition_value);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::update -> " << e.what() << std::endl;
        }
    }

    /// SELECT @columns FROM @table_name WHERE @condition_column @condition @condition_value
    template<typename ...Args, typename T>
    SQLite::Statement* get(const std::string &table_name, const std::string &condition_column, const std::string &condition,
             const T &condition_value, Args &&...columns) {
        try {
//            SQLite::Database db(db_full_path, SQLite::OPEN_READONLY);
            auto *query = new SQLite::Statement(*database, ult::concatenate("SELECT ", ult::intersect(columns...), " FROM ", table_name,
                                                         " WHERE ",
                                                         condition_column,
                                                         condition,
                                                         "?"));
            SQLite::bind(*query, condition_value);
            return query;
        } catch (std::exception &e) {
            std::cerr << "DBHelper::get -> " << e.what() << std::endl;
        }
    }

    bool table_exists(const std::string &table_name) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READONLY);
            return db.tableExists(table_name);
        } catch (std::exception &e) {
            std::cerr << "DBHelper::table_exists -> " << e.what() << std::endl;
            return false;
        }
    }

    void write_to_cli(const std::string &table_name) {
        try {
            SQLite::Database db(db_full_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            SQLite::Statement query(db, "SELECT * FROM " + table_name);
            while (query.executeStep()) {
                for (int i = 0; i < query.getColumnCount(); ++i)
                    std::cout << query.getColumn(i) << "\t";
                std::cout << std::endl;
            }
        } catch (std::exception &e) {
            std::cerr << "DBHelper::write_to_console -> " << e.what() << std::endl;
        }
    }

    void set_db_name(const std::string &name) {
        this->db_name = name;
    }

    static void set_db_dir_path(std::string &dir_path) {
        if (ult::is_linux()) {
            if (ult::is_elevated())
                dir_path = ult::concatenate("/usr/local/share/data/", ult::to_lower(program_invocation_short_name));
            else
                dir_path = ult::concatenate(ult::get_home_path(), "/.local", "/share", "/data/",
                                            ult::to_lower(program_invocation_short_name));
        } else if (ult::is_windows()) {
            //  TODO: windows support
            dir_path = ult::get_home_path() += "needswinsupport";
        }
    }

    void set_db_full_path(std::string &full_path) {
        if (ult::is_linux())
            if (ult::is_elevated())
                full_path = ult::concatenate("/usr/local/share/data", "/",
                                             ult::to_lower(program_invocation_short_name), "/", db_name, ".db3");
            else
                full_path = ult::concatenate(ult::get_home_path(), "/.local", "/share", "/data", "/",
                                             ult::to_lower(program_invocation_short_name), "/", db_name, ".db3");
        else if (ult::is_windows())
            //  TODO: windows support
            full_path = ult::get_home_path() + "\\dbhelperprealpha\\" + db_name + ".db3";
    }

    /**
     * @brief creates db_dir_path directory if doesn't exist
     */
    void set_db_dir() {
        if (db_dir_path.empty()) {
            std::cerr << "DBHelper::set_db_dir -> db_dir_path not set" << std::endl;
            return;
        }

        if (mkdir(db_dir_path.c_str(), 0777) == -1)
            if (errno != 17)    //  suppress "File exist error"
                std::cerr << "DBHelper::set_db_dir -> " << strerror(errno) << std::endl;
    }

    /**
     * @brief dont delete is used for DBHelper::create()
     */
    [[maybe_unused]] static std::string assign_value(const std::string &s) {
        return ", " + s;
    }

    static std::string assign_value(type t) {
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
                throw std::exception();
        }
    }

    template<typename T>
    std::string as_questionmark(const T &t) {
        return "?";
    }
};

