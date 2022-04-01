//
// Created by dawid on 11.12.2021.
//

#pragma once

/// TODO: divide template functions into a separate .inl file
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
    bool exists(const std::string &table_name, const std::string &column, T value) {
        SQLite::Column c = database->execAndGet(concatenate(
                "SELECT EXISTS(SELECT 1 FROM ", table_name, " WHERE ", column, "=\"", value, "\" LIMIT 1)"));
        return c.getInt();
    }

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
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::create -> " << e.what() << std::endl;
        }
    }

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
            SQLite::Statement query(*database, sql);
            typename integer_range_generate<std::size_t, half_n, n - 1, 1>::type indices;
            bind(query, indices, std::forward_as_tuple(std::forward<Args>(args)...));
            query.exec();
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::insert -> " << e.what() << std::endl;
        }
    }

    /**
     * @brief sqlite DELETE function (cant name it delete for obvious reasons)
     * @sqlite DELETE FROM <b>table_name</b> WHERE <b>column</b> <b>condition</b> <b>value</b>
     * @param s TODO: better documentation
     */
    template<typename T>
    void
    inline
    dele(const std::string &table_name, const std::string &column, const std::string &condition, const T &value) {
        if (!database) {
            std::cerr << "DBHelper::remove -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            std::string sql = concatenate(
                    "DELETE FROM ",
                    table_name,
                    " WHERE ",
                    column, condition, "?");
            SQLite::Statement query(*database, sql);
            SQLite::bind(query, value);
            query.exec();
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::dele -> " << e.what() << std::endl;
        }
    }

    /**
     * @brief sqlite DELETE function (cant name it delete for obvious reasons)
     * @sqlite DELETE FROM <b>table_name</b> WHERE <b>column</b> = <b>value</b>
     * @param s TODO: better documentation
     */
    template<typename T>
    void
    inline
    dele(const std::string &table_name, const std::string &column, const T &value) {
        if (!database) {
            std::cerr << "DBHelper::remove -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            std::string sql = concatenate(
                    "DELETE FROM ",
                    table_name,
                    " WHERE ",
                    column, "=", "?");
            SQLite::Statement query(*database, sql);
            SQLite::bind(query, value);
            query.exec();
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::dele -> " << e.what() << std::endl;
        }
    }

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
               const T &condition_value) {
        if (!database) {
            std::cerr << "DBHelper::get -> " << "database is nullptr" << std::endl;
            return {};
        }

        try {
            std::string sql = concatenate(
                    "SELECT * FROM ", table_name,
                    " WHERE ",
                    condition_column, '=', "?");

            std::shared_ptr<SQLite::Statement> query =
                    std::make_shared<SQLite::Statement>(*database, sql);
            SQLite::bind(*query, condition_value);
            query->executeStep();
            return query;
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::get -> " << e.what() << std::endl;
            return {};
        }
    }

    /**
     * @sqlite SELECT <b>column</b> FROM <b>table_name</b> WHERE <b>condition_column</b>=<b>condition_value</b>\n
     * TODO: better documentation
     */
    template<typename T>
    [[maybe_unused]] SQLite::Column
    inline get(const std::string &table_name, const std::string &column, const std::string &condition_column,
               const T &condition_value) {
        if (!database) {
            std::cerr << "DBHelper::get -> " << "database is nullptr" << std::endl;
        }

        try {
            std::string sql = concatenate(
                    "SELECT ", column,
                    " FROM ", table_name,
                    " WHERE ",
                    condition_column, "=", "?");
            SQLite::Statement query(*database, sql);
            SQLite::bind(query, condition_value);
            query.executeStep();
            return query.getColumn(0);
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::get -> " << e.what() << std::endl;
        }
    }

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
    inline select(const std::string &table_name, const std::string &condition_column, const T &condition_value) {
        if (!database) {
            std::cerr << "DBHelper::select -> " << "database is nullptr" << std::endl;
            return {};
        }

        try {
            std::string sql = concatenate(
                    "SELECT * FROM ", table_name,
                    " WHERE ",
                    condition_column, "=?");
            std::shared_ptr<SQLite::Statement> query =
                    std::make_shared<SQLite::Statement>(*database, sql);
            SQLite::bind(*query, condition_value);
            return query;
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::select -> " << e.what() << std::endl;
            return {};
        }
    }

    /**
     * @sqlite SELECT * FROM <b>table_name</b> WHERE <b>condition_column</b> <b>condition</b> <b>condition_value</b>
     * @TODO: better documentation
     */
    template<typename T>
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline select(const std::string &table_name, const std::string &condition_column, const std::string &condition,
                  const T &condition_value) {
        if (!database) {
            std::cerr << "DBHelper::select -> " << "database is nullptr" << std::endl;
            return {};
        }

        try {
            std::string sql = concatenate(
                    "SELECT * FROM ", table_name,
                    " WHERE ",
                    condition_column, condition, "?");
            std::shared_ptr<SQLite::Statement> query =
                    std::make_shared<SQLite::Statement>(*database, sql);
            SQLite::bind(*query, condition_value);
            return query;
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::select -> " << e.what() << std::endl;
            return {};
        }
    }

    /**
     * @sqlite SELECT <b>columns</b> FROM <b>table_name</b> WHERE <b>condition_column</b> <b>condition</b> <b>condition_value</b>
     * TODO: better documentation
     */
    template<typename ...Args, typename T>
    [[maybe_unused]] std::shared_ptr<SQLite::Statement>
    inline select(const std::string &table_name, const std::string &condition_column, const std::string &condition,
                  const T &condition_value, Args &&...columns) {
        if (!database) {
            std::cerr << "DBHelper::select -> " << "database is nullptr" << std::endl;
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
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::select -> " << e.what() << std::endl;
            return {};
        }
    }

    /**
    *  @sqlite UPDATE <b>table_name</b> SET (args...=args...)... WHERE <b>condition_column</b>='<b>condition_value</b>'
    *  TODO: better documentation
    */
    template<typename T, typename ...Args>
    inline void update(const std::string &table_name, const std::string &condition_column, const T &condition_value,
                       Args ...args) {
        if (!database) {
            std::cerr << "DBHelper::update -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            constexpr size_t n = sizeof...(args);
            typename integer_range_generate<std::size_t, 0, n - 2, 2>::type columns;
            typename integer_range_generate<std::size_t, 1, n - 1, 2>::type values;

            std::string sql = concatenate(
                    "UPDATE ", table_name,
                    " SET ",
                    write_col_name_eq_question_mark(columns, std::forward_as_tuple(std::forward<Args>(args)...)),
                    " WHERE ", condition_column, "=?");
            SQLite::Statement query(*database, sql);
            bind(query, values, std::forward_as_tuple(std::forward<Args>(args)...));
            query.bind((n / 2) + 1, condition_value);
            query.exec();
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::update -> " << e.what() << std::endl;
        }
    }

    /**
    *  @sqlite UPDATE <b>table_name</b> SET (args...=args...)... WHERE <b>condition_column</b>='<b>condition_value</b>'
    *  TODO: better documentation
    */
    template<typename T, typename ...Args>
    inline void
    update(const std::string &table_name, std::vector<std::tuple<std::string, std::string, T>> conditions,
           Args ...args) {
        if (!database) {
            std::cerr << "DBHelper::update -> " << "database is nullptr" << std::endl;
            return;
        }

        try {
            constexpr size_t n = sizeof...(args);
            typename integer_range_generate<std::size_t, 0, n - 2, 2>::type columns;
            typename integer_range_generate<std::size_t, 1, n - 1, 2>::type values;

            std::string sql = concatenate(
                    "UPDATE ", table_name,
                    " SET ",
                    write_col_name_eq_question_mark(columns, std::forward_as_tuple(std::forward<Args>(args)...)),
                    " WHERE ", form_conditions(conditions));
            std::cout << sql << std::endl;
            SQLite::Statement query(*database, sql);
            bind(query, values, std::forward_as_tuple(std::forward<Args>(args)...));
            int q = n-1;
            for (int i = 0; i < conditions.size(); ++i)
                query.bind(q++, std::get<2>(conditions.at(i)));

            query.exec();
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::update -> " << e.what() << std::endl;
        }
    }

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
    [[maybe_unused]] inline std::string as_questionmark(const T &t) {
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
    [[maybe_unused]] std::string intersect(const S1 &s, S ... args) {
        std::ostringstream oss;
        ((oss << s) << ... << ((std::string) ", " += std::forward<S>(args)));
        return oss.str();
    }

    static std::string intersect() {
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
        typename integer_range_generate<size_t, begin, end - 1, 1>::type indices;
        return intersect_indexes(indices, std::forward_as_tuple(std::forward<Args>(args)...));
    }

    template<typename Args, size_t... indexes>
    void bind(SQLite::Statement &query, integer_pack<size_t, indexes...>, Args &&args) {
        int n = 1;
        try {
            (query.bind(n++, std::get<indexes>(args)), ...);
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::bind -> " << e.what() << std::endl;
        }
    }

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
    void print_sequence2(integer_pack<size_t, i...>, Args &&args) {
        ((std::cout << std::get<i>(args) << ' '), ...);
        std::cout << '\n';
    }

    // debugging aid
    template<size_t... ints>
    void print_sequence(integer_pack<size_t, ints...>) {
        ((std::cout << ints << ' '), ...);
        std::cout << '\n';
    }

    // debugging aid
    template<size_t i, typename Args>
    void print_arg(Args &&args) {
        std::ostringstream oss;
        oss << std::get<i>(args) << '\n';
        std::cout << oss.str();
    }

    template<typename T>
    std::string form_conditions(std::vector<std::tuple<std::string, std::string, T>> &conditions) {
        if (conditions.empty())
            return {};

        std::ostringstream oss;
        oss << std::get<0>(conditions.at(0)) << std::get<1>(conditions.at(0)) << '?';
        if (conditions.size() == 1)
            return oss.str();

        for (int i = 1; i < conditions.size(); ++i) {
            oss << " AND " << std::get<0>(conditions.at(i)) << std::get<1>(conditions.at(i)) << '?';
        }
        return oss.str();
    }

    std::string get_str(std::string &&arg) {
        return arg;
    }

    template<typename Args, size_t... indexes>
    std::string write_col_name_eq_question_mark(integer_pack<size_t, indexes...>, Args &&args) {
        try {
            std::ostringstream oss;
            ((oss << std::get<indexes>(args) << "=?, "), ...);
            std::string result = oss.str();
            result.pop_back();
            result.pop_back();
            return result;
        } catch (SQLite::Exception &e) {
            std::cerr << "DBHelper::write_col_name_eq_question_mark -> " << e.what() << std::endl;
            return {};
        }
    }
};