//
// Created by dawid on 03.04.2022.
//

#pragma once


template<typename T>
bool DBHelper::exists(const std::string &table_name, const std::string &column, T value) {
    SQLite::Column c = database->execAndGet(concatenate(
            "SELECT EXISTS(SELECT 1 FROM ", table_name, " WHERE ", column, "=\"", value, "\" LIMIT 1)"));
    return c.getInt();
}

template<typename ...Args>
inline void DBHelper::create(const std::string &table_name, Args &&...args) {
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

template<typename ...Args>
inline void DBHelper::insert(const std::string &table_name, Args ...args) {
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

template<typename T>
void
inline
DBHelper::dele(const std::string &table_name, const std::string &column, const std::string &condition, const T &value) {
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

template<typename T>
void
inline
DBHelper::dele(const std::string &table_name, const std::string &column, const T &value) {
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

template<typename T>
[[maybe_unused]] std::shared_ptr<SQLite::Statement>
inline DBHelper::get(const std::string &table_name, const std::string &condition_column,
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

template<typename T>
[[maybe_unused]] SQLite::Column
inline DBHelper::get(const std::string &table_name, const std::string &column, const std::string &condition_column,
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

template<typename T>
[[maybe_unused]] std::shared_ptr<SQLite::Statement>
inline DBHelper::select(const std::string &table_name, const std::string &condition_column, const T &condition_value) {
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

template<typename T>
[[maybe_unused]] std::shared_ptr<SQLite::Statement>
inline DBHelper::select(const std::string &table_name, const std::string &condition_column, const std::string &condition,
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

template<typename ...Args, typename T>
[[maybe_unused]] std::shared_ptr<SQLite::Statement>
inline DBHelper::select(const std::string &table_name, const std::string &condition_column, const std::string &condition,
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

template<typename T, typename ...Args>
inline void DBHelper::update(const std::string &table_name, const std::string &condition_column, const T &condition_value,
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

template<typename T, typename ...Args>
inline void
DBHelper::update(const std::string &table_name, std::vector<std::tuple<std::string, std::string, T>> conditions,
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

template<typename T>
[[maybe_unused]] inline std::string DBHelper::as_questionmark(const T &t) {
    return "?";
}

template<typename... S>
std::string DBHelper::concatenate(S ... args) {
    std::ostringstream oss;
    (oss << ... << std::forward<S>(args));
    return oss.str();
}

template<typename S1, typename... S>
[[maybe_unused]] std::string DBHelper::intersect(const S1 &s, S ... args) {
    std::ostringstream oss;
    ((oss << s) << ... << ((std::string) ", " += std::forward<S>(args)));
    return oss.str();
}

template<typename Tuple, size_t... indexes>
std::string DBHelper::intersect_indexes(integer_pack<size_t, indexes...>, Tuple &&tuple) {
    return intersect(std::get<indexes>(tuple)...);
}

template<size_t begin, size_t end, typename... Args>
std::string DBHelper::intersect(Args &&... args) {
    typename integer_range_generate<size_t, begin, end - 1, 1>::type indices;
    return intersect_indexes(indices, std::forward_as_tuple(std::forward<Args>(args)...));
}

template<typename Args, size_t... indexes>
void DBHelper::bind(SQLite::Statement &query, integer_pack<size_t, indexes...>, Args &&args) {
    int n = 1;
    try {
        (query.bind(n++, std::get<indexes>(args)), ...);
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::bind -> " << e.what() << std::endl;
    }
}

// debugging aid
template<size_t... i, typename Args>
void DBHelper::print_sequence2(integer_pack<size_t, i...>, Args &&args) {
    ((std::cout << std::get<i>(args) << ' '), ...);
    std::cout << '\n';
}

// debugging aid
template<size_t... ints>
void DBHelper::print_sequence(integer_pack<size_t, ints...>) {
    ((std::cout << ints << ' '), ...);
    std::cout << '\n';
}

// debugging aid
template<size_t i, typename Args>
void DBHelper::print_arg(Args &&args) {
    std::ostringstream oss;
    oss << std::get<i>(args) << '\n';
    std::cout << oss.str();
}

template<typename T>
std::string DBHelper::form_conditions(std::vector<std::tuple<std::string, std::string, T>> &conditions) {
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

template<typename Args, size_t... indexes>
std::string DBHelper::write_col_name_eq_question_mark(integer_pack<size_t, indexes...>, Args &&args) {
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
