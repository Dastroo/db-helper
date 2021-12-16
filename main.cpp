//
// Created by dawid on 11.12.2021.
//

#include "DBHelper.h"

using type = DBHelper::type;

int main() {
    DBHelper db_helper;

    db_helper.create("table_name", "id", type::INTEGER, type::PRIMARY_KEY, type::AUTO_INCREMENT, "value", type::TEXT);
    if (db_helper.table_exists("table_name"))
        std::cout << "table exists" << std::endl;
    else
        std::cout << "table does not exist" << std::endl;
    db_helper.write_to_cli("table_name");
    std::cout << std::endl;

    db_helper.insert<std::string, std::string>("table_name", "id", "value", 0, "a");
    db_helper.write_to_cli("table_name");
    std::cout << std::endl;

    db_helper.insert<std::string>("table_name", "value", "b");
    db_helper.write_to_cli("table_name");
    std::cout << std::endl;

    db_helper.insert<std::string>("table_name", "value", "c");
    db_helper.write_to_cli("table_name");
    std::cout << std::endl;

    db_helper.update("table_name", "value", "d", "id", 0);
    db_helper.write_to_cli("table_name");
    std::cout << std::endl;

    db_helper.update("table_name", "value", "e", "id", ">", 0);
    db_helper.write_to_cli("table_name");
    std::cout << std::endl;

    SQLite::Statement *query = db_helper.get("table_name", "id", ">", 0, "id", "value");
    while (query->executeStep()) {
        std::cout << "id: " << query->getColumn("id") << "\tvalue: " << query->getColumn(1) << std::endl;
    }
    std::cout << std::endl;

    db_helper.remove("table_name", "id", "=", 2);
    db_helper.write_to_cli("table_name");
    std::cout << std::endl;

    db_helper.drop("table_name");
    if (db_helper.table_exists("table_name"))
        std::cout << "table exists" << std::endl;
    else
        std::cout << "table does not exist" << std::endl;

    return 0;
}