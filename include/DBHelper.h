//
// Created by dawid on 11.12.2021.
//

#pragma once

#include <vector>
#include <iostream>

#include <SQLiteCpp/Column.h>
#include <SQLiteCpp/VariadicBind.h>
#include <memory>
#include <sstream>
#include <my_utils/StringUtils.h>
#include <my_utils/ArgumentUtils.h>


namespace SQLite {
    class Database;
}

#define private public

//TODO: exception handling
//TODO: add && in arg bundles
class DBHelper {
    SQLite::Database *database;

    /// the name of the created database with extension @example database.db3
    std::string db_name;
    /// the path to the directory holding the database @example /home/username/.local/share/ProjectName/
    std::string db_dir_path;
    /// the complete path to the database @example /home/username/.local/share/ProjectName/database.db3
    std::string db_full_path;

    //  TODO: add support for project wide database path initialization
    inline static std::string default_path;

public:
    enum type {
        INTEGER,
        TEXT,
        BLOB,
        PRIMARY_KEY,
        AUTO_INCREMENT,
    };

    /**
     * TODO: write documentation
     * creates db at a default location depending on the OS in use and the PERMISSION level
     * @example
     * <b>Linux:</b>\n\n
     * <b>user level</b>\n
     *  /home/username/.local/share/ProjectName/database.db3\n\n
     * <b>root level</b>\n
     *  /var/ProjectName/database.db3\n\n
     * @example
     * <b>Windows:<b>\n\n
     * <b>not supported yet</b>\n
     */
    explicit DBHelper();

    explicit DBHelper(const std::string &db_path);

    explicit DBHelper(const int &permissions);

    /**
     * TODO: write documentation
     */
    DBHelper(const std::string &db_path,
                      const int &permissions);

    ~DBHelper();

    SQLite::Database &db() { return *database; }

    inline std::string get_db_name() { return db_name; }

    inline std::string get_db_dir_path() { return db_dir_path; }

    inline std::string get_db_full_path() { return db_full_path; }

    /**
     * use for more complicated queries that can't/are hard to be made generic
     * TODO: not working currently.. maybe..
     * @param sql
     * @return the query as SQLite::Statement
     */
    [[maybe_unused]]
    std::shared_ptr<SQLite::Statement> execute(const std::string &sql);

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
    inline std::string create(const std::string &table_name, Args &&...args);

    /**
     * @brief sqlite DROP TABLE function
     * @sqlite DROP TABLE IF EXISTS <b>table_name</b>
     * @example
     * @code drop("table_name"); @endcode
     * @param table_name name of the table you want to delete
     */
    std::string drop(const std::string &table_name);

//======================================================================================================================

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
    inline std::string insert(const std::string &table_name, Args ...args);

    /**
     * @brief Sqlite INSERT function
     * @sqlite
     * INSERT INTO <b>table_name</b> (<b>args...</b>) VALUES (<b>args...</b>);
     * @example
     * \code
     * insert("table_name", "col1", "col2", 1, 2);
     *  // TODO: write documentation
     * result:
     * INSERT INTO table_name (col1, col2) VALUES (1, 2);
     * \endcode
     */
    template<typename ...Args>
    inline std::string
    insert(const std::string &table_name, std::initializer_list<std::string> columns, Args...values);

    /**
     * @brief Sqlite INSERT function
     * @sqlite
     * INSERT INTO <b>table_name</b> (<b>args...</b>) VALUES (<b>args...</b>);
     * @example
     * \code
     * insert("table_name", "col1", "col2", 1, 2);
     *  // TODO: write documentation
     * result:
     * INSERT INTO table_name (col1, col2) VALUES (1, 2);
     * \endcode
     */
    template<typename T>
    inline std::string
    insert(const std::string &table_name, const std::vector<std::pair<std::string, T>> &columns_values);

//======================================================================================================================

    template<typename Col, typename Op, typename Val>
    inline std::string
    dele(const std::string &table_name, const std::tuple<Col, Op, Val> &condition);

    /**
     * @brief sqlite DELETE function (cant name it delete for obvious reasons)
     * @sqlite DELETE FROM <b>table_name</b> WHERE <b>column</b> <b>op</b> <b>value</b>
     * @param s TODO: better documentation
     */
    template<typename T>
    inline std::string
    dele(const std::string &table_name, const std::string &column, const std::string &op, const T &value);

    /**
     * @brief sqlite DELETE function (cant name it delete for obvious reasons)
     * @sqlite DELETE FROM <b>table_name</b> WHERE <b>column</b> = <b>value</b>
     * @param s TODO: better documentation
     */
    template<typename T>
    inline std::string
    dele(const std::string &table_name, const std::string &column, const T &value);

    template<typename Col, typename Op, typename Val>
    inline std::string
    dele(const std::string &table_name, std::vector<std::tuple<Col, Op, Val>> &conditions);

//======================================================================================================================

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>\n
     * @example
     * @code
     *  auto query = DBHelper().get("table_name", "id", 1);
     *  int id = query->getColumn("id").getInt();
     *  char* val = query->getColumn("value").getString();
     * @endcode
     * @warning
     * may produce "database locked" error
     * make sure to only return this function to variable in local space like a function/method\n
     * TODO: better documentation
     */
    template<typename T>
    inline SQLite::Column
    get(const std::string &table_name, const std::string &condition_column, const T &condition_value);

    /**
     * @sqlite SELECT <b>column</b> FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>\n
     * TODO: better documentation
     */
    template<typename T>
    inline SQLite::Column
    get(const std::string &table_name, const std::string &column,
        const std::string &condition_column, const T &condition_value);

