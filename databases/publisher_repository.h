#pragma once
#include <string>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>
#include "C:/Users/kos22/CLionProjects/library/models/publisher.h"

class PublisherRepository {
private:
    SQLite::Database db_;
    void printTable(const std::vector<Publisher>& publishers);

public:
    PublisherRepository(const std::string& db_path = "library.db");
    bool initialize();
    bool publisherExists(const Publisher& publisher);
    int save(Publisher& publisher);
    void showAll();
    bool update(const std::string& field, const int& id, const std::string& new_val);
    bool del(const std::string& field, const std::string& value);
    void filter(const std::string& field, const std::string& direction);
    int find(const std::string& field, const std::string& value);
    void exportData(const std::string& format_type);
};