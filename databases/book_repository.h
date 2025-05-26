#pragma once
#include <string>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>
#include "C:/Users/kos22/CLionProjects/library/models/book.h"

class BookRepository {
private:
    SQLite::Database db_;
    void printTable(const std::vector<Book>& books);

public:
    BookRepository(const std::string& db_path = "library.db");
    bool initialize();
    bool bookExists(const Book& book);
    int save(Book& book);
    void showAll();
    bool update(const std::string& field, const int& id,  const std::string& new_val);
    bool del(const std::string& field, const std::string& value);
    void filter(const std::string& field, const std::string& direction);
    int find(const std::string& field, const std::string& value);
    void exportData(const std::string& format_type);
};