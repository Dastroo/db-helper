//
// Created by dawid on 11.12.2021.
//

#pragma once

#include <iostream>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/VariadicBind.h>
#include "TypeRange.h"

class DBHelper {
    SQLite::Database *database;

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

    [[maybe_unused]]
    explicit DBHelper(const std::string &db_name = "data_base",
                      const int &permissions = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_NOMUTEX);

    ~DBHelper();

    SQLite::Database &db() {
        return *database;
    }

    [[maybe_unused]] std::string get_db_name() {
        return db_dir_path;
    }

    [[maybe_unused]] std::string get_db_dir_path() {
        return db_dir_path;
    }

    [[maybe_unused]] std::string get_db_full_path() {
        return db_full_path;
    }

    /**
     * use for more complicated queries that can't/are hard to be made generic
     * TODO: not working currently.. maybe..
     * @param sql
     * @return the query as SQLite::Statement
     */
    [[maybe_unused]] std::shared_ptr<SQLite::Statement> execute(const std::string &sql);

    bool table_exists(const std::string &table_name);

    bool table_empty(const std::string &table_name);

    template<typename T>
    bool exists(const std::string &table_name, const std::string &column, T value);

    /**
     * @brief function for creating all the annoying table creation syntax, you only need to give the data in the
     * right order and the rest will be done for you
     * @sqlite CREATE TABLE <b>table_name</b> (<b>args...</b>);
     * @example
     * @code
     * create("table_name", "id", DBHelper::INTEGER, DBHelper::PRIMARY_KEY, DBHelper::AUTO_INCREMENT, "value", DBHelper::TEXT);
     * @endcode
     * @param table_name how should the table be named
     * @param args column names and types
     */
    template<typename ...Args>
    inline void create(const std::string &table_name, Args &&...args);

    /**
     * @brief sqlite DROP TABLE function
     * @sqlite DROP TABLE IF EXISTS <b>table_name</b>
     * @example
     * @code drop("table_name"); @endcode
     * @param table_name name of the table you want to delete
     */
    void drop(const std::string &table_name);

    /**
     * @brief Sqlite INSERT function
     * @sqlite
     * INSERT INTO <b>table_name</b> (<b>args...</b>) VALUES (<b>args...</b>);
     * @example
     * \code
     * insert("table_name", "col1", "col2", 1, 2);
     *
     * result:
     * INSERT INTO table_name (col1, col2) VALUES (1, 2);
     * \endcode
     */
    template<typename ...Args>
    inline void insert(const std::string &table_name, Args ...args);

    /**
     * @brief sqlite DELETE function (cant name it delete for obvious reasons)
     * @sqlite DELETE FROM <b>table_name</b> WHERE <b>column</b> <b>condition</b> <b>value</b>
     * @param s TODO: better documentation
     */
    template<typename T>
    void
    inline
    dele(const std::string &table_name, const std::string &column, const std::string &condition, const T &value);

    /**
     * @brief sqlite DELETE function (cant name it delete for obvious reasons)
     * @sqlite DELETE FROM <b>table_name</b> WHERE <b>column</b> = <b>value</b>
     * @param s TODO: better documentation
     */
    template<typename T>
    void
    inline
    dele(const std::string &table_name, const std::string &column, const T &value);

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>\n
     * @example
     * @code
     *  auto query = DBHelper().get("table_name", "id", 1);
     *  int id = query->getColumn("id").getInt();
     *  char* val = query->getColumn("value").getString();
     * @endcode
     *
     * @warning
     * may produce "database locked" error
     * make sure to only return this function to variable in local space like a function/method\n
     * TODO: better documentation
     */
    template<typename T>
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline get(const std::string &table_name, const std::string &condition_column,
               const T &condition_value);

    /**
     * @sqlite SELECT <b>column</b> FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>\n
     * TODO: better documentation
     */
    template<typename T>
    [[maybe_unused]] SQLite::Column
    inline get(const std::string &table_name, const std::string &column, const std::string &condition_column,
               const T &condition_value);

