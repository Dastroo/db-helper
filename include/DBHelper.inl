//
// Created by dawid on 03.04.2022.
//

#pragma once

#include <SQLiteCpp/Database.h>


template<typename T>
bool DBHelper::exists(const std::string &table_name, const std::string &column, T value) {
    SQLite::Column c = database->execAndGet(concatenate(
            "SELECT EXISTS(SELECT 1 FROM ", table_name, " WHERE ", column, "=\"", value, "\" LIMIT 1)"));
    return c.getInt();
}

template<typename ...Args>
inline std::string DBHelper::create(const std::string &table_name, Args &&...args) {
    if (!database) {
        std::cerr << "DBHelper::create -> " << "database is nullptr" << std::endl;
        return {};
    }
    if (sizeof...(args) == 0) {
        std::cerr << "DBHelper::create -> " << "invalid argument" << std::endl;
        return {};
    }
    try {
        std::string sql = "CREATE TABLE " + table_name + " (";

        std::string body;
        (body.append(assign_value(args)), ...);
        body.erase(0, 2);

        sql += body += ')';
        SQLite::Statement query(*database, sql);
        query.exec();
        return sql;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::create -> " << e.what() << std::endl;
        return {};
    }
}

template<typename ...Args>
inline std::string DBHelper::insert(const std::string &table_name, Args ...args) {
    try {
        constexpr size_t n = sizeof...(args);
        constexpr size_t half_n = n / 2;
        if (n % 2 != 0 || n == 0) {
            std::cerr << "DBHelper::insert -> " << "needs even amount of arguments\n";
            return {};
        }
        std::string sql = mutl::concatenate(
                "INSERT INTO ",
                table_name,
                " (",
                mutl::format_with_comma<0, half_n - 1>(args...),
                ") VALUES (",
                intersected_questionmarks(half_n),
                ")");
        SQLite::Statement query(*database, sql);
        typename integer_range_generate<std::size_t, half_n, n - 1, 1>::type indices;
        bind(query, indices, std::forward_as_tuple(std::forward<Args>(args)...));
        query.exec();
        return sql;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::insert -> " << e.what() << std::endl;
        return {};
    }
}

