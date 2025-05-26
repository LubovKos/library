#pragma once
#include <string>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>

class Joiner {
private:
    std::string db_path_;

public:
    Joiner(const std::string& db_path = "library.db");
    int join(const std::string& table_title);
};