    /**
     * @sqlite SELECT * FROM <b>table_name</b>
     * @TODO: better documentation
     */
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline select(const std::string &table_name) {
        if (!database) {
            std::cerr << "DBHelper::select -> " << "database is nullptr" << std::endl;
            return {};
        }

        try {
            std::string sql = concatenate(
                    "SELECT * FROM ", table_name);
            return std::make_shared<SQLite::Statement>(*database, sql);
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::select -> " << e.what() << std::endl;
            return {};
        }
    }

    /**
     * @sqlite SELECT <b>column</b> FROM <b>table_name</b>
     * @TODO: better documentation
     */
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline select(const std::string &table_name, const std::string &column) {
        if (!database) {
            std::cerr << "DBHelper::select -> " << "database is nullptr" << std::endl;
            return {};
        }

        try {
            std::string sql = concatenate(
                    "SELECT ", column, " FROM ", table_name);
            std::shared_ptr<SQLite::Statement> query =
                    std::make_shared<SQLite::Statement>(*database, sql);
            return query;
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::select -> " << e.what() << std::endl;
            return {};
        }
    }

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>
     * @TODO: better documentation
     */
    template<typename T>
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline select(const std::string &table_name, const std::string &condition_column, const T &condition_value);

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b> <b>condition</b> <b>condition_value</b>
     * @TODO: better documentation
     */
    template<typename T>
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline select(const std::string &table_name, const std::string &condition_column, const std::string &condition,
                  const T &condition_value);

    /**
     * @sqlite SELECT <b>columns</b> FROM <b>table_name</b> WHERE <b>condition_column</b> <b>condition</b> <b>condition_value</b>
     * TODO: better documentation
     */
    template<typename ...Args, typename T>
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline select(const std::string &table_name, const std::string &condition_column, const std::string &condition,
                  const T &condition_value, Args &&...columns);

    /**
    *  @sqlite UPDATE <b>table_name</b> SET (args...=args...)... WHERE <b>condition_column</b>='<b>condition_value</b>'
    *  TODO: better documentation
    */
    template<typename T, typename ...Args>
    inline void update(const std::string &table_name, const std::string &condition_column, const T &condition_value,
                       Args ...args);

    /**
    *  @sqlite UPDATE <b>table_name</b> SET (args...=args...)... WHERE <b>condition_column</b>='<b>condition_value</b>'
    *  TODO: better documentation
    */
    template<typename T, typename ...Args>
    inline void
    update(const std::string &table_name, std::vector<std::tuple<std::string, std::string, T>> conditions,
           Args ...args);

    /// writes whole table to command line interface
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
    [[maybe_unused]] inline std::string as_questionmark(const T &t);

private:    //  utility functions
    static std::string get_home_path();

    static bool is_linux();

    static bool is_windows();

    static bool is_elevated();

    static std::string to_lower(const std::string &str);

    template<typename... S>
    std::string concatenate(S ... args);

    template<typename S1, typename... S>
    [[maybe_unused]] std::string intersect(const S1 &s, S ... args);

    static std::string intersect() {
        return {};
    }

    template<typename Tuple, size_t... indexes>
    std::string intersect_indexes(integer_pack<size_t, indexes...>, Tuple &&tuple);

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
    std::string intersect(Args &&... args);

    template<typename Args, size_t... indexes>
    void bind(SQLite::Statement &query, integer_pack<size_t, indexes...>, Args &&args);

    std::string col_name_eq_question_mark(std::string const &str) {
        std::ostringstream oss;
        oss << ", " << str << "=?";
        return oss.str();
    }

    std::string col_name_eq_question_mark(int const &str) {
        std::ostringstream oss;
        oss << ", " << str << "=?";
        return oss.str();
    }

    // debugging aid
    template<size_t... i, typename Args>
    void print_sequence2(integer_pack<size_t, i...>, Args &&args);

    // debugging aid
    template<size_t... ints>
    void print_sequence(integer_pack<size_t, ints...>);

    // debugging aid
    template<size_t i, typename Args>
    void print_arg(Args &&args);

    template<typename T>
    std::string form_conditions(std::vector<std::tuple<std::string, std::string, T>> &conditions);

    std::string get_str(std::string &&arg) {
        return arg; //TODO: check if its used if not delete
    }

    template<typename Args, size_t... indexes>
    std::string write_col_name_eq_question_mark(integer_pack<size_t, indexes...>, Args &&args);
};

#include "DBHelper.inl"