template<typename T>
std::string DBHelper::insert(const std::string &table_name, std::vector<std::string, T> columns_values) {
    try {
        std::string sql = concatenate(
                "INSERT INTO ",
                table_name,
                " (",
                mutl::format_param_with_comma<0>(columns_values),
                ") VALUES (",
                intersected_questionmarks(columns_values.size()),
                ")");
        SQLite::Statement query(*database, sql);
        for (auto[column, value]: columns_values)
            bind(query, value);

        query.exec();
        return sql;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::insert -> " << e.what() << std::endl;
        return {};
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
        std::string sql = mutl::concatenate(
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
        std::string sql = mutl::concatenate(
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
inline std::shared_ptr<SQLite::Statement>
DBHelper::get(const std::string &table_name, const std::string &condition_column,
              const T &condition_value) {
    if (!database) {
        std::cerr << "DBHelper::get -> " << "database is nullptr" << std::endl;
        return {};
    }

    try {
        std::string sql = mutl::concatenate(
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
inline SQLite::Column
DBHelper::get(const std::string &table_name, const std::string &column, const std::string &condition_column,
              const T &condition_value) {
    if (!database) {
        std::cerr << "DBHelper::get -> " << "database is nullptr" << std::endl;
    }

    try {
        std::string sql = mutl::concatenate(
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

template<typename Col, typename Op, typename Val>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 const std::tuple<Col, Op, Val> &condition) {
    try {
        auto[column, op, value] = condition;
        std::string sql = mutl::concatenate(
                "SELECT * FROM ", table_name,
                " WHERE ", column, op, "?");

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);
        SQLite::bind(*query, value);

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 const std::vector<std::tuple<Col, Op, Val>> &conditions) {
    try {
        //  TODO: maybe delete str sql, return query instead and use query->getQuery()
        std::string sql = mutl::concatenate(
                "SELECT * FROM ", table_name,
                conditions.empty() ? "" : " WHERE ", parse_conditions(conditions));

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);

        for (int i = 0; i < conditions.size(); ++i)
            query->bind(i + 1, std::get<2>(conditions.at(i)));

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val, typename ...Args>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 const std::tuple<Col, Op, Val> &condition,
                 Args...columns) {
    try {
        auto[column, op, value] = condition;
        std::string sql = mutl::concatenate(
                "SELECT ", sizeof...(columns) == 0 ? "*" : mutl::format_with_comma(columns...),
                " FROM ", table_name,
                " WHERE ", column, op, "?");

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);
        SQLite::bind(*query, value);

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 const std::vector<std::string> &columns,
                 const std::tuple<Col, Op, Val> &condition) {
    try {
        auto[column, op, value] = condition;
        std::string sql = mutl::concatenate(
                "SELECT ", columns.empty() ? "*" : mutl::format_with_comma(columns),
                " FROM ", table_name,
                " WHERE ", column, op, "?");

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);
        SQLite::bind(*query, value);

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 std::initializer_list<std::string> columns,
                 const std::tuple<Col, Op, Val> &condition) {
    try {
        auto[column, op, value] = condition;
        std::string sql = mutl::concatenate(
                "SELECT ", empty(columns) ? "*" : mutl::format_with_comma(columns),
                " FROM ", table_name,
                " WHERE ", column, op, "?");

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);
        SQLite::bind(*query, value);

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val, typename ...Args>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 const std::vector<std::tuple<Col, Op, Val>> &conditions,
                 Args...columns) {
    try {
        //  TODO: maybe delete str sql, return query instead and use query->getQuery()
        std::string sql = mutl::concatenate(
                "SELECT ", mutl::format_with_comma(columns...),
                " FROM ", table_name,
                conditions.empty() ? "" : " WHERE ", parse_conditions(conditions));

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);

        for (int i = 0; i < conditions.size(); ++i)
            query->bind(i + 1, std::get<2>(conditions.at(i)));

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 const std::vector<std::string> &columns,
                 const std::vector<std::tuple<Col, Op, Val>> &conditions) {
    try {
        //  TODO: maybe delete str sql, return query instead and use query->getQuery()
        std::string sql = mutl::concatenate(
                "SELECT ", columns.empty() ? "*" : mutl::format_with_comma<std::string>(columns),
                " FROM ", table_name,
                conditions.empty() ? "" : " WHERE ", parse_conditions(conditions));

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);

        for (int i = 0; i < conditions.size(); ++i)
            query->bind(i + 1, std::get<2>(conditions.at(i)));

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val>
inline std::shared_ptr<SQLite::Statement>
DBHelper::select(const std::string &table_name,
                 std::initializer_list<std::string> columns,
                 const std::vector<std::tuple<Col, Op, Val>> &conditions) {
    try {
        //  TODO: maybe delete str sql, return query instead and use query->getQuery()
        std::string sql = mutl::concatenate(
                "SELECT ", empty(columns) ? "*" : mutl::format_with_comma<std::string>(columns),
                " FROM ", table_name,
                conditions.empty() ? "" : " WHERE ", parse_conditions(conditions));

        std::shared_ptr<SQLite::Statement> query =
                std::make_shared<SQLite::Statement>(*database, sql);

        for (int i = 0; i < conditions.size(); ++i)
            query->bind(i + 1, std::get<2>(conditions.at(i)));

        return query;
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::select -> " << e.what() << std::endl;
        return {};
    }
}

template<typename Col, typename Op, typename Val>
inline std::shared_ptr<SQLite::Statement>
select(const std::string &table_name,
       std::initializer_list<std::string> columns,
       const std::tuple<Col, Op, Val> &condition) {

}

template<typename T, typename ...Args>
inline void
DBHelper::update(const std::string &table_name, const std::string &condition_column, const T &condition_value,
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
                parse_columns_variadic(columns, std::forward_as_tuple(std::forward<Args>(args)...)),
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
                parse_columns_variadic(columns, std::forward_as_tuple(std::forward<Args>(args)...)),
                " WHERE ", parse_conditions(conditions));
        SQLite::Statement query(*database, sql);
        bind(query, values, std::forward_as_tuple(std::forward<Args>(args)...));
        int q = n - 1;
        for (int i = 0; i < conditions.size(); ++i)
            query.bind(q++, std::get<2>(conditions.at(i)));

        query.exec();
    } catch (SQLite::Exception &e) {
        std::cerr << "DBHelper::update -> " << e.what() << std::endl;
    }
}

template<typename T>
inline std::string DBHelper::as_questionmark(const T &t) {
    return "?";
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

template<typename Col, typename Op, typename Val>
std::string DBHelper::parse_conditions(const std::vector<std::tuple<Col, Op, Val>> &conditions) {
    if (conditions.empty())
        return {};

    std::stringstream ss;
    ss << std::get<0>(conditions.at(0)) << std::get<1>(conditions.at(0)) << "?";
    if (conditions.size() == 1)
        return ss.str();

    for (int i = 1; i < conditions.size(); ++i)
        ss << " AND " << std::get<0>(conditions.at(i)) << std::get<1>(conditions.at(i)) << "?";

    return ss.str();
}

template<typename Args, size_t... indexes>
std::string DBHelper::parse_columns_variadic(integer_pack<size_t, indexes...>, Args &&args) {
    std::stringstream ss;
    ((ss << std::get<indexes>(args) << "=?, "), ...);
    std::string result = ss.str();
    result.pop_back();
    result.pop_back();
    return result;
}