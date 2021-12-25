//
// Created by dawid on 11.12.2021.
//

#pragma once

#include <iostream>
#include <vector>
#include <sys/stat.h>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

#include "TypeRange.h"

class DBHelper {
    SQLite::Database *database = nullptr;

    std::string db_name;
    std::string db_dir_path;
    std::string db_full_path;

public:
    enum type {
        INTEGER,
        TEXT,
        BLOB,
        PRIMARY_KEY,
        AUTO_INCREMENT,
    };

    explicit DBHelper(const std::string &db_name = "data_base",
                      const int &permissions = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    ~DBHelper();

    /**
     * use for more complicated queries that can't/are hard to be made generic
     * TODO: not working currently.. maybe..
     * @param sql
     * @return the query as SQLite::Statement
     */
    std::shared_ptr<SQLite::Statement> execute(const std::string &sql);

    bool table_exists(const std::string &table_name);

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
    inline void create(const std::string &table_name, Args &&...args) {
        if (!database) {
            std::cerr << "DBHelper::create -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            std::string sql = "CREATE TABLE " + table_name + " (";

            std::string body;
            ((body += assign_value(args)), ...);
            body.erase(0, 2);

            sql += body += ')';
            SQLite::Statement query(*database, sql);
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
    void drop(const std::string &table_name);

    /*/// INSERT INTO @table_name (@columns...) VALUES (@value...)
    template<typename ...C, typename ...V>
    inline void insert(const std::string &table_name, C ...columns, V ...values) {
        if (!database) {
            std::cerr << "DBHelper::insert -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            SQLite::Statement query(*database, concatenate(
                    "INSERT INTO ",
                    table_name,
                    " (",
                    intersect(columns...),
                    ") VALUES (",
                    intersect(as_questionmark(values)...),
                    ")"));
            SQLite::bind(query, std::forward<V>(values)...);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::insert -> " << e.what() << std::endl;
        }
    }*/

    /**
     * @brief intersects select part of given parameter pack
     * @sqlite
     * INSERT INTO table_name (first half of args...) VALUES (second half of args...);
     * @example
     * \code
     * insert("table_name", "col1", "col2", 1, 2);
     *
     * result:
     * INSERT INTO table_name (col1, col2) VALUES (1, 2);
     * \endcode
     */
    template<typename ...Args>
    inline void insert(const std::string &table_name, Args ...args) {
        try {
            constexpr size_t n = sizeof...(args);
            constexpr size_t half_n = n / 2;
            if (n % 2 != 0) {
                std::cerr << "DBHelper::insert -> " << "needs even amount of arguments\n";
                return;
            }

            std::string sql = concatenate(
                    "INSERT INTO ",
                    table_name,
                    " (",
                    intersect<0, half_n>(args...),
                    ") VALUES (",
                    intersected_questionmarks(half_n),
                    ")");
            std::cout << sql << std::endl;
            SQLite::Statement query(*database, sql);
            typename integer_range_generate<std::size_t, half_n, n-1, 1>::type indices;
            bind(query, indices, std::forward_as_tuple(std::forward<Args>(args)...));
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
    inline
    remove(const std::string &table_name, const std::string &column, const std::string &condition, const T &value) {
        if (!database) {
            std::cerr << "DBHelper::remove -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            SQLite::Statement query(*database, concatenate(
                    "DELETE FROM ",
                    table_name,
                    " WHERE ",
                    column, condition, "?"));
            SQLite::bind(query, value);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::remove -> " << e.what() << std::endl;
        }
    }

    /// SELECT @columns FROM @table_name WHERE @condition_column @condition @condition_value
    template<typename ...Args, typename T>
    std::shared_ptr<SQLite::Statement>
    inline get(const std::string &table_name, const std::string &condition_column, const std::string &condition,
               const T &condition_value, Args &&...columns) {
        if (!database) {
            std::cerr << "DBHelper::get -> " << "database is nullptr" << std::endl;
            return {};
        }

        try {
            std::shared_ptr<SQLite::Statement> query =
                    std::make_shared<SQLite::Statement>(*database, concatenate(
                            "SELECT ", intersect(columns...),
                            " FROM ", table_name,
                            " WHERE ",
                            condition_column, condition, "?"));
            SQLite::bind(*query, condition_value);

            return query;
        } catch (std::exception &e) {
            std::cerr << "DBHelper::get -> " << e.what() << std::endl;
            return {};
        }
    }

    /// SELECT @columns FROM @table_name WHERE @condition_column @condition @condition_value
    template<typename T>
    std::shared_ptr<SQLite::Statement>
    inline get(const std::string &table_name, const std::string &condition_column, const std::string &condition,
               const T &condition_value) {
        if (!database) {
            std::cerr << "DBHelper::get -> " << "database is nullptr" << std::endl;
            return {};
        }

        try {
            std::shared_ptr<SQLite::Statement> query =
                    std::make_shared<SQLite::Statement>(*database, concatenate(
                            "SELECT * FROM ", table_name,
                            " WHERE ",
                            condition_column, condition, "?"));
            SQLite::bind(*query, condition_value);
            return query;
        } catch (std::exception &e) {
            std::cerr << "DBHelper::get -> " << e.what() << std::endl;
            return {};
        }
    }

    /// UPDATE @table_name SET @column='@value' WHERE @condition_column='@condition_value'
    template<typename T1, typename T2>
    inline void update(const std::string &table_name, const std::string &column, const T1 &value,
                       const std::string &condition_column, const T2 &condition_value) {
        if (!database) {
            std::cerr << "DBHelper::update -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            SQLite::Statement query(*database, concatenate(
                    "UPDATE ", table_name,
                    " SET ", column,
                    "=? WHERE ",
                    condition_column, "=?"));
            SQLite::bind(query, value, condition_value);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::update -> " << e.what() << std::endl;
        }
    }

    /// UPDATE @table_name SET @column='@value' WHERE @condition_column @condition '@condition_value'
    template<typename T1, typename T2>
    inline void update(const std::string &table_name, const std::string &column, const T1 &value,
                       const std::string &condition_column, const std::string &condition, const T2 &condition_value) {
        if (!database) {
            std::cerr << "DBHelper::update -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            SQLite::Statement query(*database, concatenate(
                    "UPDATE ", table_name,
                    " SET ", column,
                    "=? WHERE ",
                    condition_column, condition, "?"));
            SQLite::bind(query, value, condition_value);
            query.exec();
        } catch (std::exception &e) {
            std::cerr << "DBHelper::update -> " << e.what() << std::endl;
        }
    }