    /**
     * @sqlite SELECT <b>column</b> FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>\n
     * TODO: better documentation
     */
    template<typename Col, typename Op, typename Val>
    inline SQLite::Column
    get(const std::string &table_name, const std::string &column,
        const std::tuple<Col, Op, Val> &condition);

    /**
     * @sqlite SELECT <b>column</b> FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>\n
     * TODO: better documentation
     */
    template<typename Col, typename Op, typename Val>
    inline SQLite::Column
    get(const std::string &table_name, const std::string &column,
        const std::vector<std::tuple<Col, Op, Val>> &conditions);

//======================================================================================================================

    /**
     * @sqlite SELECT * FROM <b>table_name</b>
     * @TODO: better documentation
     */
    std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name);

    /**
     * @sqlite SELECT <b>column</b> FROM <b>table_name</b>
     * @TODO: delete maybe, maybe it provides a speed boost
     */
    std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name, const std::string &column);

    /**
    * @sqlite SELECT <b>columns</b> FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>
    * @TODO: better documentation
    */
    std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           const std::vector<std::string> &columns);

    /**
     * @sqlite SELECT <b>columns</b> FROM <b>table_name</b>
     * @TODO: documentation
     */
    std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           std::initializer_list<std::string> columns);

    /**
     * @sqlite SELECT <b>columns</b> FROM <b>table_name</b>
     * @TODO: documentation
     */
    template<typename Col, typename Op, typename Val>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           const std::tuple<Col, Op, Val> &condition);

    /**
    * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>
    * @TODO: better documentation
    */
    template<typename Col, typename Op, typename Val>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           const std::vector<std::tuple<Col, Op, Val>> &conditions);

    /**
     * @sqlite SELECT <b>columns...</b> FROM <b>table_name</b> WHERE <b>condition_column</b> <b>condition</b> <b>condition_value</b>
     * TODO: better documentation
     */
    template<typename Col, typename Op, typename Val, typename ...Args>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           const std::tuple<Col, Op, Val> &condition,
           Args...columns);

    /**
     * @sqlite SELECT <b>columns...</b> FROM <b>table_name</b> WHERE <b>condition_column</b> <b>condition</b> <b>condition_value</b>
     * TODO: better documentation
     */
    template<typename Col, typename Op, typename Val>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           const std::vector<std::string> &columns,
           const std::tuple<Col, Op, Val> &condition);

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>
     * @TODO: better documentation
     */
    template<typename Col, typename Op, typename Val>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           std::initializer_list<std::string> columns,
           const std::tuple<Col, Op, Val> &condition);

    /**
    * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>
    * @TODO: better documentation
    */
    template<typename Col, typename Op, typename Val, typename ...Args>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           const std::vector<std::tuple<Col, Op, Val>> &conditions,
           Args...columns);

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>
     * @TODO: better documentation
     */
    template<typename Col, typename Op, typename Val>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           const std::vector<std::string> &columns,
           const std::vector<std::tuple<Col, Op, Val>> &conditions);

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>
     * @TODO: better documentation
     */
    template<typename Col, typename Op, typename Val>
    inline std::shared_ptr<SQLite::Statement>
    select(const std::string &table_name,
           std::initializer_list<std::string> columns,
           const std::vector<std::tuple<Col, Op, Val>> &conditions);

//======================================================================================================================

    /**
    *  @sqlite UPDATE <b>table_name</b> SET (args...=args...)... WHERE <b>condition_column</b>='<b>condition_value</b>'
    *  TODO: better documentation
    */
    template<typename T, typename ...Args>
    inline std::string
    update(const std::string &table_name, const std::string &condition_column, const T &condition_value, Args ...args);

    /**
    *  @sqlite UPDATE <b>table_name</b> SET (args...=args...)... WHERE <b>condition_column</b>='<b>condition_value</b>'
    *  TODO: better documentation
    */
    template<typename Col, typename Op, typename Val, typename ...Args>
    inline std::string
    update(const std::string &table_name, const std::tuple<Col, Op, Val> &condition, Args ...args);

    /**
    *  @sqlite UPDATE <b>table_name</b> SET (args...=args...)... WHERE <b>condition_column</b>='<b>condition_value</b>'
    *  TODO: better documentation, broken arguments args seem useless
    */
    template<typename T, typename ...Args>
    inline std::string
    update(const std::string &table_name, std::vector<std::tuple<std::string, std::string, T>> conditions,
           Args ...args);

    /// writes whole table to command line interface
    void write_to_cli(const std::string &table_name);

private:
    static std::string intersected_questionmarks(int num);

    std::string get_default_dir_path(const std::string &db_name);

    void set_db_name(const std::string &full_path);

    void set_db_dir_path(std::string full_path);

    /**
     * @brief creates db_dir_path directory if doesn't exist
     */
    void create_db_dir();

    /**
     * @brief dont delete is used for DBHelper::create()
     */
    static std::string assign_value(const std::string &s);

    static std::string assign_value(type t);

    template<typename T>
    inline std::string as_questionmark(const T &t);

    template<typename Args, size_t... indexes>
    void bind(SQLite::Statement &query, integer_pack<size_t, indexes...>, Args &&args);

    template<typename Col, typename Op, typename Val>
    std::string format_into_question_mark_equation_logic(const std::vector<std::tuple<Col, Op, Val>> &conditions);

    template<typename Args, size_t... indexes>
    std::string format_into_question_mark_equation_comma(integer_pack<size_t, indexes...>, Args &&args);
};

#undef private

#include "DBHelper.inl"