    /// write to command line interface
    void write_to_cli(const std::string &table_name);

    static std::string intersected_questionmarks(int num) {
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

private:

    void set_db_name(const std::string &name);

    void set_db_dir_path(std::string &dir_path);

    void set_db_full_path(std::string &full_path);

    /**
     * @brief creates db_dir_path directory if doesn't exist
     */
    void set_db_dir();

    /**
     * @brief dont delete is used for DBHelper::create()
     */
    [[maybe_unused]] static std::string assign_value(const std::string &s);

    [[maybe_unused]] static std::string assign_value(type t);

    template<typename T>
    inline std::string as_questionmark(const T &t) {
        return "?";
    }

private:    //  utility functions
    static std::string get_home_path();

    static bool is_linux();

    static bool is_windows();

    static bool is_elevated();

    static std::string to_lower(const std::string &str);

    template<typename... S>
    std::string concatenate(S ... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<S>(args));
        return oss.str();
    }

    template<typename S1, typename... S>
    std::string intersect(const S1 &s, S ... args) {
        std::ostringstream oss;
        ((oss << s) << ... << ((std::string) ", " += std::forward<S>(args)));
        return oss.str();
    }

    std::string intersect() {
        return {};
    }

    template<typename Tuple, size_t... indexes>
    std::string intersect_indexes(integer_pack<size_t, indexes...>, Tuple &&tuple) {
        return intersect(std::get<indexes>(tuple)...);
    }

    /**
     * @brief intersects select part of given parameter pack\n
     * TODO: make it so it accepts only string types
     * @example
     * \code
     * std::string example("a", "b", "c", "d") {
     *    intersect<0, sizeof...(args) / 2>(args...)
     * }
     * output: a, b
     * \endcode
     */
    template<size_t begin, size_t end, typename... Args>
    std::string intersect(Args &&... args) {
        typename integer_range_generate<size_t, begin, end-1, 1>::type indices;
        return intersect_indexes(indices, std::forward_as_tuple(std::forward<Args>(args)...));
    }

    template<typename Args, size_t... indexes>
    void bind(SQLite::Statement &query, integer_pack<size_t, indexes...>, Args &&args) {
        int n = 1;
        try {
            (query.bind(n++, std::get<indexes>(args)), ...);
        } catch (std::exception &e) {
            std::cerr << "DBHelper::bind -> " << e.what() << std::endl;
        }
    }
};